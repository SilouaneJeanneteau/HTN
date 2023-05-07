#pragma once

#include "EffectInterface.h"

#include <functional>

namespace HTN
{
	class BaseContext;

	class ActionEffect : public EffectInterface
	{
	public:
		ActionEffect(const std::string& _name, EffectType _type, std::function<void(BaseContext*, EffectType)> _action);

		void Apply(BaseContext* _ctx) override;

	private:
		std::function<void(BaseContext*, EffectType)> m_action;
	};

}
