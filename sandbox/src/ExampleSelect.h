#pragma once

#include "node/NodeGui.h"

namespace night
{

	struct ExampleBase;

	struct ExampleSelect : public NodeWindow
	{
		ExampleSelect();

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
	};

}