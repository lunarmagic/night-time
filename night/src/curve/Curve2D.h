#pragma once

#include "core.h"
#include "math/math.h"

namespace night
{
	struct NIGHT_API Curve2D
	{
		void insert(vec2 const& t, real val);
		void clear();

		// TODO: result is bugged if t0 and t1 are not set
		real interpolate(vec2 const& t) const;

		map<real, map<real, real>> map; // TODO: make struct into full wrapper

		static night::map<real, night::map<real, real>>::const_iterator upper_bound(night::map<real, night::map<real, real>> const& curve, real t);
		//static night::map<real, night::map<real, real>>::const_iterator lower_bound(night::map<real, night::map<real, real>> const& curve, real t);
	};
}