
#include "nightpch.h"
#include "NodeDistanceFields2D.h"
#include "WorldSpaceRasterizer2D.h"
#include "material/IComputeShader.h"
#include "log/log.h"
#include "aabb/AABB.h"
#include "geometry/Quad.h"
#include "utility.h"
#include "profiler/Profiler.h"
#include "texture/ITexture.h"
#include "texture/Surface.h"

namespace night
{

	NodeDistanceFields2D::~NodeDistanceFields2D()
	{
		clear();
	}

	void NodeDistanceFields2D::init(NodeDistanceFields2DParams const& params)
	{
		_fieldCount = params.field_count;
		_fieldWidth = params.field_width;
		_fieldHeight = params.field_height;
		_shouldUseGPU = params.should_use_gpu;

		_fields = vector<real>(_fieldWidth * _fieldHeight * _fieldCount, INFINITY);
		_instructions = vector<Instruction>(_fieldCount, Instruction{});
		_rasterizers = vector<WorldSpaceRasterizer2D<real>>(_fieldCount, WorldSpaceRasterizer2D<real>{});
		_tempDistZeros = vector<vector<ivec2>>(_fieldCount);

		for (s32 i = 0; i < _rasterizers.size(); i++)
		{
			auto& rasterizer = _rasterizers[i];
			rasterizer = WorldSpaceRasterizer2D<real>(&_fields[i * field_size()], _fieldWidth, _fieldHeight, QuadParams());

			ASSERT(i < _instructions.size());
			auto& instruction = _instructions[i];

			Quad const& area = rasterizer.area();
			AABB size; // TODO: get actual width and height of area.
			size.fit_around_point(area.vertices[0].point);
			size.fit_around_point(area.vertices[1].point);
			size.fit_around_point(area.vertices[2].point);
			size.fit_around_point(area.vertices[3].point);

			instruction.width = abs(size.width()) / _fieldWidth;
			instruction.height = abs(size.height()) / _fieldHeight;
		}
	}

	void NodeDistanceFields2D::clear()
	{
		_fields.clear();
		_distZeros.clear();
		_tempDistZeros.clear();
		_instructions.clear();
		_rasterizers.clear();

		_fieldCount = 0;
		_fieldWidth = 0;
		_fieldHeight = 0;
	}

	void NodeDistanceFields2D::draw_point(ivec2 internal_point, s32 field_index)
	{
		ASSERT(field_index < _fieldCount);
		ASSERT(field_index < _rasterizers.size());
		ASSERT(field_index < _instructions.size());
		ASSERT(true); // TODO: implement
	}

	void NodeDistanceFields2D::draw_point(vec2 point, s32 field_index)
	{
		ASSERT(field_index < _fieldCount);
		ASSERT(field_index < _rasterizers.size());
		ASSERT(field_index < _instructions.size());
		ASSERT(true); // TODO: implement
	}

	void NodeDistanceFields2D::draw_line(ivec2 internal_p1, ivec2 internal_p2, s32 field_index)
	{
		ASSERT(field_index < _rasterizers.size());

		ASSERT(field_index < _tempDistZeros.size());
		auto& temp_dist_zeros = _tempDistZeros[field_index];

		auto& rasterizer = _rasterizers[field_index];
		rasterizer.draw_line(internal_p1, internal_p2, [&](auto const& fragment)
		{
			if (fragment.fragment != nullptr && *fragment.fragment != 0.0f)
			{
				*fragment.fragment = 0.0f;
				temp_dist_zeros.push_back(fragment.coordinate);
			}
		}, true);
	}

	void NodeDistanceFields2D::draw_line(vec2 p1, vec2 p2, s32 field_index)
	{
		ASSERT(field_index < _rasterizers.size());

		ASSERT(field_index < _tempDistZeros.size());
		auto& temp_dist_zeros = _tempDistZeros[field_index];

		auto& rasterizer = _rasterizers[field_index];
		rasterizer.draw_line(p1, p2, [&](auto const& fragment)
		{
			if (fragment.fragment != nullptr && *fragment.fragment != 0.0f)
			{
				*fragment.fragment = 0.0f;
				temp_dist_zeros.push_back(fragment.coordinate);
			}
		}, true);
	}

	void NodeDistanceFields2D::area(Quad const& area, s32 field_index)
	{
		ASSERT(field_index < _rasterizers.size());
		auto& rasterizer = _rasterizers[field_index];
		rasterizer.area(area);

		ASSERT(field_index < _instructions.size());
		auto& instruction = _instructions[field_index];

		AABB size; // TODO: get actual width and height of area.
		size.fit_around_point(area.vertices[0].point);
		size.fit_around_point(area.vertices[1].point);
		size.fit_around_point(area.vertices[2].point);
		size.fit_around_point(area.vertices[3].point);

		instruction.width = abs(size.width()) / _fieldWidth;
		instruction.height = abs(size.height()) / _fieldHeight;
	}

	void NodeDistanceFields2D::dispatch_in_main_thread(function<void(IComputeShader&)> main_thread_callback)
	{
		_distZeros.reserve(10000); // TODO: keep count of elements in _tempDistZeros

		s32 size_sum = 0;
		for (s32 i = 0; i < _tempDistZeros.size(); i++)
		{
			auto& temp_dist_zeros = _tempDistZeros[i];

			ASSERT(i < _instructions.size());
			auto& instruction = _instructions[i];
			instruction.index = size_sum;
			size_sum += (s32)temp_dist_zeros.size();;
			instruction.count = (s32)temp_dist_zeros.size();
			_distZeros.insert(_distZeros.end(), temp_dist_zeros.begin(), temp_dist_zeros.end());
		}

		auto cs = utility::renderer().find_compute_shader("Distance Field");
		if (cs == nullptr)
		{
			ERROR("Distance Field compute shader not found!");
			return;
		}

		auto h = (handle<NodeDistanceFields2D>)handle_from_this();

		ASSERT(cs != nullptr);
		cs->dispatch_to_main_thread(
			function([h](IComputeShader& cs)
			{
				ASSERT(h != nullptr);

				if (h != nullptr)
				{
					NIGHT_PROFILER_SCOPED(DF_dispatch_in_main_thread);
					const s32 workGroupSizeX = 10;
					const s32 workGroupSizeY = 10;
					const s32 workGroupSizeZ = 10;

					s32 numGroupsX = (h->_fieldWidth + workGroupSizeX - 1) / workGroupSizeX;
					s32 numGroupsY = (h->_fieldHeight + workGroupSizeY - 1) / workGroupSizeY;
					s32 numGroupsZ = (h->_fieldCount + workGroupSizeZ - 1) / workGroupSizeZ;

					// TODO: execute 1 row of dists per fragment instead of 1:1.
					cs.num_groups = ivec3(numGroupsX, numGroupsY, numGroupsZ);

					cs.data(h->_fields.data(), h->_fields.size() * sizeof(*h->_fields.data()), 0);
					cs.data(h->_distZeros.data(), h->_distZeros.size() * sizeof(*h->_distZeros.data()), 1);
					cs.data(h->_instructions.data(), h->_instructions.size() * sizeof(*h->_instructions.data()), 2);

					cs.uniform1i("u_width", h->_fieldWidth);
					cs.uniform1i("u_height", h->_fieldHeight);

					cs.compute();

					cs.data(h->_fields.data(), 0);

#if defined(NIGHT_ENABLE_DEBUG_RENDERER) && defined(NIGHT_DEBUG)
					for (s32 k = 0; k < h->_fieldCount; k++)
					{
						real* fragments = h->field(k);
						SurfaceParams params;
						params.width = h->_fieldWidth;
						params.height = h->_fieldHeight;
						params.fill_color = COLOR_ZERO;
						sref<Surface> surface(new Surface(params));
						ASSERT(surface != nullptr);
						Color8* const& pixels = surface->pixels();

						for (s32 y = 0; y < h->_fieldHeight; y++)
						{
							for (s32 x = 0; x < h->_fieldWidth; x++)
							{
								real scalar = 1.0f - real(fragments[x + y * h->_fieldWidth]); // TODO: get max distance
								if (scalar < 0.0f)
								{
									scalar = 0.0f;
								}
								pixels[x + y * h->_fieldWidth] = Color8(RED.opaqued(scalar));
							}
						}

						auto tx = utility::renderer().create_texture(h->name() + to_string(h->unique_id()) + " db_fim_udf: " + to_string(k), { .surface = surface });
						h->__debugTextures.push_back(tx);
					}
#endif
				}
			})
		, main_thread_callback);
	}

	vec2 NodeDistanceFields2D::compare_fields(s32 field_a_index, s32 field_b_index)
	{
		vec2 result = vec2(0, 0);

		ASSERT(field_a_index < _fieldCount);
		ASSERT(field_b_index < _fieldCount);
		ASSERT(field_a_index < _tempDistZeros.size());
		ASSERT(field_b_index < _tempDistZeros.size());

		real* field_a = field(field_a_index);
		real* field_b = field(field_b_index);

		vector<ivec2> const& zeros_a = _tempDistZeros[field_a_index];
		vector<ivec2> const& zeros_b = _tempDistZeros[field_b_index];

		real dist_sum_a = 0;
		real dist_sum_b = 0;

		{
			s32 i;
			for (i = 0; i < zeros_b.size(); i++)
			{
				ivec2 const& zero = zeros_b[i];
				real const& dist_a = field_a[zero.x + zero.y * field_width()];
				dist_sum_a += dist_a;
			}

			if (i > 0)
			{
				dist_sum_b /= i;
			}
			else
			{
				dist_sum_b = -1;
			}
		}

		{
			s32 i;
			for (i = 0; i < zeros_a.size(); i++)
			{
				ivec2 const& zero = zeros_a[i];
				real const& dist_b = field_b[zero.x + zero.y * field_width()];
				dist_sum_b += dist_b;
			}

			if (i > 0)
			{
				dist_sum_b /= i;
			}
			else
			{
				dist_sum_b = -1;
			}
		}
		
		// TODO: finish function.
		return { dist_sum_a, dist_sum_b };
	}

//#ifdef NIGHT_ENABLE_DEBUG_RENDERER
//	template <> void DebugRenderer::draw_format(NodeDistanceFields2D& v)
//
//#endif

}