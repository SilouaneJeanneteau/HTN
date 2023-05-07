#include "DomainBuilder.h"

#include "Task/PrimitiveTask/PrimitiveTask.h"
#include "Task/CompoundTask/PausePlanTask.h"
#include "Task/CompoundTask/GOAPSequence.h"
#include "Task/CompoundTask/Selector.h"
#include "Task/CompoundTask/Sequence.h"
#include "Condition/FuncCondition.h"
#include "Task/GOAPTaskInterface.h"
#include "Operator/FuncOperator.h"
#include "Effect/ActionEffect.h"
#include "Task/TaskInterface.h"
#include "Task/Slot.h"
#include "Domain.h"

namespace HTN
{
	DomainBuilder::DomainBuilder(const std::string& _domainName)
	{
		m_domain = new Domain(_domainName);
		m_pointers.push_back(m_domain->GetRoot());
	}

	DomainBuilder::~DomainBuilder()
	{
		if (m_domainOwner)
		{
			delete m_domain;
		}
	}

	TaskInterface* DomainBuilder::GetPointer()
	{
		if (m_pointers.empty())
		{
			return nullptr;
		}
		return m_pointers.back();
	}

	DomainBuilder* DomainBuilder::PrimitiveTask(const std::string& _name)
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Compound))
			{
				auto* compoundTask = static_cast<CompoundTaskInterface*>(pointer);
				auto* subTask = new HTN::PrimitiveTask();
				subTask->SetName(_name);
				m_domain->Add(compoundTask, subTask);
				m_pointers.push_back(subTask);
				return this;
			}
		}

		assert(false); // PrimitiveTask can not be built

		return this;
	}

	DomainBuilder* DomainBuilder::PausePlanTask()
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Compound))
			{
				auto* compoundTask = static_cast<CompoundTaskInterface*>(pointer);
				auto* subTask = new HTN::PausePlanTask();
				subTask->SetName("Pause Plan");
				m_domain->Add(compoundTask, subTask);
				return this;
			}
		}

		assert(false); // PausePlanTask can not be built

		return this;
	}

	DomainBuilder* DomainBuilder::Sequence(const std::string& _name)
	{
		return CompoundTask<HTN::Sequence>(_name);
	}


	DomainBuilder* DomainBuilder::Select(const std::string& _name)
	{
		return CompoundTask<HTN::Selector>(_name);
	}


	DomainBuilder* DomainBuilder::GOAPSequence(const std::string& _name, const std::map<WorldStateType, u8>& _goal)
	{
		CompoundTask<HTN::GOAPSequence>(_name);
		if (auto* pointer = GetPointer())
		{
			auto* goapSequence = static_cast<HTN::GOAPSequence*>(pointer);
			for (auto kvp : _goal)
			{
				goapSequence->AddGoalState(kvp.first, kvp.second);
			}
		}

		return this;
	}

	DomainBuilder* DomainBuilder::Condition(const std::string& _name, std::function<bool(BaseContext*)> _condition)
	{
		if (auto* pointer = GetPointer())
		{
			auto* cond = new FuncCondition(_name, _condition);
			pointer->AddCondition(cond);
			return this;
		}

		assert(false); // Condition can not be built

		return this;
	}

	DomainBuilder* DomainBuilder::ExecutingCondition(const std::string& _name, std::function<bool(BaseContext*)> _condition)
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Primitive))
			{
				auto* primitiveTask = static_cast<HTN::PrimitiveTask*>(pointer);
				auto* cond = new FuncCondition(_name, _condition);
				primitiveTask->AddExecutingCondition(cond);
				return this;
			}
		}

		assert(false); // ExecutingCondition can not be built

		return this;
	}

	DomainBuilder* DomainBuilder::Do(std::function<TaskStatus(BaseContext*)> _action, std::function <void(BaseContext*)> _forceStopAction)
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Primitive))
			{
				auto* primitiveTask = static_cast<HTN::PrimitiveTask*>(pointer);
				auto* op = new FuncOperator(_action, _forceStopAction);
				primitiveTask->SetOperator(op);
				return this;
			}
		}

		assert(false); // Do could not be executed

		return this;
	}

	DomainBuilder* DomainBuilder::Effect(const std::string& _name, EffectType _effectType, std::function <void(BaseContext*, EffectType)> _action)
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Primitive))
			{
				auto* primitiveTask = static_cast<HTN::PrimitiveTask*>(pointer);
				auto* effect = new ActionEffect(_name, _effectType, _action);
				primitiveTask->AddEffect(effect);
				return this;
			}
		}

		assert(false); // Effect can not be built

		return this;
	}

	DomainBuilder* DomainBuilder::End()
	{
		assert(!m_pointers.empty()); // Too many End() calls

		m_pointers.pop_back();
		return this;
	}

	DomainBuilder* DomainBuilder::Splice(Domain* _domain)
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Compound))
			{
				auto* compoundTask = static_cast<CompoundTaskInterface*>(pointer);
				m_domain->Add(compoundTask, _domain->GetRoot());
				return this;
			}
		}

		assert(false); // Pointer is not a compound task type. Did you forget an End()?

		return this;
	}

	DomainBuilder* DomainBuilder::Slot(int _slotId)
	{
		if (auto* pointer = GetPointer())
		{
			if (pointer->HasType(TaskType::Compound))
			{
				auto* compoundTask = static_cast<CompoundTaskInterface*>(pointer);
				auto* slot = new HTN::Slot();
				slot->SetSlotID(_slotId);
				const std::string name = "Slot " + _slotId;
				slot->SetName(name);
				m_domain->Add(compoundTask, slot);
				return this;
			}
		}

		assert(false); // Pointer is not a compound task type. Did you forget an End()?

		return this;
	}

}
