#include "Sequence.h"

#include "../PrimitiveTask/PrimitiveTask.h"
#include "../../Context/BaseContext.h"
#include "../../Task/TaskInterface.h"
#include "../../Task/Slot.h"

namespace HTN
{

	Sequence::Sequence()
		: CompoundTask()
	{
		m_types.set(TaskType::Sequence);
	}

	bool Sequence::IsValid(BaseContext* _ctx) const
	{
		if (!CompoundTask::IsValid(_ctx))
		{
			return false;
		}

		// Sequence requires there to be at least one sub-task to successfully select from.
		if (m_subtasks.empty())
		{
			return false;
		}

		return true;
	}

	DecompositionStatus Sequence::OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result)
	{
		m_plan = {};

		auto oldStackDepth = _ctx->GetWorldStateChangeDepth();

		for (int taskIndex = _startIndex, count = m_subtasks.size(); taskIndex < count; ++taskIndex)
		{
			auto* task = m_subtasks[taskIndex];

			auto status = OnDecomposeTask(_ctx, task, taskIndex, oldStackDepth, _result);
			switch (status)
			{
				case DecompositionStatus::Rejected:
				case DecompositionStatus::Failed:
				case DecompositionStatus::Partial:
				{
					return status;
				}
				break;

				default:
				break;
			}
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}

	DecompositionStatus Sequence::OnDecomposeTask(BaseContext* _ctx, TaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		if (!_task->IsValid(_ctx))
		{
			m_plan = {};
			_ctx->TrimToStackDepth(_oldStackDepth);
			_result = m_plan;
			return _task->OnIsValidFailed(_ctx);
		}

		if (_task->HasType(TaskType::Compound))
		{
			return OnDecomposeCompoundTask(_ctx, static_cast<CompoundTaskInterface*>(_task), _taskIndex, _oldStackDepth, _result);
		}
		else if (_task->HasType(TaskType::Primitive))
		{
			auto* primitiveTask = static_cast<PrimitiveTask*>(_task);
			primitiveTask->ApplyEffects(_ctx);
			m_plan.push(primitiveTask);
		}
		else if (_task->HasType(TaskType::PausePlan))
		{
			_ctx->SetHasPausedPartialPlan(true);
			_ctx->GetPartialPlanQueue().push({ this, _taskIndex + 1 });

			_result = m_plan;
			return DecompositionStatus::Partial;
		}
		else if (_task->HasType(TaskType::Slot))
		{
			return OnDecomposeSlot(_ctx, static_cast<Slot*>(_task), _taskIndex, _oldStackDepth, _result);
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}

	DecompositionStatus Sequence::OnDecomposeCompoundTask(BaseContext* _ctx, CompoundTaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		std::queue<TaskInterface*> subPlan;
		auto status = _task->Decompose(_ctx, 0, subPlan);

		// If the decomposition is rejected, that means the entire planning procedure should cancel.
		// or if the decomposition failed.
		if (status == DecompositionStatus::Rejected || status == DecompositionStatus::Failed)
		{
			m_plan = {};
			_ctx->TrimToStackDepth(_oldStackDepth);
			_result = {};
			return status;
		}

		while (!subPlan.empty())
		{
			m_plan.push(subPlan.front());
			subPlan.pop();
		}

		if (_ctx->HasPausedPartialPlan())
		{
			if (_taskIndex < m_subtasks.size() - 1)
			{
				_ctx->GetPartialPlanQueue().push({ this, _taskIndex + 1 });
			}

			_result = m_plan;
			return DecompositionStatus::Partial;
		}

		_result = m_plan;
		return DecompositionStatus::Succeeded;
	}

	DecompositionStatus Sequence::OnDecomposeSlot(BaseContext* _ctx, Slot* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		std::queue<TaskInterface*> subPlan;
		auto status = _task->Decompose(_ctx, 0, subPlan);

		// If the decomposition is rejected, that means the entire planning procedure should cancel.
		// or if the decomposition failed.
		if (status == DecompositionStatus::Rejected || status == DecompositionStatus::Failed)
		{
			m_plan = {};
			_ctx->TrimToStackDepth(_oldStackDepth);
			_result = {};
			return status;
		}

		while (!subPlan.empty())
		{
			m_plan.push(subPlan.front());
			subPlan.pop();
		}

		if (_ctx->HasPausedPartialPlan())
		{
			if (_taskIndex < m_subtasks.size() - 1)
			{
				_ctx->GetPartialPlanQueue().push({ this, _taskIndex + 1 });
			}

			_result = m_plan;
			return DecompositionStatus::Partial;
		}

		_result = m_plan;
		return DecompositionStatus::Succeeded;
	}

}
