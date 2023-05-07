#include "FuncCondition.h"

namespace HTN
{
	FuncCondition::FuncCondition(const std::string& _name, std::function<bool(BaseContext*)> _func)
		: ConditionInterface(_name)
	{
		m_func = _func;
	}

	bool FuncCondition::IsValid(BaseContext* _ctx) const
	{
		return m_func != nullptr ? m_func(_ctx) : false;
	}
}
