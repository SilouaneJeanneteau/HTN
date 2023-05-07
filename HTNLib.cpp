#include <iostream>

#include "sources/HTNTypes.h"
#include "sources/Domain.h"
#include "sources/Task/CompoundTask/CompoundTaskInterface.h"
#include "sources/Task/Slot.h"
#include "sources/Task/CompoundTask/Selector.h"
#include "sources/Task/GOAPTaskInterface.h"
#include "sources/Context/BaseContext.h"
#include "sources/Planner.h"
#include "sources/DomainBuilder.h"
#include "sources/Effect/EffectInterface.h"

int main()
{
    std::cout << "Build HTN\n";

	class GOAPTaskAction : public HTN::GOAPTaskInterface
	{
		virtual float Cost(HTN::BaseContext* _ctx)
		{
			return 1.0f;
		}
	};

	class GOAPTaskActionHighCost : public HTN::GOAPTaskInterface
	{
		virtual float Cost(HTN::BaseContext* _ctx)
		{
			return 10.0f;
		}
	};

	auto funcHasAOrB = [](HTN::BaseContext* _ctx) { return _ctx->HasState(WorldStateType::HasA, 1u) || _ctx->HasState(WorldStateType::HasB, 1u); };
	auto funcHasAAndB = [](HTN::BaseContext* _ctx) { return _ctx->HasState(WorldStateType::HasA, 1u) && _ctx->HasState(WorldStateType::HasB, 1u); };
	auto funcHasA = [](HTN::BaseContext* _ctx) { return _ctx->HasState(WorldStateType::HasA, 1u); };
	auto funcHasNotA = [](HTN::BaseContext* _ctx) { return !_ctx->HasState(WorldStateType::HasA, 1u); };
	auto funcHasB = [](HTN::BaseContext* _ctx) { return _ctx->HasState(WorldStateType::HasB, 1u); };
	auto funcHasNotB = [](HTN::BaseContext* _ctx) { return !_ctx->HasState(WorldStateType::HasB, 1u); };
	auto funcHasNotC = [](HTN::BaseContext* _ctx) { return !_ctx->HasState(WorldStateType::HasC, 1u); };
	auto funcGetAPermanent = [](HTN::BaseContext* _ctx) { _ctx->SetState(WorldStateType::HasA, 1u); return TaskStatus::Success; };
	auto funcGetCPermanent = [](HTN::BaseContext* _ctx) { _ctx->SetState(WorldStateType::HasC, 1u); return TaskStatus::Success; };
	auto funcGetBPermanent = [](HTN::BaseContext* _ctx) { _ctx->SetState(WorldStateType::HasB, 1u); return TaskStatus::Success; };
	auto funcGetCEffect = [](HTN::BaseContext* _ctx, EffectType _type) { _ctx->SetState(WorldStateType::HasC, 1u, true, _type); };
	auto funcGetAEffect = [](HTN::BaseContext* _ctx, EffectType _type) { _ctx->SetState(WorldStateType::HasA, 1u, true, _type); };
	auto funcGetBEffect = [](HTN::BaseContext* _ctx, EffectType _type) { _ctx->SetState(WorldStateType::HasB, 1u, true, _type); };
	std::map<WorldStateType, HTN::u8> goal;
	goal[WorldStateType::HasC] = 1u;
	auto* domain = HTN::DomainBuilder("My Domain")
		.GOAPSequence("GOAP Sequence", goal)
		->GOAPAction<GOAPTaskAction>("Get C")
		->Condition("Has A and B", funcHasAAndB)
		->Condition("Has Not C", funcHasNotC)
		->Do(funcGetCPermanent)
		->Effect("Has C", EffectType::PlanOnly, funcGetCEffect)
		->End()
		->GOAPAction<GOAPTaskActionHighCost>("Get B")
		->Condition("Has Not B", funcHasNotB)
		->Do(funcGetBPermanent)
		->Effect("Has B", EffectType::PlanOnly, funcGetBEffect)
		->End()
		->GOAPAction<GOAPTaskAction>("Get A")
		->Condition("Has not A", funcHasNotA)
		->Do(funcGetAPermanent)
		->Effect("Has A", EffectType::PlanOnly, funcGetAEffect)
		->End()
		->End()
		->RetrieveDomain();

	HTN::BaseContext ctx;
	HTN::Planner planner;

	auto displayTask = [](const HTN::TaskInterface* _task)
	{
		std::cout << "Task " << _task->GetName() << std::endl;
	};

	planner.NewTask = displayTask;

	do
	{
		planner.Tick(domain, &ctx, false);
	} while (!planner.GetPlan().empty());

	delete domain;
}
