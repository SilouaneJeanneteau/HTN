#pragma once

#include "ConditionInterface.h"

#include <functional>

namespace HTN
{
	class BaseContext;

	class FuncCondition : public ConditionInterface
	{
	public:
		FuncCondition(const std::string& _name, std::function<bool(BaseContext*)> _func);

		bool IsValid(BaseContext* _ctx) const override;

	private:
		std::function<bool(BaseContext*)> m_func;
	};

}
