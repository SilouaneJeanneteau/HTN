#pragma once

#include "../TaskInterface.h"

namespace HTN
{
	class ConditionInterface;
	class BaseContext;

	class PausePlanTask : public TaskInterface
	{
	public:
		PausePlanTask() : TaskInterface(TaskType::PausePlan) {}

		TaskInterface* AddCondition(ConditionInterface* _condition) override { return this; }

		bool IsValid(BaseContext* _ctx) const override { return true; }

		DecompositionStatus OnIsValidFailed(BaseContext* _ctx) override { return DecompositionStatus::Failed; }
	};
}
