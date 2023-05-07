#include "ActionEffect.h"

namespace HTN
{
	ActionEffect::ActionEffect(const std::string& _name, EffectType _type, std::function<void(BaseContext*, EffectType)> _action)
		: EffectInterface(_name, _type)
	{
		m_action = _action;
	}

	void ActionEffect::Apply(BaseContext* _ctx)
	{
		if (m_action != nullptr)
		{
			m_action(_ctx, m_type);
		}
	}

}
