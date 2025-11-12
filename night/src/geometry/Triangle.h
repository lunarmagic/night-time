#pragma once

#include "core.h"
#include "Vertex.h"

namespace night
{

	template<typename T = real>
	struct NIGHT_API Triangle
	{
		array<Vertex<T>, 3> vertices;
	};

}