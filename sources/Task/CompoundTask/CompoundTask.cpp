#include "CompoundTask.h"

#include "../../Condition/ConditionInterface.h"

namespace HTN
{
	TaskInterface* CompoundTask::AddCondition(ConditionInterface* _condition)
	{
		m_conditions.push_back(_condition);
		return this;
	}

	CompoundTaskInterface* CompoundTask::AddSubtask(TaskInterface* _subtask)
	{
		m_subtasks.push_back(_subtask);
		return this;
	}

	bool CompoundTask::IsValid(BaseContext* _ctx) const
	{
		for(const auto* condition : m_conditions)
		{
			if (!condition->IsValid(_ctx))
			{
				return false;
			}
		}

		return true;
	}

	DecompositionStatus CompoundTask::OnIsValidFailed(BaseContext* _ctx)
	{
		return DecompositionStatus::Failed;
	}

	DecompositionStatus CompoundTask::Decompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result)
	{
		return OnDecompose(_ctx, _startIndex, _result);
	}

	DecompositionStatus CompoundTask::OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result)
	{
		return DecompositionStatus::Failed;
	}

	DecompositionStatus CompoundTask::OnDecomposeTask(BaseContext* _ctx, TaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		return DecompositionStatus::Failed;
	}

	DecompositionStatus CompoundTask::OnDecomposeCompoundTask(BaseContext* _ctx, CompoundTaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		return DecompositionStatus::Failed;
	}

	DecompositionStatus CompoundTask::OnDecomposeSlot(BaseContext* _ctx, Slot* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result)
	{
		return DecompositionStatus::Failed;
	}

}
