#pragma once

#include "node/NodeGui.h"

namespace night
{

	struct ExampleBase;

	struct ExampleSelect : public NodeWindow
	{
		ExampleSelect();

		u8 should_snap_camera_back_to_default = false;

		void perspective_mode(ECameraType mode);

	protected:

		virtual void on_update(real delta) override;

	private:

		template<typename T>
		void create_example();

		handle<ExampleBase> _activeExample = nullptr;

		// create examples under _viewport.
		handle<NodeWindow> _viewport = nullptr;
		handle<NodeRenderTarget> _exampleRenderTarget = nullptr;

		// _gui is passed into example gui update pure virtual function
		handle<NodeGui> _gui = nullptr;

		vec2 _deltaMouse{ 0 };

		Camera _defaultCamera;
		Camera _camera;
		real _panSensitivity{ 1.0f };
		real _rotationSensitivity{ 1.0f };
		real _zoomSensitivity{ DEGREES(1.0f) };

		u8 _isTransformingCamera = false;
		u8 _isRotatingCamera = false;
		u8 _isPanningCamera = false;
		u8 _isZoomingCamera = false;

		u8 _shouldTransformCamera{ true };
		u8 _shouldPanCamera{ true };
		u8 _shouldZoomCamera{ true };
		u8 _shouldRotateCamera{ true };
		u8 _shouldResetCamera{ true };

		void reset_camera();
	};

}