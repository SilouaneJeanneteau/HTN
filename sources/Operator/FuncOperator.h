#pragma once

#include "OperatorInterface.h"

#include <functional>

namespace HTN
{
	class BaseContext;

	class FuncOperator : public OperatorInterface
	{
	public:
		FuncOperator(std::function<TaskStatus(BaseContext*)> _func, std::function<void(BaseContext*)> _funcStop = nullptr);

		TaskStatus Update(BaseContext* _ctx) override;
		void Stop(BaseContext* _ctx) override;

	private:
		std::function<TaskStatus(BaseContext*)> m_func;
		std::function<void(BaseContext*)> m_funcStop;
	};

}
