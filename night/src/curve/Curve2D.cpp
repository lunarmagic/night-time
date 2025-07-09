
#include "nightpch.h"
#include "Curve2D.h"

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
		if (this->map.empty())
		{
			return 0;
		}

		auto wub = this->map.upper_bound(t.y);
		night::map<real, night::map<real, real>>::const_iterator wlb;

		if (wub == this->map.end() || wub == this->map.begin())
		{
			wlb = wub;
		}
		else
		{
			wlb = std::prev(wub);
		}

		if (wub == this->map.end() && wlb == this->map.end())
		{
			return 0;
		}

		if ((*wub).first - (*wlb).first == 0)
		{
			return 0;
		}

		real wt = (t.y - (*wlb).first) / ((*wub).first - (*wlb).first);

		auto wub_zub = ((*wub).second.upper_bound(t.x));
		night::map<real, real>::const_iterator wub_zlb;

		if (wub_zub == (*wub).second.end() || wub_zub == (*wub).second.begin())
		{
			wub_zlb = wub_zub;
		}
		else
		{
			wub_zlb = std::prev(wub_zub);
		}

		auto wlb_zub = ((*wlb).second.upper_bound(t.x));

		night::map<real, real>::const_iterator wlb_zlb;

		if (wlb_zub == (*wlb).second.end() || wlb_zub == (*wlb).second.begin())
		{
			wlb_zlb = wlb_zub;
		}
		else
		{
			wlb_zlb = std::prev(wlb_zub);
		}

		u8 wub_zub_x = !(wub_zub == (*wub).second.end());
		u8 wub_zlb_x = !(wub_zlb == (*wub).second.end());
		u8 wlb_zub_x = !(wlb_zub == (*wlb).second.end());
		u8 wlb_zlb_x = !(wlb_zlb == (*wlb).second.end());

		real wub_zut;
		real wub_zlt;
		real wlb_zut;
		real wlb_zlt;

		if (!wub_zub_x && !wub_zlb_x)
		{
			if (!wlb_zub_x && !wlb_zlb_x)
			{
				return 0;
			}
			else if (!wlb_zub_x && wlb_zlb_x)
			{
				wlb_zub = wlb_zlb;
			}
			else if (wlb_zub_x && !wlb_zlb_x)
			{
				wlb_zlb = wlb_zub;
			}

			wlb_zut = (*wlb_zub).first;
			wlb_zlt = (*wlb_zlb).first;

			if (wlb_zlt == wlb_zut)
			{
				return (*wlb_zub).second;
			}

			real wlb_zt = (t.x - wlb_zlt) / (wlb_zut - wlb_zlt);
			real result = lerp((*wlb_zlb).second, (*wlb_zub).second, wlb_zt);

			return result;
		}
		else if (!wub_zub_x && wub_zlb_x)
		{
			wub_zub = wub_zlb;
		}
		else if (wub_zub_x && !wub_zlb_x)
		{
			wub_zlb = wub_zub;
		}

		if (!wlb_zub_x && !wlb_zlb_x)
		{
			if (!wub_zub_x && !wub_zlb_x)
			{
				return 0;
			}
			else if (!wub_zub_x && wub_zlb_x)
			{
				wub_zub = wub_zlb;
			}
			else if (wub_zub_x && !wub_zlb_x)
			{
				wub_zlb = wub_zub;
			}

			wub_zut = (*wub_zub).first;
			wub_zlt = (*wub_zlb).first;

			if (wub_zlt == wub_zut)
			{
				return (*wub_zub).second;
			}

			real wub_zt = (t.x - wub_zlt) / (wub_zut - wub_zlt);
			real result = lerp((*wub_zlb).second, (*wub_zub).second, wub_zt);

			return result;
		}
		else if (!wlb_zub_x && wlb_zlb_x)
		{
			wlb_zub = wlb_zlb;
		}
		else if (wlb_zub_x && !wlb_zlb_x)
		{
			wlb_zlb = wlb_zub;
		}


		wub_zut = (*wub_zub).first;
		wub_zlt = (*wub_zlb).first;
		wlb_zut = (*wlb_zub).first;
		wlb_zlt = (*wlb_zlb).first;

		real wub_zt;
		real wlb_zt;

		if (wub_zlt == wub_zut)
		{
			wub_zt = (*wub_zub).second;
		}
		else
		{
			wub_zt = (t.x - wub_zlt) / (wub_zut - wub_zlt);
		}

		if (wlb_zlt == wlb_zut)
		{
			wlb_zt = (*wlb_zub).second;
		}
		else
		{
			wlb_zt = (t.x - wlb_zlt) / (wlb_zut - wlb_zlt);
		}

		real lr = lerp((*wlb_zlb).second, (*wlb_zub).second, wlb_zt);
		real ur = lerp((*wub_zlb).second, (*wub_zub).second, wub_zt);
		real result = lerp(lr, ur, wt);

		return result;
	}

}