
#include "nightpch.h"
#include "Curve.h"
#include "math/math.h"

namespace night
{

	void Curve::insert(real t, real val)
	{
		this->map.insert({ t, val });
	}

	void Curve::clear()
	{
		this->map.clear();
	}

	real Curve::interpolate(real t) const
	{
		if (this->map.empty())
		{
			return 0;
		}

		if (t == 1.0f)
		{
			return (*(std::prev(this->map.end()))).second;
		}

		//auto lb = score_curve.lower_bound(t);
		auto ub = this->map.upper_bound(t);
		auto lb = std::prev(ub);
		if (lb == this->map.end() || ub == this->map.end())
		{
			return 0;
		}

		auto& [lt, lw] = *lb;
		auto& [ut, uw] = *ub;

		if (lt == ut)
		{
			return lw;
		}

		real t2 = (t - lt) / (ut - lt);
		real result = lerp(lw, uw, t2);
		return result;
	}

}