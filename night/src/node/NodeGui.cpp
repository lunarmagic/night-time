
#include "nightpch.h"
#include "NodeGui.h"

namespace night
{
	NodeGui::NodeGui(NodeWindowParams const& params)
		: NodeWindow(params)
	{
		NodeRenderTargetParams nrt_params;
		nrt_params.depth = 0;
		nrt_params.should_use_depth_peeling = false;
		nrt_params.should_use_depth_testing = true;
		nrt_params.should_use_blending = false;
		nrt_params.should_automatically_clear = false;
		nrt_params.should_automatically_render = false;
		nrt_params.clear_color = COLOR_ZERO;
		nrt_params.camera.type = ECameraType::Orthographic;
		nrt_params.camera.translation = FORWARD * (real)10;
		nrt_params.camera.look_at = ORIGIN;
		nrt_params.camera.up = UP;
		nrt_params.camera.ortho_region = AABB<>{ .left = -1, .top = 1, .right = 1, .bottom = -1 };
		nrt_params.camera.fov = NIGHT_CAMERA_DEFAULT_FOV;
		nrt_params.camera.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
		nrt_params.camera.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;

		_guiRenderTarget = create<NodeRenderTarget>("Gui Render Target", nrt_params);
	}

	void NodeGui::__begin()
	{
		auto& gui = utility::gui();

		AABB global_area;
		global_area.fit_around_quad(Quad<>(this->base_render_target_window_area()));
		vec2 gui_position = vec2{ global_area.left, global_area.top };
		vec2 gui_size = vec2{ global_area.width(), global_area.height() };

		gui.set_next_window_position(gui_position);
		gui.set_next_window_size(gui_size);

		if (on_close() != nullptr)
		{
			b8 is_open = true;
			gui.begin(name(), &is_open, EGuiWindow::MenuBar | EGuiWindow::NoResize | EGuiWindow::NoMove | EGuiWindow::NoCollapse | EGuiWindow::NoSavedSettings);

			if (!is_open)
			{
				on_close()();
			}
		}
		else
		{
			gui.begin(name(), nullptr, EGuiWindow::MenuBar | EGuiWindow::NoResize | EGuiWindow::NoMove | EGuiWindow::NoCollapse | EGuiWindow::NoSavedSettings);
		}
	}

	void NodeGui::__end()
	{
		utility::gui().end();
	}

	b8   NodeGui::button(string const& name) { return utility::gui().button(name); }
	void NodeGui::checkbox(string const& name, b8* x) { utility::gui().checkbox(name, x); }
	void NodeGui::drag_real(string const& name, real* r, real speed, real min, real max) { utility::gui().drag_real(name, r, speed, min, max); }
	void NodeGui::drag_s32(string const& name, s32* i, real speed, s32 min, s32 max) { utility::gui().drag_s32(name, i, speed, min, max); }
	void NodeGui::drag_vec2(string const& name, vec2* v, real speed, real min, real max) { utility::gui().drag_vec2(name, v, speed, min, max); }
	void NodeGui::drag_vec3(string const& name, vec3* v, real speed, real min, real max) { utility::gui().drag_vec3(name, v, speed, min, max); }
	void NodeGui::drag_vec4(string const& name, vec4* v, real speed, real min, real max) { utility::gui().drag_vec4(name, v, speed, min, max); }
	void NodeGui::drag_ivec4(string const& name, ivec4* v, real speed, s32 min, s32 max) { utility::gui().drag_ivec4(name, v, speed, min, max); }
	void NodeGui::input_text(string const& name, string& out_buffer, string const& hint) { utility::gui().input_text(name, out_buffer); }
	void NodeGui::tree_node(string const& name, b8 selected) { utility::gui().tree_node(name, selected); }
	void NodeGui::tree_pop() { utility::gui().tree_pop(); }
	void NodeGui::begin_menu_bar() { utility::gui().begin_menu_bar(); }
	void NodeGui::end_menu_bar() { utility::gui().end_menu_bar(); }
	b8 NodeGui::begin_menu(string const& name) { return utility::gui().begin_menu(name); }
	void NodeGui::end_menu() { utility::gui().end_menu(); }
	void NodeGui::menu_item(string const& name, b8 selected) { utility::gui().menu_item(name, selected); }
	void NodeGui::begin_list_box(string const& name) { utility::gui().begin_list_box(name); }
	void NodeGui::end_list_box() { utility::gui().end_list_box(); }
	void NodeGui::begin_canvas(string const& name, real height_ratio, Color const& fill) { utility::gui().begin_canvas(name, height_ratio, fill); }
	void NodeGui::canvas_line(vec2 const& p1, vec2 const& p2, Color const& color, real thickness) { utility::gui().canvas_line(p1, p2, color, thickness); }
	void NodeGui::canvas_circle(vec2 const& point, real const& radius, Color const& color) { utility::gui().canvas_circle(point, radius, color); }
	void NodeGui::canvas_circle_filled(vec2 const& point, real const& radius, Color const& color) { utility::gui().canvas_circle_filled(point, radius, color); }
	vec2 NodeGui::canvas_mouse_position() { return utility::gui().canvas_mouse_position(); }
	void NodeGui::end_canvas() { utility::gui().end_canvas(); }
	void NodeGui::begin_group() { utility::gui().begin_group(); }
	void NodeGui::end_group() { utility::gui().end_group(); }
	void NodeGui::same_line() { utility::gui().same_line(); }
	void NodeGui::seperator() { utility::gui().seperator(); }
	void NodeGui::selectable(string const& label, b8 is_selected) { utility::gui().selectable(label, is_selected); }
	b8   NodeGui::is_item_clicked(EMouse button) { return utility::gui().is_item_clicked(button); }

	b8 NodeGui::is_mouse_down(EMouse button)
	{
		return utility::gui().is_mouse_down(button);
	}

	b8 NodeGui::is_mouse_released(EMouse button)
	{
		return utility::gui().is_mouse_released(button);
	}
	
	void NodeGui::text(string const& text, Color const& color)
	{
		if (color == COLOR_NULL)
		{
			utility::gui().text(text);
		}
		else
		{
			utility::gui().text_colored(text, color);
		}
	}

	//void NodeGui::on_update(real delta)
	//{

	//}
}