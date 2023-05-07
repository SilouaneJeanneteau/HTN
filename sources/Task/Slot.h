#pragma once

#include "TaskInterface.h"

#include <queue>

namespace HTN
{
	class CompoundTaskInterface;
	class ConditionInterface;
	class BaseContext;

	class Slot : public TaskInterface
	{
	public:
		Slot() : TaskInterface(TaskType::Slot) {}
		TaskInterface* AddCondition(ConditionInterface* _condition) override;

		bool IsValid(BaseContext* _ctx) const override;

		DecompositionStatus OnIsValidFailed(BaseContext* _ctx) override;

		virtual DecompositionStatus Decompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result);

		int GetSlotID() const { return m_slotId; }
		void SetSlotID(int _id) { m_slotId = _id; }

		const CompoundTaskInterface* GetSubtask() const { return m_subtask; }
		bool SetSubtask(CompoundTaskInterface* _subtask);
		void ClearSubtask();

	private:
		int m_slotId = -1;
		CompoundTaskInterface* m_subtask = nullptr;
	};

}
