
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

		_fragments = vector<r32>(_fieldWidth * _fieldHeight * _fieldCount, INFINITY);
		_ssboInstructions = vector<Instruction>(_fieldCount, Instruction{});
		_FIELDS = vector<DistanceField>(_fieldCount, DistanceField{});
		//_rasterizers = vector<WorldSpaceRasterizer2D<r32>>(_fieldCount, WorldSpaceRasterizer2D<r32>{});
		//_tempDistZeros = vector<vector<ivec2>>(_fieldCount);

		for (s32 i = 0; i < _FIELDS.size(); i++)
		{
			DistanceField& field = _FIELDS[i];
			//ASSERT(i < _ssboInstructions.size());
			//auto& instruction = _ssboInstructions[i];

			field.rasterizer = WorldSpaceRasterizer2D<r32>(&_fragments[i * field_size()], _fieldWidth, _fieldHeight, params.area);

			//AABB size; // TODO: get actual width and height of area.
			//size.fit_around_point(params.area.vertices[0].point);
			//size.fit_around_point(params.area.vertices[1].point);
			//size.fit_around_point(params.area.vertices[2].point);
			//size.fit_around_point(params.area.vertices[3].point);
			//
			//instruction.width = abs(size.width()) / _fieldWidth;
			//instruction.height = abs(size.height()) / _fieldHeight;
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
		_fragments.clear();
		_ssboDistanceZeros.clear();
		_ssboInstructions.clear();
		_FIELDS.clear();

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

	void DistanceField::draw_point(ivec2 internal_point)
	{
		ASSERT(false); // TODO: implement
	}

	void DistanceField::draw_point(vec2 point)
	{
		ASSERT(false); // TODO: implement
	}

	void DistanceField::draw_line(ivec2 internal_p1, ivec2 internal_p2)
	{
		rasterizer.draw_line(internal_p1, internal_p2, [&](auto const& fragment)
		{
			ASSERT(fragment.fragment != nullptr);
			if (*fragment.fragment != 0.0f)
			{
				*fragment.fragment = 0.0f;
				distance_zeros.push_back(fragment.coordinate);
			}
		}, false);
	}

	void DistanceField::draw_line(vec2 p1, vec2 p2)
	{
		rasterizer.draw_line(p1, p2, [&](auto const& fragment)
		{
			ASSERT(fragment.fragment != nullptr);
			if (*fragment.fragment != 0.0f)
			{
				*fragment.fragment = 0.0f;
				distance_zeros.push_back(fragment.coordinate);
			}
		}, false);
	}

	real DistanceField::dot(DistanceField const& a, DistanceField const& b)
	{
		// TODO: may not need r64
		ASSERT(a.rasterizer.width() * a.rasterizer.height() == b.rasterizer.width() * b.rasterizer.height());

		u64 fields_size = a.rasterizer.width() * a.rasterizer.height();
		pair<r64, r64> mags = { 0.0f, 0.0f };
		vector<pair<r64, r64>> dists(fields_size);

		for (s32 i = 0; i < fields_size; i++)
		{
			dists[i].first = (r64)a.rasterizer.fragments()[i];
			dists[i].second = (r64)b.rasterizer.fragments()[i];
			mags.first += dists[i].first * dists[i].first;
			mags.second += dists[i].second * dists[i].second;
		}

		mags.first = sqrt(mags.first);
		mags.second = sqrt(mags.second);

		r64 product = 0.0f;

		for (s32 i = 0; i < dists.size(); i++)
		{
			auto& d = dists[i];
			product += (d.first / mags.first) * (d.second / mags.second);
		}

		return product;
	}

	//void NodeDistanceFields2D::area(Quad<> const& area, s32 field_index)
	//{
	//	ASSERT(field_index < _rasterizers.size());
	//	auto& rasterizer = _rasterizers[field_index];
	//	rasterizer.area(area);
	//
	//	ASSERT(field_index < _instructions.size());
	//	auto& instruction = _instructions[field_index];
	//
	//	AABB size; // TODO: get actual width and height of area.
	//	size.fit_around_point(area.vertices[0].point);
	//	size.fit_around_point(area.vertices[1].point);
	//	size.fit_around_point(area.vertices[2].point);
	//	size.fit_around_point(area.vertices[3].point);
	//
	//	instruction.width = abs(size.width()) / _fieldWidth;
	//	instruction.height = abs(size.height()) / _fieldHeight;
	//}

	//Quad<> const& NodeDistanceFields2D::area(s32 field_index) const
	//{
	//	ASSERT(field_index < _rasterizers.size());
	//	auto& rasterizer = _rasterizers[field_index];
	//	return rasterizer.area();
	//}

	void NodeDistanceFields2D::compute_in_this_thread(b8 use_gpu, s32 begin, s32 end)
	{
		NIGHT_PROFILER_SCOPED("NodeDistanceFields2D::compute_in_this_thread");

		s32 count = (begin == -1 || end == -1) ? _fieldCount : end - begin;
		begin = (begin == -1 || end == -1) ? 0 : begin;

		_ssboDistanceZeros.clear();
		_ssboDistanceZeros.reserve(10000); // TODO: keep count of elements in _tempDistZeros

		s32 size_sum = 0;
		for (s32 i = begin; i < begin + count; i++)
		{
			DistanceField& field = _FIELDS[i];

			ASSERT(i < _ssboInstructions.size());
			auto& instruction = _ssboInstructions[i];
			

			// set instruction width and height:
			Quad<> const& area = field.rasterizer.area();
			AABB size; // TODO: get actual width and height of area.
			size.fit_around_point(area.vertices[0].point);
			size.fit_around_point(area.vertices[1].point);
			size.fit_around_point(area.vertices[2].point);
			size.fit_around_point(area.vertices[3].point);

			instruction.width = (r32)(abs(size.width()) / _fieldWidth);
			instruction.height = (r32)(abs(size.height()) / _fieldHeight);

			// copy distance zeros to ssbo for this instruction:
			instruction.index = size_sum;
			size_sum += (s32)field.distance_zeros.size();
			instruction.count = (s32)field.distance_zeros.size();
			_ssboDistanceZeros.insert(_ssboDistanceZeros.end(), field.distance_zeros.begin(), field.distance_zeros.end());
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
			s32 numGroupsZ = (count + workGroupSizeZ - 1) / workGroupSizeZ;

			cs->num_groups = ivec3(numGroupsX, numGroupsY, numGroupsZ);

			cs->data(field(begin).rasterizer.fragments(), count * _fieldWidth * _fieldHeight * sizeof(*field(begin).rasterizer.fragments()), 0);
			cs->data(_ssboDistanceZeros.data(), _ssboDistanceZeros.size() * sizeof(*_ssboDistanceZeros.data()), 1);
			cs->data(&_ssboInstructions[begin], count * sizeof(*_ssboInstructions.data()), 2);

			cs->uniform1i("u_width", _fieldWidth);
			cs->uniform1i("u_height", _fieldHeight);

			cs->compute();

			// copy data back from gpu into fragments:
			cs->data(field(begin).rasterizer.fragments(), 0);
		}
		else
		{
			ASSERT(false); // TODO: implement
		//	for (s32 i = begin; i < begin + count; i++)
		//	{
		//		r32* field = this->field(i);
		//		Instruction const& instruction = _instructions[i];
		//
		//		if (instruction.count == 0)
		//		{
		//			continue;
		//		}
		//
		//		for (s32 y = 0; y < _fieldHeight; y++)
		//		{
		//			for (s32 x = 0; x < _fieldWidth; x++)
		//			{
		//				r32& pixel = field[x + y * _fieldWidth];
		//
		//				for (s32 k = 0; k < instruction.count; k++)
		//				{
		//					ivec2 const& dist_zero = _distZeros[instruction.index + k];
		//					r32 dist = r32(abs(x - dist_zero.x)) * instruction.width + r32(abs(y - dist_zero.y))* instruction.height;
		//					if (dist < pixel)
		//					{
		//						pixel = dist;
		//					}
		//				}
		//			}
		//		}
		//	}
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

				r32* fragments = field(k).rasterizer.fragments();
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

	//s32 NodeDistanceFields2D::field_index(s32 x, s32 y) const
	//{
	//	return x + y * _fieldWidth;
	//}

}