#pragma once

#include "math/Math.h"
#include "WorldSpaceRasterizer2D.h"
#include "node/INode.h"
#include "debug_renderer/DebugRenderer.h"
#include "geometry/Quad.h"

namespace night
{
	struct IComputeShader;

	struct NodeDistanceFields2DParams
	{
		s32 field_width{ -1 };
		s32 field_height{ -1 };
		s32 field_count{ -1 };
		Quad<> area = Quad<>{};
	};

	//struct NDF2DDrawLineParams
	//{
	//	vec2 p1 = {};
	//	vec2 p2 = {};
	//	s32 field_index = -1;
	//};

	struct NIGHT_API DistanceField
	{
		//r32* field;
		//s32 width;
		//s32 height;
		vector<ivec2> distance_zeros;
		WorldSpaceRasterizer2D<r32> rasterizer;

		void draw_point(ivec2 internal_point);
		void draw_point(vec2 point);

		void draw_line(ivec2 internal_p1, ivec2 internal_p2);
		void draw_line(vec2 p1, vec2 p2);

		// dot 2 distance fields together as if they were high-dimensional
		// vectors, both distance fields must be of equal dimensions
		static real dot(DistanceField const& a, DistanceField const& b);
	};

	/*
	*	A node to handle generating multiple distance fields on the gpu in parallel.
	*/
	struct NIGHT_API NodeDistanceFields2D : public INode
	{
		~NodeDistanceFields2D();

		void init(NodeDistanceFields2DParams const& params);
		void clear();

		//void area(Quad<> const& area, s32 field_index);
		//Quad<> const& area(s32 field_index) const;

		void compute_in_this_thread(b8 use_gpu = true, s32 begin = -1, s32 end = -1);

		// TODO: dot

		vector<r32>& fragments() { return _fragments; }
		vector<r32> const& fragments() const { return _fragments; }
		//r32* field(s32 field_index) { ASSERT(field_index * field_size() < _fields.size()); return &_fields[field_index * field_size()]; }
		//const r32* field(s32 field_index) const { ASSERT(field_index * field_size() < _fields.size()); return &_fields[field_index * field_size()]; }
		
		DistanceField& field(s32 index) { ASSERT(index < _FIELDS.size()); return _FIELDS[index]; };

		s32 const& field_count() const { return _fieldCount; }
		s32 const& field_width() const { return _fieldWidth; }
		s32 const& field_height() const { return _fieldHeight; }
		s32 field_size() const { return _fieldWidth * _fieldHeight; }
		//s32 field_index(s32 x, s32 y) const;

		void should_use_gpu(b8 x) { _shouldUseGPU = x; }
		b8 const& should_use_gpu() const { return _shouldUseGPU; }

		//WorldSpaceRasterizer2D<r32>& rasterizer(s32 field_index) { ASSERT(field_index < _rasterizers.size()); return _rasterizers[field_index]; };
		//WorldSpaceRasterizer2D<r32> const& rasterizer(s32 field_index) const { ASSERT(field_index < _rasterizers.size()); return _rasterizers[field_index]; };
		//vector<ivec2> const& distance_zeros(s32 field_index) const { ASSERT(field_index < _tempDistZeros.size()); return _tempDistZeros[field_index]; }
		//vector<ivec2>& distance_zeros(s32 field_index) { ASSERT(field_index < _tempDistZeros.size()); return _tempDistZeros[field_index]; }

	private:

		vector<DistanceField> _FIELDS;
		//vector<vector<ivec2>> _tempDistZeros; // TODO: put in same block of memory as rasterizers

		struct Instruction
		{
			s32 index{-1};
			s32 count{-1};
			r32 width{ 1.0f };
			r32 height{ 1.0f };
		};

		vector<r32> _fragments;
		vector<ivec2> _ssboDistanceZeros;
		vector<Instruction> _ssboInstructions;

		// rasterizers to fill in pixels using world space:
		//vector<WorldSpaceRasterizer2D<r32>> _rasterizers;

		s32 _fieldCount{ 0 };
		s32 _fieldWidth{ 0 };
		s32 _fieldHeight{ 0 };
		b8 _shouldUseGPU{ true };

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		friend struct DebugRenderer;
		s32 __debugTextureSlider{ 0 };
		vector<handle<ITexture>> __debugTextures;
#endif
	};

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
	template<> inline void DebugRenderer::draw_format<NodeDistanceFields2D>(NodeDistanceFields2D& v)
	{
		if (!v._FIELDS.empty())
		{
			IGui& gui = DebugRenderer::gui();
			gui.drag_s32("Current Time", &v.__debugTextureSlider, 0.5f, 0, v._fieldCount - 1);
		
			ASSERT(v.__debugTextureSlider < v._FIELDS.size());
			Quad area = v._FIELDS[v.__debugTextureSlider].rasterizer.area();
			// TODO: draw all areas, skip duplicates
			DB_RENDERER_DRAW_OBJECT(area);
		
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
		}
	}
#endif

}