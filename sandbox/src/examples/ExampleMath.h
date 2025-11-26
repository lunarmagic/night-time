#pragma once

#include "ExampleBase.h"
#include "math/Math.h"

namespace night
{
	struct ExampleMath : public ExampleBase
	{
		//enum struct EEase : s32
		//{
		//	In = 0,
		//	Out,
		//	InOut,
		//	Max
		//};

		ExampleMath()
			: ExampleBase(ECameraType::Orthographic)
		{
			
		}

		virtual void update_gui(handle<NodeGui> gui)
		{
			ASSERT(gui != nullptr);

			gui->seperator();

			gui->text("Easing:");
			gui->drag_real("Exponent", &_easeExponent, 0.01f, 0.0f, INFINITY);
			//if (gui->button("Easing Function"))
			//{
			//	_easingFunction = EEase(((s32)_easingFunction + 1) % (s32)EEase::Max);
			//}
		}

	protected:

		virtual void on_render(RenderGraph& out_graph) const override
		{
			s32 segments = 100;
			for (s32 i = 1; i < segments; i++)
			{
				real t1 = (real)(i - 1) / (real)(segments - 1);
				real t2 = (real)(i) / (real)(segments - 1);

				vec2 p1;
				vec2 p2;

				p1.x = math::lerp(-1, 1, t1);
				p2.x = math::lerp(-1, 1, t2);

				//if (_easingFunction == EEase::In)
				//{
				//	p1.y = math::ease_in(t1, _easeExponent) * 2.0f - 1.0f;
				//	p2.y = math::ease_in(t2, _easeExponent) * 2.0f - 1.0f;
				//}
				//else if (_easingFunction == EEase::Out)
				//{
				//	p1.y = math::ease_out(t1, _easeExponent) * 2.0f - 1.0f;
				//	p2.y = math::ease_out(t2, _easeExponent) * 2.0f - 1.0f;
				//}
				//else if (_easingFunction == EEase::InOut)
				//{
				//	p1.y = math::ease_in_out(t1, _easeExponent) * 2.0f - 1.0f;
				//	p2.y = math::ease_in_out(t2, _easeExponent) * 2.0f - 1.0f;
				//}

				p1.y = math::ease(t1, _easeExponent) * 2.0f - 1.0f;
				p2.y = math::ease(t2, _easeExponent) * 2.0f - 1.0f;

				out_graph.draw_line(vec2(-2, -2), vec2(2, -2), BLACK.opaqued(0.75f));
				out_graph.draw_line(vec2(2, -2), vec2(2, 2), BLACK.opaqued(0.75f));
				out_graph.draw_line(vec2(2, 2), vec2(-2, 2), BLACK.opaqued(0.75f));
				out_graph.draw_line(vec2(-2, 2), vec2(-2, -2), BLACK.opaqued(0.75f));
				out_graph.draw_line(p1 * (real)2, p2 * (real)2, BLACK);
			}
		}

	private:

		//EEase _easingFunction = EEase::In;
		real _easeExponent = 1.0f;
	};
}