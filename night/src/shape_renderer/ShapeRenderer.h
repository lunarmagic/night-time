#pragma once

#include "math/math.h"
#include "color/Color.h"
#include "renderer/IRenderer.h"

#define NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY 0.1f
#define NIGHT_SHAPE_RENDERER_LINE_WIDTH RENDERER_LINE_DEFAULT_WIDTH
#define NIGHT_BOX_COLOR BROWN
#define NIGHT_SPHERE_COLOR LIGHT_BLUE
#define NIGHT_CYLINDER_COLOR GREY
#define NIGHT_CONE_COLOR NIGHT_CYLINDER_COLOR
#define NIGHT_CONVEX_COLOR ORANGE

namespace night
{
	struct RenderGraph;
	struct Plane;

	// TODO: ellipse, arrow, plane

	struct DrawBoxParams
	{
		mat4 transform{mat4(1)};
		RenderGraph* out_graph{ nullptr };
		u8 wireframe{ true };
		real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
		real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
		Color color{ NIGHT_BOX_COLOR };
	};

	struct DrawSphereParams
	{
		vec3 origin;
		real radius{ 1.0f };
		RenderGraph* out_graph{ nullptr };
		u8 wireframe{ true };
		u8 outline_only{ true };
		real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
		real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
		Color color{ NIGHT_SPHERE_COLOR };
		real resolution{ 1.0f };
	};

	struct SphereBackfacePlane
	{
		vec3 origin;
		vec3 normal;
		real radius;
	};

	struct CylinderBackfacePlane
	{
		vec3 edge_1_a;
		vec3 edge_1_b;
		vec3 edge_2_a;
		vec3 edge_2_b;
		vec3 normal;
	};

	using ConeBackfacePlane = CylinderBackfacePlane;

	struct DrawCylinderParams
	{
		//mat4 transform{ mat4(1) };
		vec3 origin;
		vec3 direction;
		real radius{ 1.0f };
		real height{ 1.0f };
		RenderGraph* out_graph{ nullptr };
		u8 wireframe{ true };
		real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
		real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
		Color color{ NIGHT_CYLINDER_COLOR };
		real resolution{ 1.0f };
	};

	using DrawConeParams = DrawCylinderParams;

	struct DrawConvexParams
	{
		vector<Plane> const* planes;
		mat4 transform{ mat4(1) };
		RenderGraph* out_graph{ nullptr };
		u8 wireframe{ true };
		real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
		real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
		Color color{ NIGHT_CONVEX_COLOR };
	};
	
	struct NIGHT_API ShapeRenderer
	{
		static void draw_box(DrawBoxParams const& params);

		static SphereBackfacePlane sphere_backface_plane(vec3 const& origin, real const& radius, vec3 const& point);
		static SphereBackfacePlane sphere_backface_plane(vec3 const& origin, real const& radius, Camera const& camera);
		static void draw_sphere(DrawSphereParams const& params);

		static CylinderBackfacePlane cylinder_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera);
		static void draw_cylinder(DrawCylinderParams const& params);

		static ConeBackfacePlane cone_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera);
		static void draw_cone(DrawConeParams const& params);

		static void draw_convex(DrawConvexParams const& params);
	};

}