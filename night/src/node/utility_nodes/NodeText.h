#pragma once

#include "node/NodeRenderable.h"

namespace night
{

	// TODO: add controls for spacing, offset(centered, bottom left, top right)
	struct NodeTextParams
	{
		string text{ "" };
		string font{ "Font10x" }; // TODO: load default font from memory
		vec3 translation = vec3(0);
		vec3 scale = vec3(0.1f, 0.1f, 1.0f);
		quat rotation = quat(vec3(0));
		Color color{ LIGHT };
	};

	struct NIGHT_API NodeText : public NodeRenderable
	{
		NodeText(const NodeTextParams& params);

		void text(const string& text) { _text = text; }
		void color(const Color& color) { _color = color; }

	protected:

		virtual void on_render(RenderGraph& out_graph) const override;

	private:

		string _text;
		//ref<ITexture> _font;
		ref<ITexture> _font;
		Color _color;
		// TODO: alignment
	};

}