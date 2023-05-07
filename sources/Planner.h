#pragma once

#include "HTNEnums.h"

#include <queue>
#include <functional>

namespace HTN
{
	class ConditionInterface;
	class EffectInterface;
	class PrimitiveTask;
	class TaskInterface;
	class BaseContext;
	class Domain;

	class Planner
	{
	public:
		TaskStatus GetLastStatus() const { return m_lastStatus; }


		// NewPlan(newPlan) is called when we found a new plan, and there is no
		// old plan to replace.
		std::function<void(const std::queue<TaskInterface*>&)> NewPlan;

		// ReplacePlan(oldPlan, currentTask, newPlan) is called when we're about to replace the
		// current plan with a new plan.
		std::function<void(const std::queue<TaskInterface*>&, const TaskInterface*, const std::queue<TaskInterface*>&)> ReplacePlan;

		// NewTask(task) is called after we popped a new task off the current plan.
		std::function<void(const TaskInterface*)> NewTask;

		// NewTaskConditionFailed(task, failedCondition) is called when we failed to
		// validate a condition on a new task.
		std::function<void(const TaskInterface*, const ConditionInterface*)> NewTaskConditionFailed;

		// StopCurrentTask(task) is called when the currently running task was stopped
		// forcefully.
		std::function<void(const PrimitiveTask*)> StopCurrentTask;

		// CurrentTaskCompletedSuccessfully(task) is called when the currently running task
		// completes successfully, and before its effects are applied.
		std::function<void(const PrimitiveTask*)> CurrentTaskCompletedSuccessfully;

		// ApplyEffect(effect) is called for each effect of the type PlanAndExecute on a
		// completed task.
		std::function<void(const EffectInterface*)> ApplyEffect;

		// CurrentTaskFailed(task) is called when the currently running task fails to complete.
		std::function<void(const PrimitiveTask*)> CurrentTaskFailed;

		// CurrentTaskContinues(task) is called every tick that a currently running task
		// needs to continue.
		std::function<void(const PrimitiveTask*)> CurrentTaskContinues;

		// CurrentTaskExecutingConditionFailed(task, condition) is called if an Executing Condition
		// fails. The Executing Conditions are checked before every call to task.Operator.Update(...).
		std::function<void(const PrimitiveTask*, const ConditionInterface*)> CurrentTaskExecutingConditionFailed;

		void Tick(Domain* _domain, BaseContext* _ctx, bool _allowImmediateReplan = true);

		void Reset(BaseContext* _ctx);

		const std::queue<TaskInterface*>& GetPlan() const { return m_plan; }

		const TaskInterface* GetCurrentTask() const { return m_currentTask; }

	private:
		std::queue<TaskInterface*> m_plan;
		TaskInterface* m_currentTask = nullptr;
		TaskStatus m_lastStatus = TaskStatus::COUNT;
	};

}
