
#include "nightpch.h"
#include "Curve2D.h"
#include "Curve.h"

namespace night
{

	void Curve2D::insert(vec2 const& t, real val)
	{
		//this->map.insert({ t, val });
		this->map[t.x].insert({ t.y, val });
	}

	void Curve2D::clear()
	{
		this->map.clear();
	}

	real Curve2D::interpolate(vec2 const& t) const
	{
		real wlt = 0.0f;
		real wut = 1.0f;

		auto wub = Curve2D::upper_bound(map, t.y);

		night::map<real, night::map<real, real>>::const_iterator wlb;

		if (wub == map.begin())
		{
			wlb = map.end();
		}
		else
		{
			wlb = std::prev(wub);
		}
		
		if (wlb != map.end())
		{
			wlt = (*wlb).first;
		}

		if (wub != map.end())
		{
			wut = (*wub).first;
		}

		real wt = math::ilerp(wlt, wut, t.y);

		auto interpolate_z = [this, &t](night::map<real, night::map<real, real>>::const_iterator it) -> real
			{
				real zt = 0.0f;
				real zw = 0.0f;

				if (it != map.end())
				{
					real zlt = 0.0f;
					real zut = 1.0f;
					real zlw = 0.0f;
					real zuw = 0.0f;

					auto zub = Curve::upper_bound((*it).second, t.x);

					night::map<real, real>::const_iterator zlb;

					if (zub == (*it).second.begin())
					{
						zlb = (*it).second.end();
					}
					else
					{
						zlb = std::prev(zub);
					}

					if (zlb != (*it).second.end())
					{
						zlt = (*zlb).first;
						zlw = (*zlb).second;
					}

					if (zub != (*it).second.end())
					{
						zut = (*zub).first;
						zuw = (*zub).second;
					}

					zt = math::ilerp(zlt, zut, t.x);
					zw = math::lerp(zlw, zuw, zt);
					return zw;
				}

				return 0.0f;
			};

		return math::lerp(interpolate_z(wlb), interpolate_z(wub), wt);
	}

	night::map<real, night::map<real, real>>::const_iterator Curve2D::upper_bound(night::map<real, night::map<real, real>> const& curve, real t)
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
	night::map<real, night::map<real, real>>::const_iterator Curve2D::lower_bound(night::map<real, night::map<real, real>> const& curve, real t)
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