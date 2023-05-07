#pragma once

#include "../TaskInterface.h"

namespace HTN
{
	class ConditionInterface;
	class OperatorInterface;
	class EffectInterface;
	class BaseContext;

	class PrimitiveTask : public TaskInterface
	{
	public:
		PrimitiveTask() : TaskInterface(TaskType::Primitive) {}
		~PrimitiveTask() override;

		const std::vector<const ConditionInterface*>& GetExecutingConditions() const { return m_executingConditions; }

		TaskInterface* AddCondition(ConditionInterface* _condition) override;

		OperatorInterface* GetOperator() const { return m_operator; }
		void SetOperator(OperatorInterface* _operator) { m_operator = _operator; }

		const std::vector<EffectInterface*>& GetEffects() const { return m_effects; }

		virtual TaskInterface* AddEffect(EffectInterface* _effect);

		bool IsValid(BaseContext* _ctx) const override;

		DecompositionStatus OnIsValidFailed(BaseContext* _ctx) override;

		virtual TaskInterface* AddExecutingCondition(const ConditionInterface* _condition);

		virtual void ApplyEffects(BaseContext* _ctx);

		virtual void Stop(BaseContext* _ctx);

	protected:
		OperatorInterface* m_operator = nullptr;
		std::vector<EffectInterface*> m_effects;
		std::vector<const ConditionInterface*> m_executingConditions;
	};

}
