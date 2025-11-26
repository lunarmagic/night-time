#pragma once

#include "math/Math.h"
#include "color/Color.h"
#include "renderer/IRenderer.h"
#include "geometry/Plane.h"
#include "geometry/Triangle.h"

#define NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY 0.1f
#define NIGHT_SHAPE_RENDERER_LINE_WIDTH RENDERER_LINE_DEFAULT_WIDTH
#define NIGHT_BOX_COLOR BROWN
#define NIGHT_SPHERE_COLOR LIGHT_BLUE
#define NIGHT_CYLINDER_COLOR GREY
#define NIGHT_CONE_COLOR NIGHT_CYLINDER_COLOR
#define NIGHT_CONVEX_COLOR ORANGE

#define NIGHT_SHAPE_RENDERER_DEFAULT_LINE_COLOR BLACK
#define NIGHT_SHAPE_RENDERER_DEFAULT_LINE_WIDTH RENDERER_LINE_DEFAULT_WIDTH
#define NIGHT_SHAPE_RENDERER_DEFAULT_NEAR_CORNER_OPACITY 1
#define NIGHT_SHAPE_RENDERER_DEFAULT_FAR_CORNER_OPACITY 0.1f

namespace night
{
	struct RenderGraph;

	// TODO: ellipse, arrow, plane

	struct DrawLineCallbackParams
	{
		DrawLineParams const& params;
		// store information about the edge, is it on the outline or near corner
	};

	struct DrawTriangleCallbackParams
	{
		Triangle<> const& triangle;
		vec3 const& normal;
	};

	struct DrawBoxParams
	{
		mat4 transform = mat4(1);
		vec3 extents = vec3(1.0f);
		Color color = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_COLOR;
		real near_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_NEAR_CORNER_OPACITY;
		real far_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_FAR_CORNER_OPACITY;
		real width = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_WIDTH;
		function<void(DrawLineCallbackParams const&)> on_draw_line;
		function<void(DrawTriangleCallbackParams const&)> on_draw_triangle;
	};

	struct DrawPyramidParams
	{
		mat4 transform = mat4(1);
		vec3 extents = vec3(1.0f);
		Color color = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_COLOR;
		real near_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_NEAR_CORNER_OPACITY;
		real far_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_FAR_CORNER_OPACITY;
		real width = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_WIDTH;
		function<void(DrawLineCallbackParams const&)> on_draw_line;
		function<void(DrawTriangleCallbackParams const&)> on_draw_triangle;
	};

	//struct DrawSphereParams
	//{
	//	vec3 origin;
	//	real radius{ 1.0f };
	//	RenderGraph* out_graph{ nullptr };
	//	b8 wireframe{ true };
	//	b8 outline_only{ true };
	//	real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
	//	real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
	//	Color color{ NIGHT_SPHERE_COLOR };
	//	real resolution{ 1.0f };
	//};

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

	//struct DrawCylinderParams
	//{
	//	//mat4 transform{ mat4(1) };
	//	vec3 origin;
	//	vec3 direction;
	//	real radius{ 1.0f };
	//	real height{ 1.0f };
	//	RenderGraph* out_graph{ nullptr };
	//	b8 wireframe{ true };
	//	real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
	//	real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
	//	Color color{ NIGHT_CYLINDER_COLOR };
	//	real resolution{ 1.0f };
	//};

	//using DrawConeParams = DrawCylinderParams;

	//struct DrawConvexParams
	//{
	//	vector<Plane<>> const* planes;
	//	mat4 transform{ mat4(1) };
	//	RenderGraph* out_graph{ nullptr };
	//	b8 wireframe{ true };
	//	real width{ NIGHT_SHAPE_RENDERER_LINE_WIDTH };
	//	real draw_through_opacity{ NIGHT_SHAPE_RENDERER_DRAW_THROUGH_OPACITY };
	//	Color color{ NIGHT_CONVEX_COLOR };
	//};

	struct DrawCylinderParams2
	{
		mat4 transform = mat4(1);
		real radius = 1.0f;
		real height = 1.0f;
		Color color = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_COLOR;
		real near_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_NEAR_CORNER_OPACITY;
		real far_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_FAR_CORNER_OPACITY;
		real width = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_WIDTH;
		real resolution{ 1.0f };
		function<void(DrawLineCallbackParams const&)> on_draw_line;
		function<void(DrawTriangleCallbackParams const&)> on_draw_triangle;
	};

	using DrawConeParams2 = DrawCylinderParams2;

	struct DrawSphereParams2
	{
		mat4 transform = mat4(1);
		real radius = 1.0f;
		Color color = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_COLOR;
		//real near_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_NEAR_CORNER_OPACITY;
		//real far_corner_opacity = NIGHT_SHAPE_RENDERER_DEFAULT_FAR_CORNER_OPACITY;
		real width = NIGHT_SHAPE_RENDERER_DEFAULT_LINE_WIDTH;
		real resolution{ 1.0f };
		function<void(DrawLineCallbackParams const&)> on_draw_line;
		function<void(DrawTriangleCallbackParams const&)> on_draw_triangle;
	};
	
	// TODO: support texture coordinates
	struct NIGHT_API ShapeRenderer3D
	{
		// TODO: add these functions directly in RenderGraph and IRenderer
		static void draw_box(RenderTarget render_target, DrawBoxParams const& params);
		static void draw_pyramid(RenderTarget render_target, DrawPyramidParams const& params);
		static void draw_cylinder2(RenderTarget render_target, DrawCylinderParams2 const& params);
		static void draw_cone2(RenderTarget render_target, DrawConeParams2 const& params);
		static void draw_sphere2(RenderTarget render_target, DrawSphereParams2 const& params);

		static SphereBackfacePlane sphere_backface_plane(vec3 const& origin, real const& radius, vec3 const& point);
		static SphereBackfacePlane sphere_backface_plane(vec3 const& origin, real const& radius, Camera const& camera);
		static CylinderBackfacePlane cylinder_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera);
		static ConeBackfacePlane cone_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera);
		
		//static void draw_sphere(DrawSphereParams const& params);
		//
		//
		//static void draw_cylinder(DrawCylinderParams const& params);
		//
		//
		//static void draw_cone(DrawConeParams const& params);
		//
		//static void draw_convex(DrawConvexParams const& params);
	};

}