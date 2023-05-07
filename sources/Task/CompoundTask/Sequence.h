#pragma once

#include "CompoundTask.h"

#include <queue>

namespace HTN
{
	class CompoundTaskInterface;
	class TaskInterface;
	class BaseContext;

	class Sequence : public CompoundTask
	{
	public:
		Sequence();
		virtual bool IsValid(BaseContext* _ctx) const;

		DecompositionStatus OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result) override;

		DecompositionStatus OnDecomposeTask(BaseContext* _ctx, TaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result) override;

		DecompositionStatus OnDecomposeCompoundTask(BaseContext* _ctx, CompoundTaskInterface* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result) override;

		DecompositionStatus OnDecomposeSlot(BaseContext* _ctx, Slot* _task, int _taskIndex, const std::vector<int>& _oldStackDepth, std::queue<TaskInterface*>& _result) override;

	protected:
		std::queue<TaskInterface*> m_plan;
	};

}
