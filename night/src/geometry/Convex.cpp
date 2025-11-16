
#include "nightpch.h"
#include "Convex.h"
//#include "log/log.h"

namespace night
{

	//u8 Convex::is_clockwise(vector<vec2> const& convex)
	//{
	//	for (s32 i = 0; i < convex.size(); i++)
	//	{
	//		vec2 const& p1 = convex[i];
	//		vec2 const& p2 = convex[(i + 1) % convex.size()];
	//		vec2 const& p3 = convex[(i + 2) % convex.size()];

	//		EOrientation o = orientation(p1, p2, p3);

	//		if (o == EOrientation::CounterClockwise)
	//		{
	//			return false;
	//		}
	//	}

	//	return true;
	//}

	//vector<vec2> Convex::make_clockwise(vector<vec2> const& convex)
	//{
	//	vector<vec2> result;
	//
	//	vec2 center = vec2(0);
	//	s32 count = 0;
	//
	//	for (const auto& i : convex)
	//	{
	//		center += i;
	//		count++;
	//	}
	//
	//	if (count == 0)
	//	{
	//		WARNING("count is 0 / out_approx is empty");
	//		return result;
	//	}
	//
	//	center /= count;
	//
	//	struct Angle
	//	{
	//		real angle;
	//		s32 index;
	//		vec2 point;
	//	};
	//
	//	vector<Angle> angles;
	//
	//	for (s32 i = 0; i < convex.size(); i++)
	//	{
	//		Angle angle;
	//		angle.angle = angle_clockwise(vec2(0, 1), math::normalize(convex[i] - center));
	//		angle.index = i;
	//		angle.point = convex[i];
	//		angles.push_back(angle);
	//	}
	//
	//	std::sort(angles.begin(), angles.end(), [](auto& a, auto& b) -> u8 {return a.angle < b.angle; });
	//
	//	for (s32 i = 0; i < angles.size(); i++)
	//	{
	//		result.push_back(angles[i].point);
	//	}
	//
	//	return result;
	//}

}