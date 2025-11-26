#pragma once

#if 0
#include "node/INode.h"
#include "TestNodeAnimation.h"
#include "TestConvex2D.h"

#define NIGHT_TEST_LAUNCHER_CLOSED_SIGNAL "Night Test Launcher Closed"

namespace night
{
	struct TestLauncher : public INode
	{

	protected:

		virtual void on_update(real delta) override
		{
			if (_activeTest == nullptr)
			{
				IGui& gui = utility::gui();
				b8 is_open = true;
				gui.begin("Select Test", &is_open);

				if (!is_open)
				{
					emit_signal(NIGHT_TEST_LAUNCHER_CLOSED_SIGNAL);
				}
				else
				{
					NodeWindowParams nwp = {};

					if (gui.button("Node Animation"))
					{
						_activeTest = create<TestNodeAnimation>("Test Node Animation", nwp);
					}

					else if (gui.button("Convex 2D"))
					{
						_activeTest = create<TestConvex2D>("Test Node Convex 2D", nwp);
					}
				}
				
				gui.end();
			}
		}

	private:

		handle<INode> _activeTest = nullptr;
	};
}
#endif