#pragma once

#include "CompoundTaskInterface.h"

namespace HTN
{
	class ConditionInterface;
	class BaseContext;
	class Slot;

	class CompoundTask : public CompoundTaskInterface
	{
	public:
		TaskInterface* AddCondition(ConditionInterface* _condition) override;

		CompoundTaskInterface* AddSubtask(TaskInterface* _subtask) override;

		bool IsValid(BaseContext* _ctx) const override;

		DecompositionStatus OnIsValidFailed(BaseContext* _ctx) override;

		DecompositionStatus Decompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result) override;

		virtual DecompositionStatus OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result);

		virtual DecompositionStatus OnDecomposeTask(BaseContext* _ctx, TaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result);

		virtual DecompositionStatus OnDecomposeCompoundTask(BaseContext* _ctx, CompoundTaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result);

		virtual DecompositionStatus OnDecomposeSlot(BaseContext* _ctx, Slot* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result);
	};

}
