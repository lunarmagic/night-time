#pragma once

#include "handle/handle.h"
#include "renderer/RenderGraph.h"
#include "aabb/AABB.h"
#include "utility.h"
//#include <utility>

#define DB_RENDERER_ARROW_COLOR ORANGE
#define DB_RENDERER_SPHERE_COLOR LIGHT_BLUE.darken(0.75f)
#define DB_RENDERER_ELLIPSE_COLOR CYAN
#define DB_RENDERER_CIRCLE_COLOR LIGHT_BLUE
#define DB_RENDERER_BOX_COLOR BROWN
#define DB_RENDERER_HIT_COLOR RED

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
#define DB_RENDERER_INIT() ::night::DebugRenderer::init()

#define DB_RENDERER_DRAW_OBJECT(obj) ::night::DebugRenderer::draw_object(obj, false)
#define DB_RENDERER_DRAW_POINT(point, color) ::night::DebugRenderer::draw_point(point, color)
#define DB_RENDERER_DRAW_LINE(p1, p2, color) ::night::DebugRenderer::draw_line(p1, p2, color)
#define DB_RENDERER_DRAW_QUAD(quad) ::night::DebugRenderer::draw_quad(quad)
#define DB_RENDERER_DRAW_TEXT(text) ::night::DebugRenderer::draw_text(text)
#define DB_RENDERER_DRAW_HOVER_TEXT(params) //::night::DebugRenderer::draw_hover_text(params)
//#define DB_RENDERER_DRAW_ARROW(params) //::night::DebugRenderer::draw_arrow(params)
//#define DB_RENDERER_DRAW_PLANE(params) //::night::DebugRenderer::draw_plane(params)
//#define DB_RENDERER_DRAW_ELLIPSE(params) //::night::DebugRenderer::draw_ellipse(params)

#define DB_RENDERER_FLUSH() ::night::DebugRenderer::render()

//#define DB_RENDERER_SET_CAMERA(camera_) ::night::DebugRenderer::camera(camera_) // TODO: handle camera automatically
#else
#define DB_RENDERER_INIT()

//#define DB_ALGO_INVOLVE_NODES(...)
//#define DB_INVOLVE_NODES_SCOPED(...)
//#define DB_ALGO_UNINVOLVE_NODES()
//#define DB_ALGO_PUSH(name)
//#define DB_ALGO_SCOPED(name)
//#define DB_ALGO_INCREMENT_STEP()
//#define DB_ALGO_POP()
//#define DB_ALGO_DRAW_STEP(render_function)
//#define DB_ALGO_STEP_SHOW_ONION_SKINS(count)

#define DB_RENDERER_DRAW_OBJECT(obj)
#define DB_RENDERER_DRAW_POINT(point, color)
#define DB_RENDERER_DRAW_LINE(p1, p2, color)
#define DB_RENDERER_DRAW_QUAD(quad)
#define DB_RENDERER_DRAW_TEXT(text)
#define DB_RENDERER_DRAW_HOVER_TEXT(params)
#define DB_RENDERER_DRAW_ARROW(params)
#define DB_RENDERER_DRAW_PLANE(params)
#define DB_RENDERER_DRAW_ELLIPSE(params)

//#define DB_ALGO_DRAW_FN(fn)
//#define DB_ALGO_DRAW_OBJECT(obj)
//#define DB_ALGO_DRAW_POINT(point, color)
//#define DB_ALGO_DRAW_LINE(p1, p2, color)
//#define DB_ALGO_DRAW_QUAD(quad)
//#define DB_ALGO_DRAW_TEXT(text)
//#define DB_ALGO_DRAW_ARROW(orgn, dir, col)
//#define DB_ALGO_DRAW_CIRCLE(orgn, nrmal, rdius, clor)
//#define DB_ALGO_DRAW_ELLIPSE(orgn, nrmal, mjor_axis, rdii, clor)
//#define DB_ALGO_DRAW_SPHERE(orgn, rdius, clor)
//#define DB_ALGO_DRAW_PLANE(orgn, nrmal, clor)

#define DB_RENDERER_FLUSH()

#define DB_RENDERER_SET_CAMERA(camera)
#define DB_ALGO_LAMBDA_CAPTURE &
#endif

#if defined(NIGHT_ENABLE_DEBUG_RENDERER) && defined(NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER)
#define NIGHT_DBAR // shortcut for ifdef.

#define DB_ALGO_INVOLVE_NODES(...) ::night::DebugRenderer::algo_involve_nodes(##__VA_ARGS__)
#define DB_ALGO_UNINVOLVE_NODES() ::night::DebugRenderer::algo_uninvolve_nodes()

#define DB_ALGO_PUSH(name) { ::night::DebugRenderer::algo_push(name); if(::night::DebugRenderer::algo_should_break()) { ASSERT(false) } }
#define DB_ALGO_INCREMENT_STEP() { ::night::DebugRenderer::algo_increment_step(); if(::night::DebugRenderer::algo_should_break()) { ASSERT(false) } }
#define DB_ALGO_POP() ::night::DebugRenderer::algo_pop()

#define DB_ALGO_INVOLVE_NODES_SCOPED(...) ::night::DBAlgoInvolveNodesScoped UNIQUE_VARIABLE_NAME(involved_nodes_scoped)(##__VA_ARGS__)
#define DB_ALGO_SCOPED(name) ::night::DBAlgoPushScoped UNIQUE_VARIABLE_NAME(algo_scoped)(name)

#define DB_ALGO_DRAW_FN(fn) ::night::DebugRenderer::push_draw_function(fn, true)
#define DB_ALGO_DRAW_OBJECT(obj) ::night::DebugRenderer::draw_object(obj, true)
#define DB_ALGO_DRAW_POINT(point, color) ::night::DebugRenderer::draw_point(point, color, true)
#define DB_ALGO_DRAW_LINE(p1, p2, color) ::night::DebugRenderer::draw_line(p1, p2, color, true)
#define DB_ALGO_DRAW_QUAD(quad) ::night::DebugRenderer::draw_quad(quad, true)
#define DB_ALGO_DRAW_TEXT(text) ::night::DebugRenderer::draw_text(text, true)
#define DB_ALGO_DRAW_ARROW(orgn, dir, col) ::night::DebugRenderer::draw_arrow(DebugRenderer::DrawArrowParams{ .origin = orgn, .direction = dir, .color = col }, true)
#define DB_ALGO_DRAW_CIRCLE(orgn, nrmal, rdius, clor) ::night::DebugRenderer::draw_ellipse({.origin = orgn, .normal = nrmal, .radii = vec2(rdius), .color = clor}, true)
#define DB_ALGO_DRAW_ELLIPSE(orgn, nrmal, mjor_axis, rdii, clor) ::night::DebugRenderer::draw_ellipse({.origin = orgn, .normal = nrmal, .major_axis = mjor_axis, .radii = rdii, .color = clor}, true)
#define DB_ALGO_DRAW_SPHERE(orgn, rdius, clor) ::night::DebugRenderer::draw_sphere({.origin = orgn, .radius = rdius, .color = clor}, true)
#define DB_ALGO_DRAW_PLANE(orgn, nrmal, clor) ::night::DebugRenderer::draw_plane({.origin = orgn, .normal = nrmal, .color = clor}, true)

#define DB_ALGO_LAMBDA_CAPTURE =
#else
#define DB_ALGO_INVOLVE_NODES(...)
#define DB_ALGO_INVOLVE_NODES_SCOPED(...)
#define DB_ALGO_UNINVOLVE_NODES()
#define DB_ALGO_PUSH(name)
#define DB_ALGO_SCOPED(name)
#define DB_ALGO_INCREMENT_STEP()
#define DB_ALGO_POP()
#define DB_ALGO_DRAW_STEP(render_function)
#define DB_ALGO_STEP_SHOW_ONION_SKINS(count)
#define DB_ALGO_RENDER()
#define DB_ALGO_LAMBDA_CAPTURE &

#define DB_ALGO_DRAW_FN(fn)
#define DB_ALGO_DRAW_OBJECT(obj)
#define DB_ALGO_DRAW_POINT(point, color)
#define DB_ALGO_DRAW_LINE(p1, p2, color)
#define DB_ALGO_DRAW_QUAD(quad)
#define DB_ALGO_DRAW_TEXT(text)
#define DB_ALGO_DRAW_ARROW(orgn, dir, col)
#define DB_ALGO_DRAW_CIRCLE(orgn, nrmal, rdius, clor)
#define DB_ALGO_DRAW_ELLIPSE(orgn, nrmal, mjor_axis, rdii, clor)
#define DB_ALGO_DRAW_SPHERE(orgn, rdius, clor)
#define DB_ALGO_DRAW_PLANE(orgn, nrmal, clor)
#endif

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
#define DB_ALGO_MAX_DEPTH 10

namespace night
{

	struct Camera;

	enum class EDebugRendererTab
	{
		Scene = 0,
		Algorithms,
		Profiler,
		Assets,
	};

	struct NIGHT_API DebugRenderer
	{
		static void init();

		template<typename... Nodes>
		static void algo_involve_nodes(handle<Nodes>&&... nodes);
		static void algo_uninvolve_nodes();

		static void algo_push(string const& name);
		static void algo_increment_step();
		static void algo_pop();

		static void render();

		template<typename T>
		static void draw_format(T& t)
		{
			TRACE("draw_format missing, type: {0}", typeid(T).name());
		}

		static void push_draw_function(function<void()> fn, b8 is_algo = false);

		template<typename T> 
		static void draw_object(T& t, b8 is_algo = false)
		{
			push_draw_function([=]() mutable
				{
					draw_format(t);
				}, is_algo);
		}

		template<typename T>
		static void add_object_draw_function();

		struct DrawPointParams
		{
			vec3 point;
			Color color;
		};

		static void draw_point(DrawPointParams const& params, b8 is_algo = false);
		static void draw_point(vec3 const& point, Color const& color = LIGHT_BLUE, b8 is_algo = false)
		{
			draw_point(DrawPointParams{ .point = point, .color = color }, is_algo);
		}
		static void draw_point(vec2 const& point, Color const& color = LIGHT_BLUE, b8 is_algo = false)
		{
			draw_point(DrawPointParams{ .point = vec3(point, 0), .color = color }, is_algo);
		}

		static void draw_line(DrawLineParams const& params, b8 is_algo = false);
		static void draw_line(vec3 const& p1, vec3 const& p2, Color const& color = RED, b8 is_algo = false)
		{
			draw_line(DrawLineParams{ .p1 = p1, .p2 = p2, .color = color }, is_algo);
		}
		static void draw_line(vec2 const& p1, vec2 const& p2, Color const& color = RED, b8 is_algo = false)
		{
			draw_line(DrawLineParams{ .p1 = vec3(p1, 0), .p2 = vec3(p2, 0), .color = color }, is_algo);
		}

		static void draw_quad(Quad<> const& quad, b8 is_algo = false);

		static void draw_text(Text const& text, b8 is_algo = false);

		struct DrawHoverTextParams
		{
			vec3 origin{ ORIGIN };
			string text{ "" };
			real hover_radius{ 0.033f };
			Color color{ WHITE };
		};

		static void draw_hover_text(DrawHoverTextParams const& params, b8 is_algo = false);

		struct DrawArrowParams
		{
			vec3 origin{ ORIGIN };
			vec3 direction{ ORIGIN };
			real length{ 0.15f };
			Color color{ RED };
		};

		static void draw_arrow(DrawArrowParams const& params, b8 is_algo = false);

		struct DrawPlaneParams
		{
			vec3 origin{ vec3(0) };
			vec3 normal{ vec3(0) };
			real grid_size{ 5.0f };
			s32 grid_resolution{ 15 };
			Color color{ ORANGE };
			//RenderGraph& out_graph; // TODO: make render graph a static member so we don't need to call the draw functions in the draw callback
		};

		static void draw_plane(DrawPlaneParams const& params, b8 is_algo = false);

		struct DrawEllipseParams
		{
			vec3 origin{ ORIGIN };
			vec3 normal{ FORWARD };
			vec3 major_axis{ UP };
			vec2 radii{ vec2(ORIGIN) };
			real min_theta{ 0.0f };
			real max_theta{ R_PI * 2 };
			Color color{ BLUE };
			s32 segments{ 32 };
		};

		static void draw_ellipse(DrawEllipseParams const& params, b8 is_algo = false);

		struct DrawSphereParams
		{
			vec3 origin{ ORIGIN };
			real radius{ 0.0f };
			Color color{ BLUE };
			s32 segments{ 32 };
		};

		static void draw_sphere(DrawSphereParams const& params, b8 is_algo = false);

		struct DrawCylinderParams
		{
			vec3 origin{ ORIGIN };
			vec3 direction{ ORIGIN };
			real radius{ 0.0f };
			real height{ 0.0f };
			Color color{ BLUE };
			s32 segments{ 32 };
		};

		static void draw_cylinder(DrawCylinderParams const& params, b8 is_algo = false);

		static RenderGraph& render_graph() { return _renderGraph; }
		static IGui& gui(); // may want to use a dummy gui and apply elements delayed.

		static void camera(Camera const& camera); // TODO: update camera automatically, flush render graph for each camera, camera is set according to currently processed nodes

		static b8 algo_should_break();

		static b8 should_save_debug_textures() { return _assetsShouldSaveDebugTextures; }

		static b8 wants_input() { return _wantsInput; }

	protected:
		DebugRenderer() = default;
		DebugRenderer(DebugRenderer const& other) = default;

	private:

		inline static b8 _wantsInput = false;
		//inline static shandle<INode> _nodlessDummyNode = nullptr;

		static EDebugRendererTab _guiCurrentTab;
		static vector<handle<INode>> _guiSelectedInvolvedNodes;
		static vector<pair<string, s32>> _guiSelectedAlgorithmSteps;

		struct Break
		{
			vector<handle<INode>> involved_nodes;
			vector<pair<string, s32>> algorithm_stack;
		};

		static Break _guiBreak;
		static string _guiSelectedAlgorithm;

		struct Algorithm;
		//using AlgorithmTree = map<string, Algorithm>;

		struct RenderStep
		{
			vector<function<void()>> callbacks;
			map<string, Algorithm> sub_steps;
		};

		struct Algorithm
		{
			vector<RenderStep> render_steps;
		};

		//static b8 _algoIsAutoUpdating;
		inline static b8 _algoIsActive = true;
		static map<vector<handle<INode>>, map<string, Algorithm>> _algoInvolvedNodes;
		//static vector<handle<INode>> _algoCurrentlyInvolvedNodes;

		//static stack<vector<handle<INode>>> _algoCurrentlyInvolvedNodesStack;
		//static vector<string> _algoAlgorithmStack;

		struct CIN
		{
			vector<handle<INode>> involved_nodes;
			vector<string> algorithm_stack;
		};

		static vector<CIN> _algoCurrentlyInvolvedNodesStack;
		//static vector<handle<ITexture>> _algoRenderTargets;
		static CameraTransform _algoCameraTransform;

		//static handle<ITexture> _sceneRenderTarget;
		static umap<handle<INode>, b8> _sceneSelectedNodes;
		static umap<type_index, function<void(void*)>> _sceneObjectDrawFunctionTable;

		static handle<ITexture> _renderTarget;
		static RenderGraph _renderGraph;

		static string _assetsSelectedTexture;
		static b8 _assetsTextureShowDepthBuffer;

		static b8 _assetsShouldSaveDebugTextures;

		static void scene_render();
		static void algo_render();
		static void profiler_render();
		static void assets_render();
	};

	template<typename ...Nodes>
	inline void DebugRenderer::algo_involve_nodes(handle<Nodes>&& ...nodes)
	{
		CIN cins;

		//vector<handle<INode>> cins;

		for (const auto i : { nodes... })
		{
			cins.involved_nodes.emplace_back(i);
		}

		_algoCurrentlyInvolvedNodesStack.push_back(cins);
#if 0
		if (!_algoIsAutoUpdating || _guiCurrentTab != EDebugRendererTab::Algorithms)
		{
			return;
		}

		ASSERT(_algoCurrentlyInvolvedNodes.empty()); // always call UNINVOLVE_NODES
		
		for (const auto i : { nodes... })
		{
			_algoCurrentlyInvolvedNodes.emplace_back(i);
		}
#endif
	}

	template<typename T>
	inline void DebugRenderer::add_object_draw_function()
	{
		_sceneObjectDrawFunctionTable[typeid(T)] = [](void* object)
			{
				T& t = *(T*)object;
				DebugRenderer::draw_format(t);
			};
	}

	// TODO: handle daabbs
	template<> inline void DebugRenderer::draw_format<AABB<real>>(AABB<real>& v)
	{
		auto& graph = DebugRenderer::render_graph();
		graph.draw_line(vec2(v.left, v.top), vec2(v.right, v.top), BLUE);
		graph.draw_line(vec2(v.right, v.top), vec2(v.right, v.bottom), BLUE);
		graph.draw_line(vec2(v.right, v.bottom), vec2(v.left, v.bottom), BLUE);
		graph.draw_line(vec2(v.left, v.bottom), vec2(v.left, v.top), BLUE);
	}

	// TODO: handle dquads
	template<> inline void DebugRenderer::draw_format<Quad<>>(Quad<>& v)
	{
		auto& graph = DebugRenderer::render_graph();
		graph.draw_line(v.vertices[0].point, v.vertices[1].point, BLUE);
		graph.draw_line(v.vertices[1].point, v.vertices[2].point, BLUE);
		graph.draw_line(v.vertices[2].point, v.vertices[3].point, BLUE);
		graph.draw_line(v.vertices[3].point, v.vertices[0].point, BLUE);
	}

	struct DBAlgoPushScoped
	{
		DBAlgoPushScoped(string const& name)
		{
			//DB_ALGO_PUSH(name);
			DebugRenderer::algo_push(name);
		}

		~DBAlgoPushScoped()
		{
			//DB_ALGO_POP();
			DebugRenderer::algo_pop();
		}
	};

	struct DBAlgoInvolveNodesScoped
	{
		template<typename... Args>
		DBAlgoInvolveNodesScoped(Args&&... nodes)
		{
			//DB_ALGO_INVOLVE_NODES(nodes...);
			DebugRenderer::algo_involve_nodes(std::forward<Args>(nodes)...);
		}

		~DBAlgoInvolveNodesScoped()
		{
			//DB_ALGO_UNINVOLVE_NODES();
			DebugRenderer::algo_uninvolve_nodes();
		}
	};

	//template<typename... Args>
	//DBAlgoInvolveNodesScoped __db_construct_DBAINS(Args... nodes)
	//{
	//	DB_ALGO_INVOLVE_NODES(nodes...);
	//	return DBAlgoInvolveNodesScoped();
	//}

}
#endif