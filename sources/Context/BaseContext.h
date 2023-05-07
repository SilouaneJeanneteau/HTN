#pragma once

#include "../HTNTypes.h"
#include "../HTNEnums.h"
#include "../Task/CompoundTask/GOAPNode.h"

#include <queue>
#include <stack>
#include <string>
#include <array>

namespace HTN
{
	class CompoundTaskInterface;

	struct PartialPlanEntry
	{
		CompoundTaskInterface* task = nullptr;
		int taskIndex = -1;
	};

	class BaseContext
	{
	public:
		BaseContext() : m_goapNodes(64) {}

		bool IsInitialized() const { return m_isInitialized; }
		bool IsDirty() const { return m_isDirty; }
		void SetIsDirty(bool _value) { m_isDirty = _value; }

		ContextState GetContextState() const { return m_contextState; }
		void SetContextState(ContextState _ctxState) { m_contextState = _ctxState; }

		int GetCurrentDecompositionDepth() const { return m_currentDecompositionDepth; }
		void SetCurrentDecompositionDepth(int _depth) { m_currentDecompositionDepth = _depth; }

		std::vector<int>& GetCurrentMTR() { return m_currentMTR; }
		void SetCurrentMTR(const std::vector<int>& _mtr) { m_currentMTR = _mtr; }

		std::vector<int>& GetLastMTR() { return m_lastMTR; }
		void SetLastMTR(const std::vector<int>& _mtr) { m_lastMTR = _mtr; }

		std::queue<PartialPlanEntry>& GetPartialPlanQueue() { return m_partialPlanQueue; }
		void SetPartialPlanQueue(const std::queue<PartialPlanEntry>& _partialPlan) { m_partialPlanQueue = _partialPlan; }

		bool HasPausedPartialPlan() const { return m_hasPausedPartialPlan; }
		void SetHasPausedPartialPlan(bool _paused) { m_hasPausedPartialPlan = _paused; }

		std::array<u8, static_cast<int>(WorldStateType::COUNT)>& GetWorldState() { return m_worldState; }

		std::array<std::stack<std::pair<EffectType, u8>>, static_cast<int>(WorldStateType::COUNT)>& GetWorldStateChangeStack() { return m_worldStateChangeStack; }

		void Reset();

		void TrimForExecution();
		void TrimToStackDepth(const std::vector<int>& _stackDepth);

		bool HasState(WorldStateType _state, u8 _value) const;
		bool HasState(int _state, u8 _value) const;
		u8 GetState(int _state) const;
		void SetState(WorldStateType _state, u8 _value, bool _setAsDirty = true, EffectType _effectType = EffectType::Permanent);
		void SetState(int _state, u8 _value, bool _setAsDirty = true, EffectType _effectType = EffectType::Permanent);

		std::vector<int> GetWorldStateChangeDepth() const;

		GOAPNode* CreateGOAPNode() { return m_goapNodes.New(); }
		void DeleteGOAPNode(GOAPNode* _node) { m_goapNodes.Delete(_node); }

#ifndef HTN_FINALE
		std::vector<std::string>& GetCurrentMTRDebug() { return m_currentMTRDebug; }
		void SetCurrentMTRDebug(const std::vector<std::string>& _mtrDebug) { m_currentMTRDebug = _mtrDebug; }

		std::vector<std::string>& GetLastMTRDebug() { return m_lastMTRDebug; }
		void SetLastMTRDebug(const std::vector<std::string>& _mtrDebug) { m_lastMTRDebug = _mtrDebug; }
#endif

	private:
		SmallPool<HTN::GOAPNode> m_goapNodes;
		std::vector<int> m_currentMTR;
		std::vector<int> m_lastMTR;
		std::queue<PartialPlanEntry> m_partialPlanQueue;
		ContextState m_contextState = ContextState::Executing;
		std::array<u8, static_cast<int>(WorldStateType::COUNT)> m_worldState;
		std::array<std::stack<std::pair<EffectType, u8>>, static_cast<int>(WorldStateType::COUNT)> m_worldStateChangeStack;
		int m_currentDecompositionDepth = -1;
		bool m_hasPausedPartialPlan = false;
		bool m_isInitialized = false;
		bool m_isDirty = false;

#ifndef HTN_FINALE
		std::vector<std::string> m_currentMTRDebug;
		std::vector<std::string> m_lastMTRDebug;
#endif
	};
}
