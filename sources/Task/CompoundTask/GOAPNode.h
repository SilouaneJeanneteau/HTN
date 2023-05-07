#pragma once

namespace HTN
{
	class PrimitiveTask;
	struct GOAPNode
	{
		~GOAPNode()
		{
			runningCost = 0.0f;
			parent = nullptr;
			task = nullptr;
		}

		float runningCost = 0.0f;
		GOAPNode* parent = nullptr;
		PrimitiveTask* task = nullptr;
	};
}
