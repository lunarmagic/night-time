
#include "sandboxpch.h"
#include "application/Application.h"
#include "entry.h"
#include "node/NodeRenderTarget.h"

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
	}

}


night::Application* night::create_application()
{
	return new night::Sandbox();
}