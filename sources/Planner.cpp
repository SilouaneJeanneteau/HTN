#include "Planner.h"

#include "Task/TaskInterface.h"
#include "Task/PrimitiveTask/PrimitiveTask.h"
#include "Condition/ConditionInterface.h"
#include "Operator/OperatorInterface.h"
#include "Effect/EffectInterface.h"
#include "Context/BaseContext.h"
#include "Domain.h"

namespace HTN
{
	void Planner::Tick(Domain* _domain, BaseContext* _ctx, bool _allowImmediateReplan)
	{
		DecompositionStatus decompositionStatus = DecompositionStatus::Failed;
		bool isTryingToReplacePlan = false;
		auto& partialPlanQueue = _ctx->GetPartialPlanQueue();
		auto& lastMTR = _ctx->GetLastMTR();
		auto& currentMTR = _ctx->GetCurrentMTR();
#ifndef HTN_FINAL
		auto& lastMTRDebug = _ctx->GetLastMTRDebug();
		auto& currentMTRDebug = _ctx->GetCurrentMTRDebug();
#endif
		// Check whether state has changed or the current plan has finished running.
		// and if so, try to find a new plan.
		if ((m_currentTask == nullptr && m_plan.empty()) || _ctx->IsDirty())
		{
			bool hasSavedPreviousPartialPlan = false;
			std::queue<PartialPlanEntry> lastPartialPlanQueue;

			if (_ctx->IsDirty())
			{
				_ctx->SetIsDirty(false);
				// If we're simply re-evaluating whether to replace the current plan because
				// some world state got dirt, then we do not intend to continue a partial plan
				// right now, but rather see whether the world state changed to a degree where
				// we should pursue a better plan. Thus, if this replan fails to find a better
				// plan, we have to add back the partial plan temps cached above.
				if (_ctx->HasPausedPartialPlan())
				{
					hasSavedPreviousPartialPlan = true;
					_ctx->SetHasPausedPartialPlan(false);
					while (!partialPlanQueue.empty())
					{
						lastPartialPlanQueue.push(partialPlanQueue.front());
						partialPlanQueue.pop();
					}

					// We also need to ensure that the last MTR is up to date with the on-going MTR of the partial plan,
					// so that any new potential plan that is decomposing from the domain root has to beat the currently
					// running partial plan.
					lastMTR = currentMTR;

#ifndef HTN_FINAL
					lastMTRDebug = currentMTRDebug;
#endif
				}
			}
			
			std::queue<TaskInterface*> newPlan;
			decompositionStatus = _domain->FindPlan(_ctx, newPlan);
			isTryingToReplacePlan = !m_plan.empty();
			if (decompositionStatus == DecompositionStatus::Succeeded || decompositionStatus == DecompositionStatus::Partial)
			{
				if (isTryingToReplacePlan)
				{
					if (m_currentTask != nullptr)
					{
						if (ReplacePlan)
						{
							ReplacePlan(m_plan, m_currentTask, newPlan);
						}
					}
				}
				else
				{
					if (NewPlan)
					{
						NewPlan(newPlan);
					}
				}

				m_plan = {};
				while (!newPlan.empty())
				{
					m_plan.push(newPlan.front());
					newPlan.pop();
				}

				if (m_currentTask != nullptr && m_currentTask->HasType(TaskType::Primitive))
				{
					auto* primitiveTask = static_cast<PrimitiveTask*>(m_currentTask);
					if (StopCurrentTask)
					{
						StopCurrentTask(primitiveTask);
					}
					primitiveTask->Stop(_ctx);
					m_currentTask = nullptr;
				}

				// Copy the MTR into our LastMTR to represent the current plan's decomposition record
				// that must be beat to replace the plan.
				lastMTR = currentMTR;
#ifndef HTN_FINAL
				lastMTRDebug = currentMTRDebug;
#endif
			}
			else if (hasSavedPreviousPartialPlan)
			{
				_ctx->SetHasPausedPartialPlan(true);
				partialPlanQueue = {};
				while (!lastPartialPlanQueue.empty())
				{
					partialPlanQueue.push(lastPartialPlanQueue.front());
					lastPartialPlanQueue.pop();
				}

				if (!lastMTR.empty())
				{
					currentMTR = lastMTR;
					lastMTR = {};

#ifndef HTN_FINAL
					currentMTRDebug = lastMTRDebug;
					lastMTRDebug = {};
#endif
				}
			}
		}

		if (m_currentTask == nullptr && !m_plan.empty())
		{
			m_currentTask = m_plan.front();
			m_plan.pop();
			if (m_currentTask != nullptr)
			{
				if (NewTask)
				{
					NewTask(m_currentTask);
				}
				for (auto* condition : m_currentTask->GetConditions())
				{
					// If a condition failed, then the plan failed to progress! A replan is required.
					if (!condition->IsValid(_ctx))
					{
						if (NewTaskConditionFailed)
						{
							NewTaskConditionFailed(m_currentTask, condition);
						}

						m_currentTask = nullptr;
						m_plan = {};

						lastMTR = {};
#ifndef HTN_FINAL
						lastMTRDebug = {};
#endif

						_ctx->SetHasPausedPartialPlan(false);
						partialPlanQueue = {};
						_ctx->SetIsDirty(false);

						return;
					}
				}
			}
		}

		if (m_currentTask != nullptr)
		{
			if (m_currentTask->HasType(TaskType::Primitive))
			{
				const auto* primitiveTask = static_cast<PrimitiveTask*>(m_currentTask);
				if (auto* taskOperator = primitiveTask->GetOperator())
				{
					for (auto* condition : primitiveTask->GetExecutingConditions())
					{
						// If a condition failed, then the plan failed to progress! A replan is required.
						if (!condition->IsValid(_ctx))
						{
							if (CurrentTaskExecutingConditionFailed)
							{
								CurrentTaskExecutingConditionFailed(primitiveTask, condition);
							}

							m_currentTask = nullptr;
							m_plan = {};

							lastMTR = {};
#ifndef HTN_FINAL
							lastMTRDebug = {};
#endif

							_ctx->SetHasPausedPartialPlan(false);
							partialPlanQueue = {};
							_ctx->SetIsDirty(false);

							return;
						}
					}

					m_lastStatus = taskOperator->Update(_ctx);

					// If the operation finished successfully, we set task to null so that we dequeue the next task in the plan the following tick.
					if (m_lastStatus == TaskStatus::Success)
					{
						if (CurrentTaskCompletedSuccessfully)
						{
							CurrentTaskCompletedSuccessfully(primitiveTask);
						}

						// All effects that is a result of running this task should be applied when the task is a success.
						for (auto* effect : primitiveTask->GetEffects())
						{
							if (effect->GetType() == EffectType::PlanAndExecute)
							{
								ApplyEffect(effect);
								effect->Apply(_ctx);
							}
						}

						m_currentTask = nullptr;
						if (m_plan.empty())
						{
							lastMTR = {};
#ifndef HTN_FINAL
							lastMTRDebug = {};
#endif

							_ctx->SetIsDirty(false);

							if (_allowImmediateReplan)
							{
								Tick(_domain, _ctx, false);
							}
						}
					}
					// If the operation failed to finish, we need to fail the entire plan, so that we will replan the next tick.
					else if (m_lastStatus == TaskStatus::Failure)
					{
						if (CurrentTaskFailed)
						{
							CurrentTaskFailed(primitiveTask);
						}

						m_currentTask = nullptr;
						m_plan = {};

						lastMTR = {};
#ifndef HTN_FINAL
						lastMTRDebug = {};
#endif

						_ctx->SetHasPausedPartialPlan(false);
						partialPlanQueue = {};
						_ctx->SetIsDirty(false);
					}
					// Otherwise the operation isn't done yet and need to continue.
					else
					{
						if (CurrentTaskContinues)
						{
							CurrentTaskContinues(primitiveTask);
						}
					}
				}
				else
				{
					// This should not really happen if a domain is set up properly.
					m_currentTask = nullptr;
					m_lastStatus = TaskStatus::Failure;
				}
			}
		}

		if (m_currentTask == nullptr && m_plan.empty() && !isTryingToReplacePlan &&
			(decompositionStatus == DecompositionStatus::Failed ||
				decompositionStatus == DecompositionStatus::Rejected))
		{
			m_lastStatus = TaskStatus::Failure;
		}
	}

	void Planner::Reset(BaseContext* _ctx)
	{
		m_plan = {};

		if (m_currentTask != nullptr && m_currentTask->HasType(TaskType::Primitive))
		{
			static_cast<PrimitiveTask*>(m_currentTask)->Stop(_ctx);
		}
		m_currentTask = nullptr;
	}

}
