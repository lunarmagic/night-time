#pragma once

#include "NodeWindow.h"

#define NODE_GUI_TEMP_BEGIN(node_gui) node_gui->__begin()
#define NODE_GUI_TEMP_END(node_gui) node_gui->__end()

namespace night
{

	struct NIGHT_API NodeGui : public NodeWindow
	{
		NodeGui(NodeWindowParams const& params);

		void __begin();
		void __end();

		b8 button(string const& name);
		void checkbox(string const& name, b8* x);
		void drag_real(string const& name, real* r, real speed = 0.1f, real min = 0.0f, real max = 0.0f);
		void drag_s32(string const& name, s32* i, real speed = 0.1f, s32 min = 0, s32 max = 0);
		void drag_vec2(string const& name, vec2* v, real speed = 0.1f, real min = 0, real max = 0);
		void drag_vec3(string const& name, vec3* v, real speed = 0.1f, real min = 0, real max = 0);
		void drag_vec4(string const& name, vec4* v, real speed = 0.1f, real min = 0, real max = 0);
		void drag_ivec4(string const& name, ivec4* v, real speed = 0.1f, s32 min = 0, s32 max = 0);
		void input_text(string const& name, string& out_buffer, string const& hint = "");
		void tree_node(string const& name, b8 selected = false);
		void tree_pop();
		void begin_menu_bar();
		void end_menu_bar();
		b8 begin_menu(string const& name); // TODO: for some reason imgui returns false sometimes, switch to void for dist gui
		void end_menu();
		void menu_item(string const& name, b8 selected = false);
		void begin_list_box(string const& name);
		void end_list_box();
		void begin_canvas(string const& name, real height_ratio = 1.0f, Color const& fill = BLACK);
		void canvas_line(vec2 const& p1, vec2 const& p2, Color const& color = WHITE, real thickness = 1.0f);
		void canvas_circle(vec2 const& point, real const& radius, Color const& color = WHITE);
		void canvas_circle_filled(vec2 const& point, real const& radius, Color const& color = WHITE);
		vec2 canvas_mouse_position();
		void end_canvas();
		//void begin_child(string const& name, vec2 size);
		//void end_child();
		void begin_group();
		void end_group();
		void same_line();
		void seperator();
		void selectable(string const& label, b8 is_selected);
		b8 is_item_clicked(EMouse button = EMouse::Left);
		b8 is_mouse_down(EMouse button = EMouse::Left);
		b8 is_mouse_released(EMouse button = EMouse::Left);
		//real get_frame_height_with_spacing();
		//b8 set_next_window_size(vec2 const& size);
		//b8 set_next_window_position(vec2 const& position);
		void text(string const& text, Color const& color = COLOR_NULL);
		//void text_colored(string const& text, Color const& color);

	private:

		handle<NodeRenderTarget> _guiRenderTarget = nullptr;

	};

}