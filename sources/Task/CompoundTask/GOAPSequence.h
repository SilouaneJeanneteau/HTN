#pragma once

#include "Sequence.h"

#include <queue>
#include <map>

namespace HTN
{
	class TaskInterface;
	class BaseContext;
	struct GOAPNode;

	class GOAPSequence : public Sequence
	{
	public:
		GOAPSequence();

		void AddGoalState(WorldStateType _state, u8 _value);
		DecompositionStatus OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result) override;

	private:
		std::vector<TaskInterface*> _GetSubset(BaseContext* _ctx, TaskInterface* _currentTask, const std::vector<TaskInterface*>& _tasks);
		bool _ValidatesGoal(BaseContext* _ctx);
		GOAPNode* _GetCheapestLeaf(const std::vector<GOAPNode*> _leaves);
		void _GeneratePlan(BaseContext* _ctx, GOAPNode* _node);
		void _FreeNode(BaseContext* _ctx, GOAPNode* _node);
		bool _TryBuildGraph(BaseContext* _ctx, GOAPNode* _parent, std::vector<GOAPNode*>& _leaves, const std::vector<TaskInterface*>& _openSubtasks);

		std::map<WorldStateType, u8> m_goal;
	};

}
