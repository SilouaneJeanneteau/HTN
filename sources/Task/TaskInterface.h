#pragma once

#include "../HTNEnums.h"
#include "../HTNTypes.h"
#include "../Condition/ConditionInterface.h"

#include <string>
#include <vector>

namespace HTN
{
	class CompoundTaskInterface;
	class BaseContext;

	class TaskInterface
	{
	public:
		TaskInterface(TaskType _type) : m_types({ _type }) {}

		virtual ~TaskInterface()
		{
			for (auto* cond : m_conditions)
			{
				delete cond;
			}
		}

		const std::string& GetName() const { return m_name; }
		void SetName(const std::string& _name) { m_name = _name; }

		CompoundTaskInterface* GetParent() const { return m_parent; }
		void SetParent(CompoundTaskInterface* _parent) { m_parent = _parent; }

		const std::vector<ConditionInterface*>& GetConditions() const { return m_conditions; }

		TaskStatus GetLastStatus() const { return m_lastStatus; }

		virtual TaskInterface* AddCondition(ConditionInterface* _condition) = 0;

		virtual bool IsValid(BaseContext* _ctx) const = 0;

		virtual DecompositionStatus OnIsValidFailed(BaseContext* _ctx) = 0;

		bool HasType(TaskType _type) { return m_types.test(_type); }

	protected:
		BitFieldEnum<TaskType> m_types;
		std::string m_name;
		CompoundTaskInterface* m_parent = nullptr;
		TaskStatus m_lastStatus = TaskStatus::COUNT;
		std::vector<ConditionInterface*> m_conditions;
	};
}
