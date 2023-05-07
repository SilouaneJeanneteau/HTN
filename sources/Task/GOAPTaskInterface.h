#pragma once

#include "../HTNTypes.h"
#include "PrimitiveTask/PrimitiveTask.h"

namespace HTN
{
	class BaseContext;

	class GOAPTaskInterface : public PrimitiveTask
	{
	public:
		GOAPTaskInterface()
			: PrimitiveTask()
		{
			m_types.set(TaskType::GOAP);
		}

		virtual float Cost(BaseContext* _ctx) = 0;
	};
}
