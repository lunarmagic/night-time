
#include "nightpch.h"
#include "Camera.h"

namespace night
{

	vec3 Camera::direction() const
	{
		return normalize(look_at - translation);
	}

	u8 Camera::should_cull_plane(vec3 const& point_on_plane, vec3 const& plane_normal) const
	{
		if (type == ECameraType::Perspective)
		{
			vec3 ab = point_on_plane - translation;
			real d = dot(plane_normal, ab);
			return (d > 0);
		}
		else if (type == ECameraType::Orthographic)
		{
			return (dot(plane_normal, direction()) > 0);
		}

		return false;
	}

	vec3 Camera::triangle_normal(vec3 const& p1, vec3 const& p2, vec3 const& p3)
	{
		return normalize(cross(p2 - p1, p3 - p2));
	}

	Camera Camera::operator*(CameraTransform const& x) const
	{
		Camera result = *this;
		vec3 direction = this->direction();
		vec3 rhs_direction = x.direction;

		auto rotation = rotate_about_vector(NIGHT_CAMERA_DEFAULT_DIRECTION, rhs_direction);
		vec3 new_direction = rotation * vec4(direction, 1);
		result.translation = this->translation + x.translation;
		result.look_at = result.translation + new_direction;
		return result;
	}

}