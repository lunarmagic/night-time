
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
#include "application/Application.h"

namespace night
{

	NodeDistanceFields2D::~NodeDistanceFields2D()
	{
		clear();
	}

	void NodeDistanceFields2D::init(NodeDistanceFields2DParams const& params)
	{
		clear();

		_fieldCount = params.field_count;
		_fieldWidth = params.field_width;
		_fieldHeight = params.field_height;
		//_shouldUseGPU = params.should_use_gpu;

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

			Quad<> const& area = rasterizer.area();
			AABB size; // TODO: get actual width and height of area.
			size.fit_around_point(area.vertices[0].point);
			size.fit_around_point(area.vertices[1].point);
			size.fit_around_point(area.vertices[2].point);
			size.fit_around_point(area.vertices[3].point);

			instruction.width = abs(size.width()) / _fieldWidth;
			instruction.height = abs(size.height()) / _fieldHeight;
		}

#if NIGHT_ENABLE_DEBUG_RENDERER
		for (const auto& i : __debugTextures)
		{
			utility::renderer().destroy_texture(i);
		}

		__debugTextures.clear();

		__debugTextures = vector<handle<ITexture>>(_fieldCount, nullptr);
#endif
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

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		for (const auto& i : __debugTextures)
		{
			utility::renderer().destroy_texture(i);
		}

		__debugTextures.clear();
#endif
	}

	void NodeDistanceFields2D::draw_point(ivec2 internal_point, s32 field_index)
	{
		ASSERT(field_index < _fieldCount);
		ASSERT(field_index < _rasterizers.size());
		ASSERT(field_index < _instructions.size());
		ASSERT(false); // TODO: implement
	}

	void NodeDistanceFields2D::draw_point(vec2 point, s32 field_index)
	{
		ASSERT(field_index < _fieldCount);
		ASSERT(field_index < _rasterizers.size());
		ASSERT(field_index < _instructions.size());
		ASSERT(false); // TODO: implement
	}

	void NodeDistanceFields2D::draw_line(ivec2 internal_p1, ivec2 internal_p2, s32 field_index)
	{
		ASSERT(field_index < _rasterizers.size());

		ASSERT(field_index < _tempDistZeros.size());
		auto& temp_dist_zeros = _tempDistZeros[field_index];

		auto& rasterizer = _rasterizers[field_index];
		rasterizer.draw_line(internal_p1, internal_p2, [&](auto const& fragment)
		{
			ASSERT(fragment.fragment != nullptr);
			if (*fragment.fragment != 0.0f)
			{
				*fragment.fragment = 0.0f;
				temp_dist_zeros.push_back(fragment.coordinate);
			}
		}, false);
	}

	void NodeDistanceFields2D::draw_line(vec2 p1, vec2 p2, s32 field_index)
	{
		ASSERT(field_index < _rasterizers.size());

		ASSERT(field_index < _tempDistZeros.size());
		auto& temp_dist_zeros = _tempDistZeros[field_index];

		auto& rasterizer = _rasterizers[field_index];
		rasterizer.draw_line(p1, p2, [&](auto const& fragment)
		{
			ASSERT(fragment.fragment != nullptr);
			if (*fragment.fragment != 0.0f)
			{
				*fragment.fragment = 0.0f;
				temp_dist_zeros.push_back(fragment.coordinate);
			}
		}, false);
	}

	void NodeDistanceFields2D::area(Quad<> const& area, s32 field_index)
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

	Quad<> const& NodeDistanceFields2D::area(s32 field_index) const
	{
		ASSERT(field_index < _rasterizers.size());
		auto& rasterizer = _rasterizers[field_index];
		return rasterizer.area();
	}

#if 0
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
					//NIGHT_PROFILER_SCOPED(DF_dispatch_in_main_thread);
					NIGHT_PROFILER_PUSH(h->name() + ": " + "DF_dispatch_in_main_thread");
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
					NIGHT_PROFILER_POP();

#if defined(NIGHT_ENABLE_DEBUG_RENDERER) && defined(NIGHT_DEBUG_SAVE_FIELD_TEXTURES)
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

	void NodeDistanceFields2D::dispatch_in_this_thread()
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

		NIGHT_PROFILER_SCOPED(DF_dispatch_in_main_thread);
		const s32 workGroupSizeX = 10;
		const s32 workGroupSizeY = 10;
		const s32 workGroupSizeZ = 10;

		s32 numGroupsX = (_fieldWidth + workGroupSizeX - 1) / workGroupSizeX;
		s32 numGroupsY = (_fieldHeight + workGroupSizeY - 1) / workGroupSizeY;
		s32 numGroupsZ = (_fieldCount + workGroupSizeZ - 1) / workGroupSizeZ;

		// TODO: execute 1 row of dists per fragment instead of 1:1.
		cs->num_groups = ivec3(numGroupsX, numGroupsY, numGroupsZ);

		cs->data(_fields.data(), _fields.size() * sizeof(*_fields.data()), 0);
		cs->data(_distZeros.data(), _distZeros.size() * sizeof(*_distZeros.data()), 1);
		cs->data(_instructions.data(), _instructions.size() * sizeof(*_instructions.data()), 2);

		cs->uniform1i("u_width", _fieldWidth);
		cs->uniform1i("u_height", _fieldHeight);

		cs->compute();

		cs->data(_fields.data(), 0);

#if defined(NIGHT_ENABLE_DEBUG_RENDERER) && defined(NIGHT_DEBUG_SAVE_FIELD_TEXTURES)
		{
			for (s32 k = 0; k < _fieldCount; k++)
			{
				real* fragments = field(k);
				SurfaceParams params;
				params.width = _fieldWidth;
				params.height = _fieldHeight;
				params.fill_color = COLOR_ZERO;
				sref<Surface> surface(new Surface(params));
				ASSERT(surface != nullptr);
				Color8* const& pixels = surface->pixels();

				for (s32 y = 0; y < _fieldHeight; y++)
				{
					for (s32 x = 0; x < _fieldWidth; x++)
					{
						real scalar = 1.0f - real(fragments[x + y * _fieldWidth]); // TODO: get max distance
						if (scalar < 0.0f)
						{
							scalar = 0.0f;
						}
						pixels[x + y * _fieldWidth] = Color8(RED.opaqued(scalar));
					}
				}

				auto tx = utility::renderer().create_texture(name() + to_string(unique_id()) + " db_fim_udf: " + to_string(k), { .surface = surface });
				__debugTextures.push_back(tx);
			}
		}
#endif
	}
#endif

	//void NodeDistanceFields2D::draw_line(NDF2DDrawLineParams const& params)
	//{
	//	auto& rasterizer = this->rasterizer(params.field_index);
	//	vector<ivec2>& dist_zeros = distance_zeros(params.field_index);

	//	rasterizer.draw_line(params.p1, params.p2, [&](LineFragmentData<real> const& fragment)
	//		{
	//			ASSERT(fragment.fragment != nullptr);
	//			if (*fragment.fragment == 0.0f)
	//			{
	//				return;
	//			}

	//			*fragment.fragment = 0.0f;

	//			dist_zeros.push_back(fragment.coordinate);
	//		}, false);
	//}

	//void NodeDistanceFields2D::compute_in_main_thread(function<void()> main_thread_callback)
	//{
	//	Application::get().queue_for_main_thread([main_thread_callback, self = (handle<NodeDistanceFields2D>)handle_from_this()]()
	//		{
	//			ASSERT(self != nullptr);
	//			self->compute_in_this_thread();

	//			if (main_thread_callback != nullptr)
	//			{
	//				main_thread_callback();
	//			}
	//		});
	//}

	void NodeDistanceFields2D::compute_in_this_thread(u8 use_gpu, s32 begin, s32 end)
	{
		s32 count = (begin == -1 || end == -1) ? _fieldCount : end - begin;
		begin = (begin == -1 || end == -1) ? 0 : begin;

		NIGHT_PROFILER_SCOPED("NodeDistanceFields2D::compute_in_this_thread");
		_distZeros.clear();
		_distZeros.reserve(10000); // TODO: keep count of elements in _tempDistZeros

		s32 size_sum = 0;
		for (s32 i = begin; i < begin + count; i++)
		{
			auto& temp_dist_zeros = _tempDistZeros[i];

			ASSERT(i < _instructions.size());
			auto& instruction = _instructions[i];
			instruction.index = size_sum;
			size_sum += (s32)temp_dist_zeros.size();;
			instruction.count = (s32)temp_dist_zeros.size();
			_distZeros.insert(_distZeros.end(), temp_dist_zeros.begin(), temp_dist_zeros.end());
		}

		if(use_gpu)
		{
			auto cs = utility::renderer().find_compute_shader("Distance Field");
			if (cs == nullptr)
			{
				ERROR("Distance Field compute shader not found!");
				return;
			}

			const s32 workGroupSizeX = 10;
			const s32 workGroupSizeY = 10;
			const s32 workGroupSizeZ = 10;

			s32 numGroupsX = (_fieldWidth + workGroupSizeX - 1) / workGroupSizeX;
			s32 numGroupsY = (_fieldHeight + workGroupSizeY - 1) / workGroupSizeY;
			//s32 numGroupsZ = (_fieldCount + workGroupSizeZ - 1) / workGroupSizeZ;
			s32 numGroupsZ = (count + workGroupSizeZ - 1) / workGroupSizeZ;

			// TODO: execute 1 row of dists per fragment instead of 1:1.
			cs->num_groups = ivec3(numGroupsX, numGroupsY, numGroupsZ);

			cs->data(field(begin), count * _fieldWidth * _fieldHeight * sizeof(*_fields.data()), 0);
			cs->data(_distZeros.data(), _distZeros.size() * sizeof(*_distZeros.data()), 1);
			cs->data(&_instructions[begin], count * sizeof(*_instructions.data()), 2);
			//cs->data(_fields.data(), _fields.size() * sizeof(*_fields.data()), 0);
			//cs->data(_distZeros.data(), _distZeros.size() * sizeof(*_distZeros.data()), 1);
			//cs->data(_instructions.data(), _instructions.size() * sizeof(*_instructions.data()), 2);

			cs->uniform1i("u_width", _fieldWidth);
			cs->uniform1i("u_height", _fieldHeight);

			cs->compute();

			//cs->data(_fields.data(), 0);
			cs->data(field(begin), 0);
		}
		else
		{
			for (s32 i = begin; i < begin + count; i++)
			{
				real* field = this->field(i);
				Instruction const& instruction = _instructions[i];

				if (instruction.count == 0)
				{
					continue;
				}

				for (s32 y = 0; y < _fieldHeight; y++)
				{
					for (s32 x = 0; x < _fieldWidth; x++)
					{
						real& pixel = field[x + y * _fieldWidth];

						for (s32 k = 0; k < instruction.count; k++)
						{
							ivec2 const& dist_zero = _distZeros[instruction.index + k];
							real dist = real(abs(x - dist_zero.x)) * instruction.width + real(abs(y - dist_zero.y))* instruction.height;
							if (dist < pixel)
							{
								pixel = dist;
							}
						}
					}
				}
			}

			//ivec3 point = ivec3(gl_GlobalInvocationID.xyz);
			//int fragment_index = _index(point, ivec2(u_width, u_height));
			//
			//float min_dist = udfs[fragment_index];
			//if (min_dist == 0)
			//{
			//	return;
			//}
			//
			//int instruction_index = fragment_index / (u_width * u_height);
			//
			//Instruction instruction = instructions[point.z];
			//
			//if (instruction.count == 0)
			//{
			//	return;
			//}
			//
			//for (int i = 0; i < instruction.count; i++)
			//{
			//	ivec2 dist_zero = dist_zeros[instruction.index + i];
			//	float dist = float(abs(point.x - dist_zero.x)) * instruction.width + float(abs(point.y - dist_zero.y)) * instruction.height;
			//
			//	if (dist < min_dist)
			//	{
			//		min_dist = dist;
			//	}
			//}
			//
			//udfs[fragment_index] = min_dist;
		}

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		if(DebugRenderer::should_save_debug_textures())
		{
			NIGHT_PROFILER_SCOPED("NodeDistanceFields2D::create_debug_textures");

			for (s32 k = begin; k < begin + count; k++)
			{
				auto& texture = __debugTextures[k];
				if (texture != nullptr)
				{
					utility::renderer().destroy_texture(texture);
				}

				real* fragments = field(k);
				SurfaceParams params;
				params.width = _fieldWidth;
				params.height = _fieldHeight;
				params.fill_color = COLOR_ZERO;
				sref<Surface> surface(new Surface(params));
				ASSERT(surface != nullptr);
				Color8* const& pixels = surface->pixels();

				for (s32 y = 0; y < _fieldHeight; y++)
				{
					for (s32 x = 0; x < _fieldWidth; x++)
					{
						real scalar = 1.0f - real(fragments[x + y * _fieldWidth]); // TODO: get max distance
						if (scalar < 0.0f)
						{
							scalar = 0.0f;
						}
						pixels[x + y * _fieldWidth] = Color8(RED.opaqued(scalar));
					}
				}

				texture = utility::renderer().create_texture(name() + to_string(unique_id()) + " db_fim_udf: " + to_string(k), { .surface = surface });
			}
		}
#endif
	}

	real NodeDistanceFields2D::compare_fields(s32 field_a_index, s32 field_b_index)
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
		s32 count = 0;

		for (s32 i = 0; i < zeros_b.size(); i++)
		{
			ivec2 const& zero = zeros_b[i];
			real const& dist_a = field_a[zero.x + zero.y * field_width()];
			dist_sum_a += dist_a;
			count++;
		}

		for (s32 i = 0; i < zeros_a.size(); i++)
		{
			ivec2 const& zero = zeros_a[i];
			real const& dist_b = field_b[zero.x + zero.y * field_width()];
			dist_sum_b += dist_b;
			count++;
		}

		real dist_sum = dist_sum_a + dist_sum_b;

		if (count > 0)
		{
			dist_sum /= count;
		}
		
		return dist_sum;
	}

	s32 NodeDistanceFields2D::field_index(s32 x, s32 y) const
	{
		return x + y * _fieldWidth;
	}

//#ifdef NIGHT_ENABLE_DEBUG_RENDERER
//	template <> void DebugRenderer::draw_format(NodeDistanceFields2D& v)
//
//#endif

}