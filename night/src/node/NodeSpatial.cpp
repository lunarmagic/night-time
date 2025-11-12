
#include "nightpch.h"
#include "NodeSpatial.h"

namespace night
{

	mat4 const& NodeSpatial::local_transform() const
	{
		return _transform;
	}

	void NodeSpatial::local_transform(mat4 const& x)
	{
		_transform = x;
		update_tsr();;
	}

	void NodeSpatial::local_translate(const vec3& translation)
	{
		_translation += translation;
		update_transform();
	}

	void NodeSpatial::local_rotate(const vec3& axis, real radians)
	{
		_rotation *= math::angle_axis(radians, axis);
		update_transform();
	}

	void NodeSpatial::local_translation(const vec3& x)
	{
		_translation = x;
		update_transform();
	}

	void NodeSpatial::local_scale(const vec3& x)
	{
		_scale = x;
		update_transform();
	}

	void NodeSpatial::local_rotation(const quat& x)
	{
		_rotation = x;
		update_transform();
	}

	const vec3& NodeSpatial::local_translation() const
	{
		return _translation;
	}

	const vec3& NodeSpatial::local_scale() const
	{
		return _scale;
	}

	const quat& NodeSpatial::local_rotation() const
	{
		return _rotation;
	}

	void NodeSpatial::update_transform()
	{
		_transform = mat4(1);
		_transform = math::scale(_scale) * _transform;
		_transform = math::quat_to_mat4(_rotation) * _transform;
		_transform = math::translate(_translation) * _transform;
		
		on_transformed();

		dispatch_system([](NodeSpatial& node)
			{
				node.on_transformed();
			}, exclude<>, true);
	}

	void NodeSpatial::update_tsr()
	{
		//vec3 skew;
		//vec4 perspective;
		DecomposedTransform<> dt = math::decompose(_transform);
		_translation = dt.translation;
		_scale = dt.scale;
		_rotation = dt.rotation;
		//math::decompose(_transform, _scale, _rotation, _translation, skew, perspective);

		on_transformed();

		dispatch_system([](NodeSpatial& node)
			{
				node.on_transformed();
			}, exclude<>, true);
	}

	// TODO: cache global transform of tree every time local transform is updated or node is moved
	mat4 NodeSpatial::global_transform() const
	{
		if (_isDetachedFromParent)
		{
			return _transform;
		}

		auto const& parent = this->parent();

		if(parent == nullptr || !parent->is_of_type<NodeSpatial>())
		{
			return _transform;
		}

		return handle<NodeSpatial>(parent)->global_transform() * _transform;
	}

	vec3 NodeSpatial::global_translation() const
	{
		mat4 gt = global_transform(); // TODO: cache everything on EventNodeGlobalTransformUpdated

		return math::decompose(gt).translation;

		//vec3 scale;
		//quat rotation;
		//vec3 translation;
		//math::vec3 skew;
		//math::vec4 perspective;
		//math::decompose(gt, scale, rotation, translation, skew, perspective);

		//return translation;
	}

	vec3 NodeSpatial::global_scale() const
	{
		mat4 gt = global_transform();

		return math::decompose(gt).scale;

		//vec3 scale;
		//quat rotation;
		//vec3 translation;
		//math::vec3 skew;
		//math::vec4 perspective;
		//math::decompose(gt, scale, rotation, translation, skew, perspective);

		//return scale;
	}

	quat NodeSpatial::global_rotation() const
	{
		mat4 gt = global_transform();

		return math::decompose(gt).rotation;

		//vec3 scale;
		//quat rotation;
		//vec3 translation;
		//math::vec3 skew;
		//math::vec4 perspective;
		//math::decompose(gt, scale, rotation, translation, skew, perspective);

		//return rotation;
	}

}