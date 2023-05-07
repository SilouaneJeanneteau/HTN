#pragma once

enum class DecompositionStatus
{
	Succeeded,
	Partial,
	Failed,
	Rejected,
	COUNT
};

enum class TaskStatus
{
	Continue,
	Success,
	Failure,
	COUNT
};

enum class ContextState
{
	Planning,
	Executing,
	COUNT
};

enum class EffectType
{
	PlanAndExecute,
	PlanOnly,
	Permanent,
	COUNT
};

enum class WorldStateType : int
{
	HasA,
	HasB,
	HasC,
	COUNT
};

enum class TaskType
{
	Primitive,
	Compound,
	Slot,
	PausePlan,
	GOAP,
	GOAPSequence,
	Sequence,
	Selector,
	COUNT
};

