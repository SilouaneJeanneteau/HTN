#pragma once

#include "../../HTNTypes.h"
#include "../TaskInterface.h"

#include <queue>

namespace HTN
{
	class BaseContext;

	class CompoundTaskInterface : public TaskInterface
	{
	public:
		CompoundTaskInterface() : TaskInterface(TaskType::Compound) {}
		~CompoundTaskInterface() override
		{
			for (auto* subtask : m_subtasks)
			{
				delete subtask;
			}
		}

		const std::vector<TaskInterface*> GetSubtasks() const { return m_subtasks; }
		virtual CompoundTaskInterface* AddSubtask(TaskInterface* _subtask) = 0;

		virtual DecompositionStatus Decompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result) = 0;

	protected:
		std::vector<TaskInterface*> m_subtasks;
	};

}
