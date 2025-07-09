
#include "sandboxpch.h"
#include "application/Application.h"
#include "entry.h"
#include "node/NodeRenderTarget.h"
#include "node/utility_nodes/NodeText.h"

namespace night
{
	struct Sandbox : public Application
	{
		Sandbox() = default;

	protected:

		virtual void on_initialized() override;
	};

	void Sandbox::on_initialized()
	{
		TRACE("Initialized Sandbox!");

		auto r = root();
		ASSERT(r != nullptr);

		auto nrt = r->create<NodeRenderTarget>("Sandbox Render Target", NodeRenderTargetParams{.clear_color = BLACK});
		ASSERT(nrt != nullptr);

		struct test_renderable : public NodeRenderable
		{
			test_renderable()
			{
				textures(utility::renderer().find_texture("Test"));
			}

			virtual void on_render(RenderGraph& out_graph) const override
			{
				out_graph.draw_quad(QuadParams{});
			}
		};

		auto nr = nrt->create<test_renderable>("Sandbox Renderable");
		ASSERT(nr != nullptr);


		struct test_text : public NodeText
		{
			test_text(NodeTextParams const& params) : NodeText(params) {}

		protected:

			virtual void on_update(real delta)
			{
				quat q = quat(vec3(utility::window().time_elapsed() * 0.5f, utility::window().time_elapsed(), utility::window().time_elapsed() * 0.33f));
				local_rotation(q);
			}
		};

		NodeTextParams ntp;
		ntp.text = "Hello.";
		ntp.font = "Font10x";
		ntp.translation.z = 20.0f; // move test text infront of test renderable
		auto tt = nrt->create<test_text>("Test Text", ntp);
	}
}


night::Application* night::create_application()
{
	return new night::Sandbox();
}