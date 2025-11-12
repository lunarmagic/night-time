#pragma once

#include "nightpch.h"
//#include "log/log.h"
//#include "types/types.h"
//#include "stl/stl.h"
//#include "math/math.h"
#include "node/NodeGui.h"
#include "profiler/Profiler.h"

namespace night
{
	struct ExampleBase : public NodeRenderable
	{
		//ExampleBase();
		virtual void update_gui(handle<NodeGui> gui) = 0;
	};
}