#pragma once

#include "ExampleBase.h"
#include "node/utility_nodes/node_animation/NodeAnimation.h"

namespace night
{
	struct ExampleNodeAnimation : public ExampleBase
	{
		ExampleNodeAnimation()
			: ExampleBase()
		{
			NodeAnimationParams nap;

			AnimationKeyframe key_1;
			key_1.length = 0.76f;

			key_1.animation_function = [&](KeyframeParams const& params)
				{
					vec2 p2;
					p2.x = sin(params.keyframe_time * R_PI * 2) * 2;
					p2.y = cos(params.keyframe_time * R_PI * 2) * 2;
					params.out_graph.draw_line(vec2(ORIGIN), p2, BLUE);
				};

			key_1.on_complete = []()
				{
					TRACE("Keyframe 1 Completed.");
				};

			AnimationKeyframe key_2;
			key_2.length = 2.33f;

			key_2.animation_function = [&](KeyframeParams const& params)
				{
					vec2 p2;
					p2.x = sin((1.0f - params.keyframe_time) * R_PI * 2) * 2;
					p2.y = cos((1.0f - params.keyframe_time) * R_PI * 2) * 2;
					params.out_graph.draw_line(vec2(ORIGIN), p2, RED);
				};

			key_2.on_complete = []()
				{
					TRACE("Keyframe 2 Completed.");
				};

			AnimationKeyframe key_3;
			key_3.length = 0.2f;

			key_3.animation_function = [&](KeyframeParams const& params)
				{
					vec2 p2;
					p2.x = sin(params.keyframe_time * R_PI * 2) * 2;
					p2.y = cos(params.keyframe_time * R_PI * 2) * 2;
					params.out_graph.draw_line(vec2(ORIGIN), p2, PURPLE);
				};

			key_3.on_complete = []()
				{
					TRACE("Keyframe 3 Completed.");
				};

			nap.keyframes = {
				key_1,
				key_2,
				key_3
			};

			nap.playback_speed = 1.0f;
			nap.should_loop = false;
			nap.should_render_when_paused = true;

			_animation = create<NodeAnimation>("Animation", nap);
		}

		virtual void update_gui(handle<NodeGui> gui)
		{
			ASSERT(gui != nullptr);

			ASSERT(_animation != nullptr);

			switch (_animation->state())
			{
			case ENodeAnimationState::Reset:
			{
				gui->text("State: Reset", WHITE);
				break;
			}
			case ENodeAnimationState::Playing:
			{
				gui->text("State: Playing", GREEN);
				break;
			}
			case ENodeAnimationState::Paused:
			{
				gui->text("State: Paused", ORANGE);
				break;
			}
			case ENodeAnimationState::Finished:
			{
				gui->text("State: Finished", CYAN);
				break;
			}
			}

			if (gui->button("Play"))
			{
				_animation->play();
			}

			if (gui->button("Pause"))
			{
				_animation->pause();
			}

			if (gui->button("Resume"))
			{
				_animation->resume();
			}

			if (gui->button("Reset"))
			{
				_animation->reset();
			}

			gui->checkbox("Should Loop", &_animation->should_loop);
			gui->checkbox("Should Render When Paused", &_animation->should_render_when_paused);
			gui->drag_real("Playback Speed", &_animation->playback_speed, 0.025f, 0.0f, 1000.0f);

			gui->begin_canvas("Animation Time", 0.2f, BLACK);
			real animation_time = _animation->animation_time();
			vec2 p1 = vec2(math::lerp(-1, 1, animation_time), -1.0f);
			vec2 p2 = vec2(math::lerp(-1, 1, animation_time), 1.0f);
			gui->canvas_line(p1, p2, WHITE);
			gui->end_canvas();
		}

	private:

		handle<NodeAnimation> _animation = nullptr;
	};
}