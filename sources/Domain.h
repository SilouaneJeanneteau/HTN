#pragma once

#include "HTNEnums.h"

#include <queue>
#include <map>
#include <string>

namespace HTN
{
	class CompoundTaskInterface;
	class TaskInterface;
	class BaseContext;
	class Selector;
	class Slot;

	class Domain
	{
	public:
		Domain(const std::string& _name);
		~Domain();

		Selector* GetRoot() { return m_root; }

		void Add(CompoundTaskInterface* _parent, TaskInterface* _subtask);
		void Add(CompoundTaskInterface* _parent, Slot* _slot);
		bool TrySetSlotDomain(int _slotId, Domain* _subDomain);
		void ClearSlot(int _slotId);

		DecompositionStatus FindPlan(BaseContext* _ctx, std::queue<TaskInterface*>& _plan);

	private:
		Selector* m_root = nullptr;
		std::map<int, Slot*> m_slots;
	};

}
