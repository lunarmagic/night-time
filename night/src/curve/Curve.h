#pragma once

#include "core.h"

namespace night
{
	struct NIGHT_API Curve
	{
		void insert(real t, real val);
		void clear();

		real interpolate(real t) const;

		map<real, real> map; // TODO: make struct into full wrapper
	};
}