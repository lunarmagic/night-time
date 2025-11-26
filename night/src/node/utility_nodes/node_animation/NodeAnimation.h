#pragma once

#include "node/NodeRenderable.h"

namespace night
{

	enum struct ENodeAnimationState : s32
	{
		Reset = 0,
		Playing,
		Paused,
		Finished
	};

	struct AnimationKeyframe;

	struct KeyframeParams
	{
		RenderGraph& out_graph;
		real keyframe_time;
		real animation_time;
	};

	struct AnimationKeyframe
	{
		real length = 1.0f;
		function<void(KeyframeParams const&)> animation_function = nullptr;
		function<void()> on_complete = nullptr;
	};

	struct NodeAnimationParams
	{
		vector<AnimationKeyframe> keyframes;
		real playback_speed = 1.0f;
		b8 should_loop = false;
		b8 should_render_when_paused = true;
		function<void()> on_finish = nullptr;
	};

	struct NIGHT_API NodeAnimation : public NodeRenderable
	{
		NodeAnimation(NodeAnimationParams const& params);

		virtual void initialize(NodeAnimationParams const& params);

		virtual b8 play();
		virtual b8 pause();
		virtual b8 resume();
		virtual void reset();
		virtual void finish();

		real const& animation_length() const { return _animationLength; }

		void keyframes(vector<AnimationKeyframe> const& keyframes);
		void clear();

		b8 is_playing() const { return _state == ENodeAnimationState::Playing; }
		b8 is_reset() const { return _state == ENodeAnimationState::Reset; }
		b8 is_finished() const { return _state == ENodeAnimationState::Finished; }
		real animation_time() const;

		ENodeAnimationState state() { return _state; };

		real playback_speed = 1.0f;
		b8 should_loop = false;
		b8 should_render_when_paused = true;

	protected:

		virtual void on_update(real delta) override;

		virtual void on_render(RenderGraph& out_graph) const override;

	private:

		ENodeAnimationState _state;
		real _animationLength = -1.0f;
		real _prevPlaytime = -1;
		real _playtime = -1;
		vector<AnimationKeyframe> _keyframes;
		s32 _currentKeyframe = 0;
		real _keyframeAccumulation = 0.0f;
		function<void()> _onFinish = nullptr;

		void loop();
	};

}