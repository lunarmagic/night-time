
#include "nightpch.h"
#include "ExampleSelect.h"
#include "examples/ExampleBase.h"
#include "examples/ExampleNodeAnimation.h"
#include "examples/ExampleConvex2D.h"
#include "examples/ExampleShapeCast2D.h"
#include "examples/ExampleShapeCast3D.h"
#include "examples/ExampleShapeRenderer3D.h"
#include "examples/ExampleMath.h"
#include "examples/ExampleNodeWindow.h"
#include "profiler/Profiler.h"
#include "event/MouseEvent.h"

// TODO: add example for intersection between shapes
// TODO: add example for raycasting
// TODO: add example for curves
// TODO: add example for gjk intersects 2d and 3d
// TODO: add example for rendering.

namespace night
{

	ExampleSelect::ExampleSelect()
		: NodeWindow(NodeWindowParams{
				.state = ENodeWindowState::Fullscreen,
				.dock_where = ENodeWindowDockWhere::Centered,
				.dock_space = { .left = 0.7f, .top = 0.0f, .right = 0.0f, .bottom = 0.0f }
			})
	{
		NodeWindowParams vp_params;
		vp_params.dock_where = ENodeWindowDockWhere::Centered;
		//vp_params.dock_space = AABB{ 0.5f, 0.5f, 0.0f, 0.0f };
		_viewport = create<NodeWindow>("Viewport", vp_params);

		NodeWindowParams gui_params;
		gui_params.dock_where = ENodeWindowDockWhere::Left;
		_gui = create<NodeGui>("Example Select", gui_params);
		_gui->on_close(nullptr);

		NodeRenderTargetParams nrt_params;
		nrt_params.depth = EXAMPLE_RENDER_TARGET_DEPTH;
		nrt_params.clear_color = LIGHT;
		nrt_params.should_use_depth_peeling = true;
		nrt_params.should_use_depth_testing = true;
		nrt_params.should_use_blending = true;
		nrt_params.should_automatically_clear = true;
		nrt_params.should_automatically_render = true;

		nrt_params.camera.type = ECameraType::Orthographic;
		nrt_params.camera.translation = FORWARD * (real)10;
		nrt_params.camera.look_at = ORIGIN;
		nrt_params.camera.up = UP;
		nrt_params.camera.ortho_region = AABB<>{ .left = -4, .top = 4, .right = 4, .bottom = -4 };
		nrt_params.camera.fov = NIGHT_CAMERA_DEFAULT_FOV;
		nrt_params.camera.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
		nrt_params.camera.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;

		_exampleRenderTarget = _viewport->create<NodeRenderTarget>("Example Render Target", nrt_params);
	
		clear_color = LIGHT;

		{
			ASSERT(_exampleRenderTarget != nullptr);
			_defaultCamera = _exampleRenderTarget->camera();
			_camera = _defaultCamera;

			bind_input(EMouse::Left, EInputType::Pressed, [&]()
				{
					if (!_isPanningCamera && !_isRotatingCamera && !_isZoomingCamera)
					{
						return;
					}

					_isTransformingCamera = _shouldTransformCamera;
				});

			bind_input(EMouse::Left, EInputType::Released, [&]()
				{
					_isTransformingCamera = false;
				});

			bind_input(EKey::LCtrl, EInputType::Pressed, [&]()
				{
					_isRotatingCamera = _shouldRotateCamera;
				});

			bind_input(EKey::LCtrl, EInputType::Released, [&]()
				{
					_isRotatingCamera = false;
				});

			bind_input(EKey::Space, EInputType::Pressed, [&]()
				{
					_isPanningCamera = _shouldPanCamera;
				});

			bind_input(EKey::Space, EInputType::Released, [&]()
				{
					_isPanningCamera = false;
				});

			bind_input(EKey::LShift, EInputType::Pressed, [&]()
				{
					_isZoomingCamera = _shouldZoomCamera;
				});

			bind_input(EKey::LShift, EInputType::Released, [&]()
				{
					_isZoomingCamera = false;
				});

			bind_input(EKey::R, EInputType::Pressed, [&]()
				{
					reset_camera();
				});

			bind_input(EKey::P, EInputType::Pressed, [&]()
				{
					if (_camera.type == ECameraType::Orthographic)
					{
						perspective_mode(ECameraType::Orthographic);
					}
					else
					{
						perspective_mode(ECameraType::Perspective);
					}
				});

			bind_event([&](MouseMotionEvent const& event)
				{
					_deltaMouse = event.motion();
				});
		}

		bind_input(EKey::Q, EInputType::Pressed, [&]()
			{
				utility::window().grab_mouse();
			});

		bind_input(EKey::Q, EInputType::Released, [&]()
			{
				utility::window().release_mouse();
				utility::window().warp_mouse(RIGHT);
			});

		//listen_signal("TEST SIGNAL", [&](SignalParams<real const&> x)
		//	{
		//		PRINT(x.node->name());
		//		PRINT(x.params);
		//	}
		//);
	}

	void ExampleSelect::reset_camera()
	{
		if (!_shouldResetCamera)
		{
			return;
		}

		_camera = _defaultCamera;

		_isRotatingCamera = false;
		_isPanningCamera = false;
		_isTransformingCamera = false;

		_deltaMouse = vec2(0);

		ASSERT(_exampleRenderTarget != nullptr);
		_exampleRenderTarget->camera(_defaultCamera);
	}

	void ExampleSelect::perspective_mode(ECameraType mode)
	{
		_defaultCamera.type = mode;
		_camera.type = mode;
		ASSERT(_exampleRenderTarget != nullptr);
		_exampleRenderTarget->camera(_camera);
	}

	template<typename T>
	inline void ExampleSelect::create_example()
	{
		ASSERT(_activeExample == nullptr);
		_activeExample = _exampleRenderTarget->create<T>("Active Example");

		ASSERT(_activeExample != nullptr);
		perspective_mode(_activeExample->initial_perspective_mode);

		_gui->on_close([self = (handle<ExampleSelect>)handle_from_this()]()
			{
				ASSERT(self != nullptr);
				if (self != nullptr)
				{
					if (self->_activeExample != nullptr)
					{
						self->_activeExample->destroy();
						self->_activeExample = nullptr;
						self->_gui->on_close(nullptr);
					}
				}
			});
	}

	void ExampleSelect::on_update(real delta)
	{
		__super::on_update(delta);

		{
			// TODO: use delta
			if (!is_taking_events) // TODO: make better solution
			{
				return;
			}

			Camera prev_camera = _camera;

			vec2 dm = _deltaMouse;

			if (_isTransformingCamera)
			{
				if (_shouldZoomCamera && _isZoomingCamera)
				{
					_camera.fov -= dm.y * _zoomSensitivity;
					_camera.fov = MAX(_camera.fov, 0.0f);
				}
				else if (_shouldRotateCamera && _isRotatingCamera)
				{
					_camera.translation = math::rotate(R_PI * -dm.x * _rotationSensitivity, UP) * vec4(_camera.translation, 1);
					vec3 right = math::normalize(math::cross(_camera.translation, UP));
					_camera.translation = math::rotate(R_PI * -dm.y * _rotationSensitivity, right) * vec4(_camera.translation, 1);
				}
				else if (_shouldPanCamera && _isPanningCamera)
				{
					vec3 direction = _camera.look_at - _camera.translation;
					real len = math::length(direction);
					direction /= len;

					vec3 dxr = math::normalize(math::cross(direction, -math::normalize(math::cross(direction, _camera.up))));
					vec3 dxu = math::normalize(math::cross(direction, _camera.up));
					_camera.translation -= dxr * dm.y * _panSensitivity * (real)3;
					_camera.translation -= dxu * dm.x * _panSensitivity * (real)3;
					_camera.look_at = _camera.translation + direction * len;
				}
			}
			else if (should_snap_camera_back_to_default)
			{
				// TODO: use slerp
				vec3 tt = (_defaultCamera.translation - _camera.translation) * delta * (real)10;
				vec3 tl = (_defaultCamera.look_at - _camera.look_at) * delta * (real)10;
			
				if (math::length(tt) < 0.001f)
				{
					_camera.translation = _defaultCamera.translation;
				}
				else
				{
					_camera.translation = _camera.translation + tt;
				}
			
				if (math::length(tl) < 0.001f)
				{
					_camera.look_at = _camera.look_at + tl;
				}
				else
				{
					_camera.look_at = _defaultCamera.look_at;
				}
			}

			if (prev_camera.translation != _camera.translation || prev_camera.look_at != _camera.look_at || prev_camera.fov != _camera.fov)
			{
				ASSERT(_exampleRenderTarget != nullptr);
				_exampleRenderTarget->camera(_camera);
			}

			_deltaMouse = vec2(0);
		}

		NIGHT_PROFILER_SCOPED("ExampleSelect::on_update");
		ASSERT(_gui != nullptr);
		NODE_GUI_TEMP_BEGIN(_gui);

		if (_activeExample != nullptr)
		{
			_activeExample->update_gui(_gui);
		}
		else
		{
			if (_gui->button("Math"))
			{
				create_example<ExampleMath>();
			}
			if (_gui->button("NodeWindow"))
			{
				create_example<ExampleNodeWindow>();
			}
			if (_gui->button("NodeAnimation"))
			{
				create_example<ExampleNodeAnimation>();
			}
			if (_gui->button("Convex 2D"))
			{
				create_example<ExampleConvex2D>();
			}
			//if (_gui->button("Shape Cast 2D"))
			//{
			//	create_example<ExampleShapecast2D>();
			//}
			if (_gui->button("Shape Cast"))
			{
				create_example<ExampleShapecast3D>();
			}
			if (_gui->button("Shape Renderer 3D"))
			{
				create_example<ExampleShapeRenderer3D>();
			}
		}

		NODE_GUI_TEMP_END(_gui);
	}

}