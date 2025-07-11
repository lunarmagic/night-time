
#include "nightpch.h"
//#include "nightpch.h"
#include "NodeText.h"
//#include "geometry/Quad.h"
//#include "utility.h"

namespace night
{

	NodeText::NodeText(const NodeTextParams& params)
	{
		_text = params.text;
		_color = params.color;

		local_transform(compose({.scale = params.scale, .translation = params.translation, .rotation = params.rotation}));

		auto texture = utility::renderer().find_texture(params.font);
		ASSERT(texture != nullptr); // font missing
		textures(texture);
	}

	void NodeText::on_update(real delta)
	{
		quat q = quat(vec3(utility::window().time_elapsed() * 0.5f, utility::window().time_elapsed(), utility::window().time_elapsed() * 0.33f));
		local_rotation(q);
	}

	void NodeText::on_render(RenderGraph& out_graph) const
	{
		for (s32 i = 0; i < _text.size(); i++)
		{
			auto& chr = _text[i];

			ivec2 ascii_index;
			ascii_index.x = chr % 16;
			ascii_index.y = chr / 16;

			array<vec2, 4> texture_coords;
			texture_coords[0] = vec2(real(ascii_index.x) / 16.0f, 1.0f - real(ascii_index.y) / 16.0f);
			texture_coords[1] = vec2(real(ascii_index.x + 1) / 16.0f, 1.0f - real(ascii_index.y) / 16.0f);
			texture_coords[2] = vec2(real(ascii_index.x + 1) / 16.0f, 1.0f - real(ascii_index.y + 1) / 16.0f);
			texture_coords[3] = vec2(real(ascii_index.x) / 16.0f, 1.0f - real(ascii_index.y + 1) / 16.0f);

			out_graph.draw_quad(
				{
					.position = vec3(ORIGIN.x + (real)i * 2.0f, ORIGIN.y, 0),
					.size = vec2(1.0f),
					.color = _color,
					.texture_coords = texture_coords
				});
		}
	}

}