#include "Slot.h"

#include "../Condition/ConditionInterface.h"
#include "../Task/CompoundTask/CompoundTaskInterface.h"

#include <cassert>

namespace HTN
{
	TaskInterface* Slot::AddCondition(ConditionInterface* _condition)
	{
		assert(false); // HTN Slot task does not support conditions
		return this;
	}

	bool Slot::IsValid(BaseContext* _ctx) const
	{
		return m_subtask != nullptr;
	}

	DecompositionStatus Slot::OnIsValidFailed(BaseContext* _ctx)
	{
		return DecompositionStatus::Failed;
	}

	DecompositionStatus Slot::Decompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result)
	{
		if (m_subtask != nullptr)
		{
			return m_subtask->Decompose(_ctx, _startIndex, _result);
		}

		return DecompositionStatus::Failed;
	}

	bool Slot::SetSubtask(CompoundTaskInterface* _subtask)
	{
		if (m_subtask != nullptr)
		{
			return false;
		}

		m_subtask = _subtask;
		return true;
	}

	void Slot::ClearSubtask()
	{
		m_subtask = nullptr;
	}

}
