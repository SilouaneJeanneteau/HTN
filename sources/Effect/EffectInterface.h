#pragma once

#include "../HTNEnums.h"

#include <string>

namespace HTN
{
	class BaseContext;

	class EffectInterface
	{
	public:
		EffectInterface(const std::string& _name, EffectType _type)
			: m_name(_name)
			, m_type(_type)
		{
		}

		virtual ~EffectInterface() = default;

		const std::string& GetName() const { return m_name; }
		EffectType GetType() const { return m_type; }
		virtual void Apply(BaseContext* _ctx) = 0;

	protected:
		std::string m_name;
		EffectType m_type;
	};
}
