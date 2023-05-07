#include "GOAPSequence.h"

#include "../PrimitiveTask/PrimitiveTask.h"
#include "../../Context/BaseContext.h"
#include "../TaskInterface.h"
#include "../CompoundTask/GOAPNode.h"
#include "../GOAPTaskInterface.h"

namespace HTN
{
	GOAPSequence::GOAPSequence()
		: Sequence()
	{
		m_types.set(TaskType::GOAPSequence);
	}

	void GOAPSequence::AddGoalState(WorldStateType _state, u8 _value)
	{
		m_goal[_state] = _value;
	}

	DecompositionStatus GOAPSequence::OnDecompose(BaseContext* _ctx, int _startIndex, std::queue<TaskInterface*>& _result)
	{
		m_plan = {};

		std::vector<GOAPNode*> leaves;
		auto* start = _ctx->CreateGOAPNode();

		if (_TryBuildGraph(_ctx, start, leaves, m_subtasks))
		{
			_GeneratePlan(_ctx, _GetCheapestLeaf(leaves));
		}

		_ctx->DeleteGOAPNode(start);

		for (auto* leaf : leaves)
		{
			_FreeNode(_ctx, leaf);
		}

		_result = m_plan;
		return _result.empty() ? DecompositionStatus::Failed : DecompositionStatus::Succeeded;
	}

	bool GOAPSequence::_ValidatesGoal(BaseContext* _ctx)
	{
		for (auto& kvp : m_goal)
		{
			if (_ctx->GetState((int)kvp.first) != kvp.second)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<TaskInterface*> GOAPSequence::_GetSubset(BaseContext* _ctx, TaskInterface* _currentTask, const std::vector<TaskInterface*>& _tasks)
	{
		std::vector<TaskInterface*> subset;
		for (auto* task : _tasks)
		{
			if (task == _currentTask)
			{
				continue;
			}

			subset.push_back(task);
		}

		return subset;
	}

	GOAPNode* GOAPSequence::_GetCheapestLeaf(const std::vector<GOAPNode*> _leaves)
	{
		GOAPNode* cheapestLeaf = nullptr;
		for (auto* leaf : _leaves)
		{
			if (cheapestLeaf != nullptr)
			{
				if (leaf->runningCost < cheapestLeaf->runningCost)
				{
					cheapestLeaf = leaf;
				}
			}
			else
			{
				cheapestLeaf = leaf;
			}
		}
		return cheapestLeaf;
	}

	void GOAPSequence::_GeneratePlan(BaseContext* _ctx, GOAPNode* _node)
	{
		if (_node != nullptr && _node->task != nullptr)
		{
			_GeneratePlan(_ctx, _node->parent);

			_node->task->ApplyEffects(_ctx);
			m_plan.push(_node->task);
		}
	}

	void GOAPSequence::_FreeNode(BaseContext* _ctx, GOAPNode* _node)
	{
		auto* nextNode = _node->parent;
		if (nextNode->parent == nullptr)
		{
			nextNode = nullptr;
		}
		_ctx->DeleteGOAPNode(_node);
		if (nextNode != nullptr)
		{
			_FreeNode(_ctx, nextNode);
		}
	}

	bool GOAPSequence::_TryBuildGraph(BaseContext* _ctx, GOAPNode* _parent, std::vector<GOAPNode*>& _leaves, const std::vector<TaskInterface*>& _openSubtasks)
	{
		bool foundLeaf = false;

		for (auto* task : _openSubtasks)
		{
			if (!task->IsValid(_ctx))
			{
				continue;
			}

			if (task->HasType(TaskType::Primitive))
			{
				auto* primitiveTask = static_cast<PrimitiveTask*>(task);
				auto oldStackDepth = _ctx->GetWorldStateChangeDepth();

				primitiveTask->ApplyEffects(_ctx);

				auto* node = _ctx->CreateGOAPNode();
				node->parent = _parent;
				node->task = primitiveTask;
				if (task->HasType(TaskType::GOAP))
				{
					auto* goapTask = static_cast<GOAPTaskInterface*>(task);
					node->runningCost = _parent->runningCost + goapTask->Cost(_ctx);
				}
				else
				{
					node->runningCost = _parent->runningCost + 1.0f; // Default cost is 1 when task is not a GOAP Task.
				}


				if (_ValidatesGoal(_ctx))
				{
					_leaves.push_back(node);
					foundLeaf = true;
				}
				else
				{
					auto subset = _GetSubset(_ctx, task, _openSubtasks);
					if (_TryBuildGraph(_ctx, node, _leaves, subset))
					{
						foundLeaf = true;
					}
					else
					{
						// If we failed to find a valid branch for this node,
						// then it will no longer be referenced after this point.
						// Otherwise its still used as a parent reference in the
						// leaves list, and we can't return it to the pool yet.
						_ctx->DeleteGOAPNode(node);
					}
				}

				_ctx->TrimToStackDepth(oldStackDepth);
			}
		}

		return foundLeaf;
	}

}
