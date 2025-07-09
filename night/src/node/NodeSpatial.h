#pragma once

#include "INode.h"
#include "math/math.h"

namespace night
{
	struct NIGHT_API NodeSpatial : public INode
	{
		mat4 const& local_transform() const;
		void local_transform(mat4 const& x);

		void local_translate(const vec3& translation);
		void local_rotate(const vec3& axis, real radians);

		void local_translation(const vec3& x);
		void local_scale(const vec3& x);
		void local_rotation(const quat& x);

		const vec3& local_translation() const;
		const vec3& local_scale() const;
		const quat& local_rotation() const;

		mat4 global_transform() const;
		vec3 global_translation() const;
		vec3 global_scale() const;
		quat global_rotation() const;

		void detach_from_parent() { _isDetachedFromParent = true; }
		void reattach_to_parent() { _isDetachedFromParent = false; }
		u8 is_detached_from_parent() { return _isDetachedFromParent; }

	protected:

		virtual void on_transformed() {}

	private:

		mat4 _transform{ mat4(1) };
		vec3 _scale{ 1, 1, 1 };
		vec3 _translation{ 0, 0, 0 };
		quat _rotation{ quat(vec3(0, 0, 0)) };

		u8 _isDetachedFromParent{ false };

		void update_transform();
		void update_tsr();
	};
}
