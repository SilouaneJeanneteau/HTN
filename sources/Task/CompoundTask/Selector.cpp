#include "Selector.h"

#include "../PrimitiveTask/PrimitiveTask.h"
#include "../../Context/BaseContext.h"
#include "../../Task/TaskInterface.h"
#include "../../Task/Slot.h"

namespace HTN
{
	Selector::Selector()
		: CompoundTask()
	{
		m_types.set(TaskType::Selector);
	}

	bool Selector::IsValid(BaseContext* _ctx) const
	{
		if (!CompoundTask::IsValid(_ctx))
		{
			return false;
		}

		// Selector requires there to be at least one sub-task to successfully select from.
		if (m_subtasks.empty())
		{
			return false;
		}

		return true;
	}

	bool Selector::_BeatsLastMTR(BaseContext* _ctx, int _taskIndex, int _currentDecompositionIndex)
	{
		// If the last plan's traversal record for this decomposition layer 
		// has a smaller index than the current task index we're about to
		// decompose, then the new decomposition can't possibly beat the
		// running plan, so we cancel finding a new plan.
		const auto& lastMTR = _ctx->GetLastMTR();
		if (lastMTR[_currentDecompositionIndex] < _taskIndex)
		{
			// But, if any of the earlier records beat the record in LastMTR, we're still good, as we're on a higher priority branch.
			// This ensures that [0,0,1] can beat [0,1,0]
			auto& currentMTR = _ctx->GetCurrentMTR();
			for (u32 i = 0u, count = currentMTR.size(); i < count; i++)
			{
				int diff = currentMTR[i] - lastMTR[i];
				if (diff < 0)
				{
					return true;
				}
				if (diff > 0)
				{
					// We should never really be able to get here, but just in case.
					return false;
				}
			}

			return false;
		}

		return true;
	}

	DecompositionStatus Selector::OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result)
	{
		m_plan = {};

		for (int taskIndex = _startIndex, count = m_subtasks.size(); taskIndex < count; ++taskIndex)
		{
			// If the last plan is still running, we need to check whether the
			// new decomposition can possibly beat it.
			const auto& lastMTR = _ctx->GetLastMTR();
			if (!lastMTR.empty())
			{
				auto& currentMTR = _ctx->GetCurrentMTR();
				auto currentDecompositionIndex = currentMTR.size();
				if (currentDecompositionIndex < lastMTR.size())
				{
					if (!_BeatsLastMTR(_ctx, taskIndex, currentDecompositionIndex))
					{
						currentMTR.push_back(-1);
#ifndef HTN_FINAL
						_ctx->GetCurrentMTRDebug().push_back("REPLAN FAIL {Subtasks[taskIndex].Name}");

#endif

						_result = {};
						return DecompositionStatus::Rejected;
					}
				}
			}

			auto* task = m_subtasks[taskIndex];

			auto status = OnDecomposeTask(_ctx, task, taskIndex, {}, _result);
			switch (status)
			{
				case DecompositionStatus::Rejected:
				case DecompositionStatus::Succeeded:
				case DecompositionStatus::Partial:
				{
					return status;
				}
				break;

				case DecompositionStatus::Failed:
				default:
				{
					continue;
				}
				break;
			}
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}

	DecompositionStatus Selector::OnDecomposeTask(BaseContext* _ctx, TaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		if (!_task->IsValid(_ctx))
		{
			_result = m_plan;
			return _task->OnIsValidFailed(_ctx);
		}

		if (_task->HasType(TaskType::Compound))
		{
			return OnDecomposeCompoundTask(_ctx, static_cast<CompoundTaskInterface*>(_task), _taskIndex, {}, _result);
		}
		else if (_task->HasType(TaskType::Primitive))
		{
			auto* primitiveTask = static_cast<PrimitiveTask*>(_task);
			primitiveTask->ApplyEffects(_ctx);
			m_plan.push(primitiveTask);
		}
		else if (_task->HasType(TaskType::Slot))
		{
			return OnDecomposeSlot(_ctx, static_cast<Slot*>(_task), _taskIndex, {}, _result);
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}

	DecompositionStatus Selector::OnDecomposeCompoundTask(BaseContext* _ctx, CompoundTaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		auto& currentMTR = _ctx->GetCurrentMTR();
		currentMTR.push_back(_taskIndex);
#ifndef HTN_FINAL
		_ctx->GetCurrentMTRDebug().push_back(_task->GetName());
#endif

		std::queue<TaskInterface*> subPlan;
		const auto status = _task->Decompose(_ctx, 0, subPlan);

		// If status is rejected, that means the entire planning procedure should cancel.
		if (status == DecompositionStatus::Rejected)
		{
			_result = {};
			return DecompositionStatus::Rejected;
		}

		// If the decomposition failed
		if (status == DecompositionStatus::Failed)
		{
			// Remove the taskIndex if it failed to decompose.
			currentMTR.pop_back();
#ifndef HTN_FINAL
			_ctx->GetCurrentMTRDebug().pop_back();
#endif

			_result = m_plan;
			return DecompositionStatus::Failed;
		}

		while (!subPlan.empty())
		{
			m_plan.push(subPlan.front());
			subPlan.pop();
		}

		if (_ctx->HasPausedPartialPlan())
		{
			_result = m_plan;
			return DecompositionStatus::Partial;
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}

	DecompositionStatus Selector::OnDecomposeSlot(BaseContext* _ctx, Slot* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		// We need to record the task index before we decompose the task,
		// so that the traversal record is set up in the right order.
		auto& currentMTR = _ctx->GetCurrentMTR();
		currentMTR.push_back(_taskIndex);
#ifndef HTN_FINAL
		_ctx->GetCurrentMTRDebug().push_back(_task->GetName());
#endif

		std::queue<TaskInterface*> subPlan;
		auto status = _task->Decompose(_ctx, 0, subPlan);

		// If status is rejected, that means the entire planning procedure should cancel.
		if (status == DecompositionStatus::Rejected)
		{
			_result = {};
			return DecompositionStatus::Rejected;
		}

		// If the decomposition failed
		if (status == DecompositionStatus::Failed)
		{
			// Remove the taskIndex if it failed to decompose.
			currentMTR.pop_back();
#ifndef HTN_FINAL
			_ctx->GetCurrentMTRDebug().pop_back();
#endif

			_result = m_plan;
			return DecompositionStatus::Failed;
		}

		while (!subPlan.empty())
		{
			m_plan.push(subPlan.front());
			subPlan.pop();
		}

		if (_ctx->HasPausedPartialPlan())
		{
			_result = m_plan;
			return DecompositionStatus::Partial;
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}
}
