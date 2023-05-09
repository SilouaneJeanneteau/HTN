#include "Domain.h"

#include "Task/CompoundTask/Selector.h"
#include "Task/Slot.h"
#include "Context/BaseContext.h"

namespace HTN
{
	Domain::Domain(const std::string& _name)
	{
		m_root = new Selector();
		m_root->SetName(_name);
	}

	Domain::~Domain()
	{
		delete m_root;
	}

	void Domain::Add(CompoundTaskInterface* _parent, TaskInterface* _subtask)
	{
		assert(_parent != _subtask); // Parent-task and Sub-task can't be the same instance

		_parent->AddSubtask(_subtask);
		_subtask->SetParent(_parent);
	}

	void Domain::Add(CompoundTaskInterface* _parent, Slot* _slot)
	{
		assert(m_slots.count(_slot->GetSlotID()) == 0); // This slot id already exist in the domain definition

		_parent->AddSubtask(_slot);
		_slot->SetParent(_parent);

		m_slots[_slot->GetSlotID()] = _slot;
	}

	bool Domain::TrySetSlotDomain(int _slotId, Domain* _subDomain)
	{
		auto slotIt = m_slots.find(_slotId);
		if (slotIt != m_slots.end())
		{
			return slotIt->second->SetSubtask(_subDomain->GetRoot());
		}

		return false;
	}

	void Domain::ClearSlot(int _slotId)
	{
		auto slotIt = m_slots.find(_slotId);
		if (slotIt != m_slots.end())
		{
			slotIt->second->ClearSubtask();
		}
	}

	DecompositionStatus Domain::FindPlan(BaseContext* _ctx, std::queue<TaskInterface*>& _plan)
	{
		_ctx->SetContextState(ContextState::Planning);

		_plan = {};
		auto status = DecompositionStatus::Rejected;
		auto& partialPlanQueue = _ctx->GetPartialPlanQueue();

		auto& currentMTR = _ctx->GetCurrentMTR();
		auto& lastMTR = _ctx->GetLastMTR();
#ifndef HTN_FINAL
		auto& currentMTRDebug = _ctx->GetCurrentMTRDebug();
#endif

		if (_ctx->HasPausedPartialPlan() && lastMTR.empty())
		{
			_ctx->SetHasPausedPartialPlan(false);
			while (!partialPlanQueue.empty())
			{
				auto& kvp = partialPlanQueue.front();
				if (_plan.empty())
				{
					status = kvp.task->Decompose(_ctx, kvp.taskIndex, _plan);
				}
				else
				{
					std::queue<TaskInterface*> subPlan;
					status = kvp.task->Decompose(_ctx, kvp.taskIndex, subPlan);
					if (status == DecompositionStatus::Succeeded || status == DecompositionStatus::Partial)
					{
						while (!subPlan.empty())
						{
							_plan.push(subPlan.front());
							subPlan.pop();
						}
					}
				}

				// While continuing a partial plan, we might encounter
				// a new pause.
				if (_ctx->HasPausedPartialPlan())
				{
					break;
				}
			}

			// If we failed to continue the paused partial plan,
			// then we have to start planning from the root.
			if (status == DecompositionStatus::Rejected || status == DecompositionStatus::Failed)
			{
				currentMTR.clear();
#ifndef HTN_FINAL
				currentMTRDebug.clear();
#endif

				status = m_root->Decompose(_ctx, 0, _plan);
			}
		}
		else
		{
			bool wasPartialPlan = false;
			std::queue<PartialPlanEntry> lastPartialPlanQueue;
			if (_ctx->HasPausedPartialPlan())
			{
				wasPartialPlan = true;
				_ctx->SetHasPausedPartialPlan(false);
				while (!partialPlanQueue.empty())
				{
					lastPartialPlanQueue.push(partialPlanQueue.front());
					partialPlanQueue.pop();
				}
			}

			// We only erase the MTR if we start from the root task of the domain.
			currentMTR.clear();
#ifndef HTN_FINAL
			currentMTRDebug.clear();
#endif

			status = m_root->Decompose(_ctx, 0, _plan);

			// If we failed to find a new plan, we have to restore the old plan,
			// if it was a partial plan.
			if (wasPartialPlan)
			{
				if (status == DecompositionStatus::Rejected || status == DecompositionStatus::Failed)
				{
					_ctx->SetHasPausedPartialPlan(true);
					partialPlanQueue = {};
					while (!lastPartialPlanQueue.empty())
					{
						partialPlanQueue.push(lastPartialPlanQueue.front());
						lastPartialPlanQueue.pop();
					}
				}
			}
		}

		// If this MTR equals the last MTR, then we need to double check whether we ended up
		// just finding the exact same plan. During decomposition each compound task can't check
		// for equality, only for less than, so this case needs to be treated after the fact.
		bool isMTRsEqual = currentMTR.size() == lastMTR.size();
		if (isMTRsEqual)
		{
			for (size_t index = 0, count = currentMTR.size(); index < count; ++index)
			{
				if (currentMTR[index] < lastMTR[index])
				{
					isMTRsEqual = false;
					break;
				}
			}

			if (isMTRsEqual)
			{
				_plan = {};
				status = DecompositionStatus::Rejected;
			}
		}

		if (status == DecompositionStatus::Succeeded || status == DecompositionStatus::Partial)
		{
			// Trim away any plan-only or plan&execute effects from the world state change stack, that only
			// permanent effects on the world state remains now that the planning is done.
			_ctx->TrimForExecution();

			// Apply permanent world state changes to the actual world state used during plan execution.
			auto& worldStateChangeStack = _ctx->GetWorldStateChangeStack();
			auto& worldState = _ctx->GetWorldState();
			for (size_t index = 0, count = worldStateChangeStack.size(); index < count; ++index)
			{
				auto& stack = worldStateChangeStack[index];
				if (!stack.empty())
				{
					worldState[index] = stack.top().second;
					stack = {};
				}
			}
		}
		else
		{
			// Clear away any changes that might have been applied to the stack
			// No changes should be made or tracked further when the plan failed.
			auto& worldStateChangeStack = _ctx->GetWorldStateChangeStack();
			for (size_t index = 0, count = worldStateChangeStack.size(); index < count; ++index)
			{
				worldStateChangeStack[index] = {};
			}
		}

		_ctx->SetContextState(ContextState::Executing);
		return status;
	}
}
