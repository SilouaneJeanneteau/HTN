#pragma once

#include <string>

namespace HTN
{
	class BaseContext;

	class ConditionInterface
	{
	public:
		ConditionInterface(const std::string& _name)
			: m_name(_name)
		{
		}

		virtual ~ConditionInterface() = default;

		const std::string& GetName() const { return m_name; }

		virtual bool IsValid(BaseContext* _ctx) const = 0;

	protected:
		std::string m_name;
	};
}
