
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
		if (map.empty())
		{
			return 0;
		}

		auto ub = Curve::upper_bound(map, t);

		night::map<real, real>::const_iterator lb;

		if (ub == map.begin())
		{
			lb = map.end();
		}
		else
		{
			lb = std::prev(ub);
		}

		real lt = 0.0f;
		real ut = 1.0f;
		real lw = 0.0f;
		real uw = 0.0f;

		if (lb != map.end())
		{
			lt = (*lb).first;
			lw = (*lb).second;
		}

		if (ub != map.end())
		{
			ut = (*ub).first;
			uw = (*ub).second;
		}

		if (lt == ut)
		{
			return lw;
		}

		real ilt = math::ilerp(lt, ut, t);
		real result = math::lerp(lw, uw, ilt);
		return result;
	}

	night::map<real, real>::const_iterator Curve::upper_bound(night::map<real, real> const& curve, real t)
	{
		auto it = curve.begin();

		if (it != curve.end())
		{
			if ((*it).first > t)
			{
				return it;
			}

			while (true)
			{
				it++;
				if (it == curve.end())
				{
					return curve.end();
				}

				if ((*it).first > t)
				{
					break;
				}
			}

			if ((*it).first < t)
			{
				return curve.end();
			}
		}

		return it;
	}

#if 0
	night::map<real, real>::const_iterator Curve::lower_bound(night::map<real, real> const& curve, real t)
	{
		auto it = curve.begin();

		if (it != curve.end())
		{
			if ((*it).first > t)
			{
				return curve.end();
			}

			while (true)
			{
				auto next = std::next(it);
				if (next == curve.end() || ((*it).first < t && (*next).first >= t))
				{
					break;
				}

				it = next;
			}
		}

		return it;
	}
#endif

}