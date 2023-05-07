#include "FuncOperator.h"

namespace HTN
{
	FuncOperator::FuncOperator(std::function<TaskStatus(BaseContext*)> _func, std::function<void(BaseContext*)> _funcStop)
		: m_func(_func)
		, m_funcStop(_funcStop)
	{
	}

	TaskStatus FuncOperator::Update(BaseContext* _ctx)
	{
		return m_func != nullptr ? m_func(_ctx) : TaskStatus::Failure;
	}

	void FuncOperator::Stop(BaseContext* _ctx)
	{
		if (m_funcStop != nullptr)
		{
			m_funcStop(_ctx);
		}
	}

}
