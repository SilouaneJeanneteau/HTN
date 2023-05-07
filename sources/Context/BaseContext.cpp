#include "BaseContext.h"

#include <cassert>

namespace HTN
{
	void BaseContext::Reset()
	{
		m_currentMTR.clear();
		m_lastMTR.clear();

#ifndef HTN_FINAL
		m_currentMTRDebug.clear();
		m_lastMTRDebug.clear();
#endif
	}

	void BaseContext::TrimForExecution()
	{
		assert(m_contextState != ContextState::Executing); // HTN BaseContext: Can not trim a context when in execution mode

		for (auto& stack : m_worldStateChangeStack)
		{
			while (stack.size() > 0 && stack.top().first != EffectType::Permanent)
			{
				stack.pop();
			}
		}
	}

	void BaseContext::TrimToStackDepth(const std::vector<int>& _stackDepth)
	{
		assert(m_contextState != ContextState::Executing); // HTN BaseContext: Can not trim a context when in execution mode
		assert(m_worldStateChangeStack.size() == _stackDepth.size()); // HTN BaseContext: WorldStateChangeStack size should be the same as stackDepth

		for (u32 index = 0u, count = _stackDepth.size(); index < count; ++index)
		{
			auto& stack = m_worldStateChangeStack[index];
			while (stack.size() > _stackDepth[index])
			{
				stack.pop();
			}
		}
	}

	bool BaseContext::HasState(WorldStateType _state, u8 _value) const
	{
		return GetState(static_cast<int>(_state)) == _value;
	}

	bool BaseContext::HasState(int _state, u8 _value) const
	{
		return GetState(_state) == _value;
	}

	u8 BaseContext::GetState(int _state) const
	{
		if (m_contextState == ContextState::Executing)
		{
			return m_worldState[_state];
		}

		if (m_worldStateChangeStack[_state].empty())
		{
			return m_worldState[_state];
		}

		return m_worldStateChangeStack[_state].top().second;
	}

	void BaseContext::SetState(WorldStateType _state, u8 _value, bool _setAsDirty, EffectType _effectType)
	{
		SetState(static_cast<int>(_state), _value, _setAsDirty, _effectType);
	}

	void BaseContext::SetState(int _state, u8 _value, bool _setAsDirty, EffectType _effectType)
	{
		if (m_contextState == ContextState::Executing)
		{
			// Prevent setting the world state dirty if we're not changing anything.
			if (m_worldState[_state] == _value)
			{
				return;
			}

			m_worldState[_state] = _value;
			if (_setAsDirty)
			{
				m_isDirty = true; // When a state change during execution, we need to mark the context dirty for replanning!
			}
		}
		else
		{
			m_worldStateChangeStack[_state].push({ _effectType, _value });
		}
	}

	std::vector<int> BaseContext::GetWorldStateChangeDepth() const
	{
		std::vector<int> worldStateChangeDepth;
		worldStateChangeDepth.resize(m_worldStateChangeStack.size());
		u32 index = 0u;
		for (const auto& stack : m_worldStateChangeStack)
		{
			worldStateChangeDepth[index++] = stack.size();
		}
		return worldStateChangeDepth;
	}

}
