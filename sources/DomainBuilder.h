#pragma once

#include "HTNTypes.h"
#include "Domain.h"
#include "Task/TaskInterface.h"

#include <functional>

namespace HTN
{
	class CompoundTaskInterface;
	class TaskInterface;
	class BaseContext;

	class DomainBuilder
	{
	public:
		DomainBuilder(const std::string& _domainName);
		~DomainBuilder();

		TaskInterface* GetPointer();

		template<class T>
		DomainBuilder* CompoundTask(const std::string& _name, T* _task)
		{
			if (_task != nullptr)
			{
				if (auto* pointer = GetPointer())
				{
					if (pointer->HasType(TaskType::Compound))
					{
						auto* compoundTask = static_cast<CompoundTaskInterface*>(pointer);
						_task->SetName(_name);
						m_domain->Add(compoundTask, _task);
						m_pointers.push_back(_task);
						return this;
					}
				}
			}

			assert(false); // CompoundTask can not be built

			return this;
		}

		template<class T>
		DomainBuilder* CompoundTask(const std::string& _name)
		{
			return CompoundTask(_name, new T());
		}

		DomainBuilder* PrimitiveTask(const std::string& _name);
		DomainBuilder* PausePlanTask();
		DomainBuilder* Sequence(const std::string& _name);
		DomainBuilder* Select(const std::string& _name);
		DomainBuilder* GOAPSequence(const std::string& _name, const std::map<WorldStateType, u8>& _goal);

		template<class T>
		DomainBuilder* GOAPAction(const std::string& _name)
		{
			if (auto* pointer = GetPointer())
			{
				if (pointer->HasType(TaskType::Compound))
				{
					auto* compoundTask = static_cast<CompoundTaskInterface*>(pointer);
					auto* subTask = new T();
					subTask->SetName(_name);
					m_domain->Add(compoundTask, subTask);
					m_pointers.push_back(subTask);
					return this;
				}
			}

			assert(false); // Pointer is not a GOAP Sequence, which is required for adding GOAP Actions!
			return this;
		}

		DomainBuilder* Condition(const std::string& _name, std::function<bool(BaseContext*)> _condition);
		DomainBuilder* ExecutingCondition(const std::string& _name, std::function<bool(BaseContext*)> _condition);
		DomainBuilder* Do(std::function<TaskStatus(BaseContext*)> _action, std::function <void(BaseContext*)> _forceStopAction = nullptr);
		DomainBuilder* Effect(const std::string& _name, EffectType effectType, std::function<void(BaseContext*, EffectType)> _action);
		DomainBuilder* End();
		DomainBuilder* Splice(Domain* _domain);
		DomainBuilder* Slot(int _slotId);
		Domain* RetrieveDomain() { m_domainOwner = false; return m_domain; }
	private:
		bool m_domainOwner = true;
		Domain* m_domain = nullptr;
		std::vector<TaskInterface*> m_pointers;
	};
}
