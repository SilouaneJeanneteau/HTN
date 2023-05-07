#include "PrimitiveTask.h"

#include "../../Condition/ConditionInterface.h"
#include "../../Operator/OperatorInterface.h"
#include "../../Effect/EffectInterface.h"

namespace HTN
{
	PrimitiveTask::~PrimitiveTask()
	{
		delete m_operator;

		for (auto* effect : m_effects)
		{
			delete effect;
		}

		for (auto* cond : m_executingConditions)
		{
			delete cond;
		}
	}

	TaskInterface* PrimitiveTask::AddCondition(ConditionInterface* _condition)
	{
		m_conditions.push_back(_condition);
		return this;
	}

	TaskInterface* PrimitiveTask::AddEffect(EffectInterface* _effect)
	{
		m_effects.push_back(_effect);
		return this;
	}

	bool PrimitiveTask::IsValid(BaseContext* _ctx) const
	{
		for (const auto* cond : m_conditions)
		{
			if (!cond->IsValid(_ctx))
			{
				return false;
			}
		}

		return true;
	}

	DecompositionStatus PrimitiveTask::OnIsValidFailed(BaseContext* _ctx)
	{
		return DecompositionStatus::Failed;
	}

	TaskInterface* PrimitiveTask::AddExecutingCondition(const ConditionInterface* _condition)
	{
		m_executingConditions.push_back(_condition);
		return this;
	}

	void PrimitiveTask::Stop(BaseContext* _ctx)
	{
		m_operator->Stop(_ctx);
	}

	void PrimitiveTask::ApplyEffects(BaseContext* _ctx)
	{
		for (auto* effect : m_effects)
		{
			effect->Apply(_ctx);
		}
	}

}
