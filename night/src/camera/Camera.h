#pragma once

#include "core.h"
#include "math/math.h"
#include "aabb/AABB.h"

#define NIGHT_CAMERA_DEFAULT_NEAR_CLIP 0.1f
#define NIGHT_CAMERA_DEFAULT_FAR_CLIP 100.0f
#define NIGHT_CAMERA_DEFAULT_FOV 35.0f

#define NIGHT_CAMERA_DEFAULT_DIRECTION FORWARD

namespace night
{
	enum ECameraType
	{
		Perspective = 0,
		Orthographic,
	};

	struct CameraTransform
	{
		vec3 translation{ ORIGIN };
		vec3 direction{ NIGHT_CAMERA_DEFAULT_DIRECTION };
	};

	struct NIGHT_API Camera
	{
		vec3 translation{ ORIGIN };
		vec3 look_at{ NIGHT_CAMERA_DEFAULT_DIRECTION };
		vec3 up{ UP };
		ECameraType type{ECameraType::Perspective};
		real fov{ NIGHT_CAMERA_DEFAULT_FOV };
		AABB ortho_region{ .left = -1, .right = 1, .top = 1, .bottom = -1 };
		real near_clip{ NIGHT_CAMERA_DEFAULT_NEAR_CLIP };
		real far_clip{ NIGHT_CAMERA_DEFAULT_FAR_CLIP };

		Camera operator*(CameraTransform const& x) const;

		vec3 direction() const;
		u8 should_cull_plane(vec3 const& point_on_plane, vec3 const& plane_normal) const;

		static vec3 triangle_normal(vec3 const& p1, vec3 const& p2, vec3 const& p3);
	};
}