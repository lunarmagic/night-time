#pragma once

#include "math/math.h"
#include "WorldSpaceRasterizer2D.h"
#include "node/INode.h"
#include "debug_renderer/DebugRenderer.h"

namespace night
{
	struct Quad;
	struct IComputeShader;

	struct NodeDistanceFields2DParams
	{
		s32 field_width{ -1 };
		s32 field_height{ -1 };
		s32 field_count{ -1 };
		u8 should_use_gpu{ true };
	};

	/*
	*	A node to handle generating multiple distance fields on the gpu in parallel.
	*/
	struct NIGHT_API NodeDistanceFields2D : public INode
	{
		~NodeDistanceFields2D();

		void init(NodeDistanceFields2DParams const& params);
		void clear();

		void area(Quad const& area, s32 field_index);

		void draw_point(ivec2 internal_point, s32 field_index);
		void draw_point(vec2 point, s32 field_index);

		void draw_line(ivec2 internal_p1, ivec2 internal_p2, s32 field_index);
		void draw_line(vec2 p1, vec2 p2, s32 field_index);

		void dispatch_in_main_thread(function<void(IComputeShader&)> main_thread_callback);

		vec2 compare_fields(s32 field_a_index, s32 field_b_index);

		vector<real>& fields() { return _fields; }
		vector<real> const& fields() const { return _fields; }
		real* field(s32 field_index) { ASSERT(field_index * field_size() < _fields.size()); return &_fields[field_index * field_size()]; }
		const real* field(s32 field_index) const { ASSERT(field_index * field_size() < _fields.size()); return &_fields[field_index * field_size()]; }
		s32 field_size() const { return _fieldWidth * _fieldHeight; }
		s32 const& field_count() const { return _fieldCount; }
		s32 const& field_width() const { return _fieldWidth; }
		s32 const& field_height() const { return _fieldHeight; }
		void should_use_gpu(u8 x) { _shouldUseGPU = x; }
		u8 const& should_use_gpu() const { return _shouldUseGPU; }
		WorldSpaceRasterizer2D<real>& rasterizer(s32 field_index) { ASSERT(field_index < _rasterizers.size()); return _rasterizers[field_index]; };
		WorldSpaceRasterizer2D<real> const& rasterizer(s32 field_index) const { ASSERT(field_index < _rasterizers.size()); return _rasterizers[field_index]; };
		vector<ivec2> const& distance_zeros(s32 field_index) const { ASSERT(field_index < _tempDistZeros.size()); return _tempDistZeros[field_index]; }
		vector<ivec2>& distance_zeros(s32 field_index) { ASSERT(field_index < _tempDistZeros.size()); return _tempDistZeros[field_index]; }

	private:

		vector<real> _fields;
		vector<ivec2> _distZeros;
		vector<vector<ivec2>> _tempDistZeros; // TODO: put in same block of memory as rasterizers

		struct Instruction
		{
			s32 index{-1};
			s32 count{-1};
			real width{ 1.0f };
			real height{ 1.0f };
		};
		vector<Instruction> _instructions;

		// rasterizers to fill in pixels using world space:
		vector<WorldSpaceRasterizer2D<real>> _rasterizers;

		s32 _fieldCount{ 0 };
		s32 _fieldWidth{ 0 };
		s32 _fieldHeight{ 0 };
		u8 _shouldUseGPU{ true };

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		friend struct DebugRenderer;
		s32 __debugTextureSlider{ 0 };
#ifdef NIGHT_DEBUG
		vector<handle<ITexture>> __debugTextures;
#endif
#endif
	};

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
	template<> inline void DebugRenderer::draw_format<NodeDistanceFields2D>(NodeDistanceFields2D& v)
	{
		IGui& gui = DebugRenderer::gui();
		gui.drag_s32("Current Time", &v.__debugTextureSlider, 0.5f, 0, v._fieldCount - 1);

		ASSERT(v.__debugTextureSlider < v._rasterizers.size());
		Quad area = v._rasterizers[v.__debugTextureSlider].area();
		// TODO: draw all areas, skip duplicates
		DB_RENDERER_DRAW_OBJECT(area);

#ifdef NIGHT_DEBUG
		if (v.__debugTextures.empty())
		{
			return;
		}

		ASSERT(v.__debugTextureSlider < v.__debugTextures.size());

		area.vertices[0].point.z -= 0.001f;
		area.vertices[1].point.z -= 0.001f;
		area.vertices[2].point.z -= 0.001f;
		area.vertices[3].point.z -= 0.001f;

		auto& tx = v.__debugTextures[v.__debugTextureSlider];
		if (tx != nullptr)
		{
			auto& graph = DebugRenderer::render_graph();
			graph.current_buffer(graph.current_render_target(), nullptr, tx);
			graph.draw_quad(area);
		}
#endif
	}
#endif

}