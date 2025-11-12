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

		static night::map<real, real>::const_iterator upper_bound(night::map<real, real> const& curve, real t);
		//static night::map<real, real>::const_iterator lower_bound(night::map<real, real> const& curve, real t);
	};


}