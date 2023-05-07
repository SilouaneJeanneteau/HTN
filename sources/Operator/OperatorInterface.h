#pragma once

#include "../HTNEnums.h"

namespace HTN
{
	class BaseContext;

	class OperatorInterface
	{
	public:
		virtual ~OperatorInterface() = default;

		virtual TaskStatus Update(BaseContext* _ctx) = 0;

		virtual void Stop(BaseContext* _ctx) = 0;
	};
}
