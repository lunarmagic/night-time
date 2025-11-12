
#include "nightpch.h"
#include "NodeAnimation.h"

namespace night
{
	NodeAnimation::NodeAnimation(NodeAnimationParams const& params)
	{
		initialize(params);
	}

	void NodeAnimation::initialize(NodeAnimationParams const& params)
	{
		clear();
		keyframes(params.keyframes);
		playback_speed = params.playback_speed;
		should_loop = params.should_loop;
		should_render_when_paused = params.should_render_when_paused;
		_state = ENodeAnimationState::Reset;
		_onFinish = params.on_finish;
	}

	// TODO: delay play until after lag spike.
	u8 NodeAnimation::play()
	{
		if (_keyframes.empty())
		{
			WARNING("node animation ", name_and_id(), "'s keyframes are empty");
			return false;
		}

		if (_state == ENodeAnimationState::Playing)
		{
			WARNING("node animation ", name_and_id(), " is already playing");
			return false;
		}

		_prevPlaytime = -1.0f;
		_playtime = 0.0f;
		_state = ENodeAnimationState::Playing;
		_currentKeyframe = 0;
		_keyframeAccumulation = 0.0f;
		return true;
	}

	u8 NodeAnimation::pause()
	{
		if (_state == ENodeAnimationState::Paused || _state == ENodeAnimationState::Reset)
		{
			return false;
		}

		_state = ENodeAnimationState::Paused;
		return true;
	}

	u8 NodeAnimation::resume()
	{
		if(_state == ENodeAnimationState::Playing || _state == ENodeAnimationState::Finished || _state == ENodeAnimationState::Reset)
		{
			return false;
		}

		_state = ENodeAnimationState::Playing;
		return true;
	}

	void NodeAnimation::reset()
	{
		_state = ENodeAnimationState::Reset;
		_prevPlaytime = -1.0f;
		_playtime = -1.0f;
		_currentKeyframe = 0;
		_keyframeAccumulation = 0.0f;
	}

	void NodeAnimation::keyframes(vector<AnimationKeyframe> const& keyframes)
	{
		_animationLength = 0.0f;
		_keyframes = keyframes;
		for (const auto& i : _keyframes)
		{
			ASSERT(i.length >= 0.0f);
			_animationLength += i.length;
		}
	}

	void NodeAnimation::clear()
	{
		playback_speed = 1.0f;
		should_loop = false;
		should_render_when_paused = true;

		_state = ENodeAnimationState::Reset;
		_animationLength = -1.0f;
		_prevPlaytime = -1;
		_playtime = -1;
		_keyframes.clear();
		_currentKeyframe = 0;
		_keyframeAccumulation = 0.0f;
	}

	real NodeAnimation::animation_time() const
	{
		if (_state == ENodeAnimationState::Reset)
		{
			return 0.0f;
		}

		return _prevPlaytime / _animationLength;
	}

	void NodeAnimation::finish()
	{
		// TODO: force trigger t=1 last keyframe
		// trigger callbacks:
		for (s32 i = _currentKeyframe; i < _keyframes.size(); i++)
		{
			if (_keyframes[i].on_complete != nullptr)
			{
				_keyframes[i].on_complete();
			}
		}

		_playtime = _animationLength;
		_state = ENodeAnimationState::Finished;

		if (_onFinish != nullptr)
		{
			_onFinish();
		}
	}

	void NodeAnimation::loop()
	{
		s32 max_iterations = 1000;

		// trigger callbacks:
		for (s32 i = 0; _playtime > _animationLength && i < max_iterations; i++)
		{
			for (s32 j = _currentKeyframe; j < _keyframes.size(); j++)
			{
				if (_keyframes[j].on_complete != nullptr)
				{
					_keyframes[j].on_complete();
				}
			}

			_playtime -= _animationLength;
			_currentKeyframe = 0;
		}

		// the remainder:
		_keyframeAccumulation = 0.0f;
		for (s32 i = 0; i < _keyframes.size(); i++)
		{
			AnimationKeyframe& keyframe = _keyframes[i];
			_keyframeAccumulation += keyframe.length;
			if (_keyframeAccumulation > _playtime)
			{
				for (s32 j = _currentKeyframe; j < i; j++)
				{
					ASSERT(_keyframes[j].on_complete != nullptr);
					_keyframes[j].on_complete();
				}

				_currentKeyframe = i;
				break;
			}
		}
	}

	void NodeAnimation::on_update(real delta)
	{
		if (_state == ENodeAnimationState::Finished && should_loop)
		{
			_state = ENodeAnimationState::Playing;
		}
		else if (_state != ENodeAnimationState::Playing)
		{
			_prevPlaytime = _playtime;
			return;
		}

		if (_playtime > _animationLength)
		{
			if (!should_loop)
			{
				finish();
				return;
			}
			else
			{
				loop();
			}
		}
		else
		{
			_keyframeAccumulation = 0.0f;
			for (s32 i = 0; i < _keyframes.size(); i++)
			{
				AnimationKeyframe& keyframe = _keyframes[i];
				_keyframeAccumulation += keyframe.length;
				if (_keyframeAccumulation > _playtime)
				{
					// trigger callbacks:
					for (s32 j = _currentKeyframe; j < i; j++)
					{
						if (_keyframes[j].on_complete != nullptr)
						{
							_keyframes[j].on_complete();
						}
					}

					_currentKeyframe = i;
					break;
				}
			}
		}

		_prevPlaytime = _playtime;
		_playtime += delta * playback_speed;
	}

	void NodeAnimation::on_render(RenderGraph& out_graph) const
	{
		if (_state == ENodeAnimationState::Reset)
		{
			return;
		}

		if (_state != ENodeAnimationState::Playing && !should_render_when_paused)
		{
			return;
		}

		if (_keyframes.empty())
		{
			return; // TODO: investigate
		}

		ASSERT(_currentKeyframe >= 0 && _currentKeyframe < _keyframes.size());
		AnimationKeyframe const& keyframe = _keyframes[_currentKeyframe];

		if (keyframe.animation_function != nullptr)
		{
			KeyframeParams kf_params = {
				.out_graph = out_graph,
				.keyframe_time = 1.0f - (_keyframeAccumulation - _prevPlaytime) / keyframe.length,
				.animation_time = _prevPlaytime / _animationLength
			};

			keyframe.animation_function(kf_params);
		}
		
	}
}