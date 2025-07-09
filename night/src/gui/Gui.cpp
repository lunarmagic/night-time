
#include "nightpch.h"
#include "Gui.h"
#include "GL/glew.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "utility.h"
#include "window/backends/sdl/WindowSDL.h"
#include "renderer/backends/opengl/RendererOpenGL.h"


namespace night
{
	s32 Gui::init()
	{
		auto& w = utility::window();
		auto& r = utility::renderer();
		auto dcw = dynamic_cast<WindowSDL*>(&w);
		auto dcr = dynamic_cast<RendererOpenGL*>(&r);

		if (dcw && dcr)
		{
			WindowSDL& window = (WindowSDL&)w;
			RendererOpenGL& renderer = (RendererOpenGL&)r;
			_context = renderer.context();

			_window = window.sdl_window();

			if (!_window || !_context)
			{
				ERROR("window or renderer are not initialized!, imgui failed to init.");
				return -1;
			}

			ImGui::CreateContext();
			auto io = ImGui::GetIO(); (void)(io);
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			ImGui::StyleColorsDark();

			ImGui_ImplSDL3_InitForOpenGL(window.sdl_window(), _context);
			const char* glsl_version = "#version 130";
			ImGui_ImplOpenGL3_Init(glsl_version);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();
		}
		else
		{
			TRACE("Window / Renderer backend not supported by imgui.");
			return -1;
		}


		return 1;
	}

	GuiPollEventResult Gui::poll_event(SDL_Event const& event)
	{
		GuiPollEventResult result;

		ImGui_ImplSDL3_ProcessEvent(&event);

		auto io = ImGui::GetIO(); (void)(io);

		result.wants_mouse_capture = io.WantCaptureMouse;
		result.wants_keyboard_capture = io.WantCaptureKeyboard;

		return result;
	}

	void Gui::update()
	{
		auto io = ImGui::GetIO(); (void)(io);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		//static bool show_demo_window = true;
		//ImGui::ShowDemoWindow(&show_demo_window);
	}

	void Gui::close()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	u8 Gui::begin(string const& name, u8* p_open, EGuiWindow flags)
	{
		return ImGui::Begin(name.c_str(), (bool*)p_open, (ImGuiWindowFlags_)flags); // TODO: make flags
	}

	u8 Gui::button(string const& name)
	{
		return ImGui::Button(name.c_str());
	}

	u8 Gui::checkbox(string const& name, u8 const* x)
	{
		return ImGui::Checkbox(name.c_str(), (bool*)const_cast<u8*>(x));
	}

	u8 Gui::drag_r32(string const& name, r32 const* r, r32 speed, r32 min, r32 max)
	{
		return ImGui::DragFloat(name.c_str(), const_cast<r32*>(r), speed, min, max);
	}

	u8 Gui::drag_s32(string const& name, s32 const* i, r32 speed, s32 min, s32 max)
	{
		return ImGui::DragInt(name.c_str(), const_cast<s32*>(i), speed, min, max);
	}

	u8 Gui::drag_ivec4(string const& name, ivec4 const* v, r32 speed, s32 min, s32 max)
	{
		return ImGui::DragInt4(name.c_str(), const_cast<int*>((int*)v), speed, min, max);
	}

	u8 Gui::input_text(string const& name, string& out_buffer, string const& hint)
	{
		if (hint.empty())
		{
			return ImGui::InputText(name.c_str(), const_cast<char*>(out_buffer.c_str()), out_buffer.size());
		}
		else
		{
			return ImGui::InputTextWithHint(name.c_str(), hint.c_str(), const_cast<char*>(out_buffer.c_str()), out_buffer.size());
		}
	}

	u8 Gui::tree_node(string const& name, u8 selected)
	{
		s32 flags = 0;
		flags |= ImGuiTreeNodeFlags_DefaultOpen;
		flags |= ImGuiTreeNodeFlags_Leaf;
		flags |= selected ? ImGuiTreeNodeFlags_Selected : 0;
		//flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		return ImGui::TreeNodeEx(name.c_str(), flags);
	}

	void Gui::tree_pop()
	{
		ImGui::TreePop();
	}

	u8 Gui::begin_menu_bar()
	{
		return ImGui::BeginMenuBar();
	}

	void Gui::end_menu_bar()
	{
		ImGui::EndMenuBar();
	}

	u8 Gui::begin_menu(string const& name)
	{
		return ImGui::BeginMenu(name.c_str());
	}

	void Gui::end_menu()
	{
		ImGui::EndMenu();
	}

	u8 Gui::menu_item(string const& name, u8 selected)
	{
		return ImGui::MenuItem(name.c_str(), NULL, false);
	}

	u8 Gui::begin_list_box(string const& name)
	{
		return ImGui::BeginListBox(name.c_str());
	}

	void Gui::end_list_box()
	{
		ImGui::EndListBox();
	}

	static ImDrawList* draw_list;
	static ImVec2 canvas_p0;
	ImVec2 canvas_sz;
	static ImVec2 canvas_p1;

	u8 Gui::begin_canvas(string const& name, real height_ratio, Color const& fill)
	{
		canvas_p0 = ImGui::GetCursorScreenPos();
		canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
		//if (canvas_sz.x > max_size.x) canvas_sz.x = max_size.x;
		//if (canvas_sz.y > max_size.y) canvas_sz.y = max_size.y;
		canvas_sz.y = canvas_sz.x / height_ratio;
		canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

		ImGui::InvisibleButton("Canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

		Color8 fill8 = Color8(fill);

		//ImGuiIO& io = ImGui::GetIO();
		draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(fill8.r, fill8.g, fill8.b, fill8.a));
		draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
		draw_list->PushClipRect(canvas_p0, canvas_p1, true);

		return true;
	}

	u8 Gui::canvas_line(vec2 const& p1, vec2 const& p2, Color const& color, real thickness)
	{
		vec2 p1p;
		p1p.x = (p1.x + 1.0f) * canvas_sz.x / 2.0f;
		p1p.y = (p1.y + 1.0f) * canvas_sz.y / 2.0f;

		vec2 p2p;
		p2p.x = (p2.x + 1.0f) * canvas_sz.x / 2.0f;
		p2p.y = (p2.y + 1.0f) * canvas_sz.y / 2.0f;

		Color8 color8 = Color8(color);

		draw_list->AddLine(
			ImVec2(canvas_p0.x + p1p.x, canvas_p0.y + p1p.y),
			ImVec2(canvas_p0.x + p2p.x, canvas_p0.y + p2p.y),
			IM_COL32(color8.r, color8.g, color8.b, color8.a),
			thickness
		);

		return true;
	}

	u8 Gui::canvas_circle(vec2 const& point, real const& radius, Color const& color)
	{
		vec2 pp;
		pp.x = (point.x + 1.0f) * canvas_sz.x / 2.0f;
		pp.y = (point.y + 1.0f) * canvas_sz.y / 2.0f;

		Color8 color8 = Color8(color);

		draw_list->AddCircle(
			ImVec2(canvas_p0.x + pp.x, canvas_p0.y + pp.y),
			radius,
			IM_COL32(color8.r, color8.g, color8.b, color8.a)
		);

		return true;
	}

	u8 Gui::canvas_circle_filled(vec2 const& point, real const& radius, Color const& color)
	{
		vec2 pp;
		pp.x = (point.x + 1.0f) * canvas_sz.x / 2.0f;
		pp.y = (point.y + 1.0f) * canvas_sz.y / 2.0f;

		Color8 color8 = Color8(color);

		draw_list->AddCircleFilled(
			ImVec2(canvas_p0.x + pp.x, canvas_p0.y + pp.y),
			radius,
			IM_COL32(color8.r, color8.g, color8.b, color8.a)
		);

		return true;
	}

	vec2 Gui::canvas_mouse_position()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImVec2 mouse = io.MousePos;
		mouse.x = ((mouse.x - canvas_p0.x) / canvas_sz.x - 0.5f) * 2;
		mouse.y = ((mouse.y - canvas_p0.y) / canvas_sz.y - 0.5f) * 2;

		return { (real)mouse.x, (real)mouse.y };
	}

	void Gui::end_canvas()
	{
		draw_list->PopClipRect();
		ImGui::Dummy(ImVec2(0.0f, canvas_sz.y));
		ImGui::Spacing();
	}

	u8 Gui::begin_child(string const& name, vec2 size)
	{
		return ImGui::BeginChild(name.c_str(), {size.x, size.y}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
	}

	void Gui::end_child()
	{
		ImGui::EndChild();
	}

	void Gui::begin_group()
	{
		ImGui::BeginGroup();
	}

	void Gui::end_group()
	{
		ImGui::EndGroup();
	}

	void Gui::same_line()
	{
		ImGui::SameLine();
	}

	void Gui::seperator()
	{
		ImGui::Separator();
	}

	u8 Gui::selectable(string const& label, u8 is_selected)
	{
		return ImGui::Selectable(label.c_str(), is_selected);
	}

	u8 Gui::is_item_clicked(EMouse button)
	{
		ImGuiMouseButton_ b;

		switch (button)
		{
		case EMouse::Left:
		{
			b = ImGuiMouseButton_Left;
			break;
		}
		case EMouse::Right:
		{
			b = ImGuiMouseButton_Right;
			break;
		}
		case EMouse::Middle:
		{
			b = ImGuiMouseButton_Middle;
			break;
		}
		}

		return ImGui::IsItemClicked(b);
	}

	u8 Gui::is_mouse_down(EMouse button)
	{
		ImGuiMouseButton_ b;

		switch (button)
		{
		case EMouse::Left:
		{
			b = ImGuiMouseButton_Left;
			break;
		}
		case EMouse::Right:
		{
			b = ImGuiMouseButton_Right;
			break;
		}
		case EMouse::Middle:
		{
			b = ImGuiMouseButton_Middle;
			break;
		}
		}

		return ImGui::IsMouseDown(b);
	}

	u8 Gui::is_mouse_released(EMouse button)
	{
		ImGuiMouseButton_ b;

		switch (button)
		{
		case EMouse::Left:
		{
			b = ImGuiMouseButton_Left;
			break;
		}
		case EMouse::Right:
		{
			b = ImGuiMouseButton_Right;
			break;
		}
		case EMouse::Middle:
		{
			b = ImGuiMouseButton_Middle;
			break;
		}
		}

		return ImGui::IsMouseReleased(b);
	}

	real Gui::get_frame_height_with_spacing()
	{
		return ImGui::GetFrameHeightWithSpacing();
	}

	u8 Gui::set_next_window_size(vec2 const& size)
	{
		ImGui::SetNextWindowSize({ size.x, size.y });
		return true;
	}

	u8 Gui::set_next_window_position(vec2 const& position)
	{
		ImGui::SetNextWindowPos({ position.x, position.y });
		return true;
	}

	void Gui::text(string const& text)
	{
		ImGui::Text(text.c_str());
	}

	void Gui::text_colored(string const& text, Color const& color)
	{
		ImVec4 c;
		c.x = color.r;
		c.y = color.g;
		c.z = color.b;
		c.w = color.a;
		ImGui::TextColored(c, text.c_str());
	}

	void Gui::end()
	{
		ImGui::End();
	}
}