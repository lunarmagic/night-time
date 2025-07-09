#pragma warning(disable : 4251) // TODO: figure this out.
#pragma once

#include "color/Color.h"
#include "input/EInput.h"
//#include "stl/stl.h"
#include "math/math.h"

struct SDL_Window;
//struct SDL_Renderer;
struct SDL_GLContextState;
union SDL_Event;

namespace night
{
//	enum ImGuiWindowFlags_
//	{
//		ImGuiWindowFlags_None = 0,
//		ImGuiWindowFlags_NoTitleBar = 1 << 0,   // Disable title-bar
//		ImGuiWindowFlags_NoResize = 1 << 1,   // Disable user resizing with the lower-right grip
//		ImGuiWindowFlags_NoMove = 1 << 2,   // Disable user moving the window
//		ImGuiWindowFlags_NoScrollbar = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
//		ImGuiWindowFlags_NoScrollWithMouse = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
//		ImGuiWindowFlags_NoCollapse = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
//		ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,   // Resize every window to its content every frame
//		ImGuiWindowFlags_NoBackground = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
//		ImGuiWindowFlags_NoSavedSettings = 1 << 8,   // Never load/save settings in .ini file
//		ImGuiWindowFlags_NoMouseInputs = 1 << 9,   // Disable catching mouse, hovering test with pass through.
//		ImGuiWindowFlags_MenuBar = 1 << 10,  // Has a menu-bar
//		ImGuiWindowFlags_HorizontalScrollbar = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
//		ImGuiWindowFlags_NoFocusOnAppearing = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
//		ImGuiWindowFlags_NoBringToFrontOnFocus = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
//		ImGuiWindowFlags_AlwaysVerticalScrollbar = 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
//		ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1 << 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
//		ImGuiWindowFlags_NoNavInputs = 1 << 16,  // No keyboard/gamepad navigation within the window
//		ImGuiWindowFlags_NoNavFocus = 1 << 17,  // No focusing toward this window with keyboard/gamepad navigation (e.g. skipped by CTRL+TAB)
//		ImGuiWindowFlags_UnsavedDocument = 1 << 18,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
//		ImGuiWindowFlags_NoDocking = 1 << 19,  // Disable docking of this window
//		ImGuiWindowFlags_NoNav = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
//		ImGuiWindowFlags_NoDecoration = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
//		ImGuiWindowFlags_NoInputs = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
//
//		// [Internal]
//		ImGuiWindowFlags_ChildWindow = 1 << 24,  // Don't use! For internal use by BeginChild()
//		ImGuiWindowFlags_Tooltip = 1 << 25,  // Don't use! For internal use by BeginTooltip()
//		ImGuiWindowFlags_Popup = 1 << 26,  // Don't use! For internal use by BeginPopup()
//		ImGuiWindowFlags_Modal = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
//		ImGuiWindowFlags_ChildMenu = 1 << 28,  // Don't use! For internal use by BeginMenu()
//		ImGuiWindowFlags_DockNodeHost = 1 << 29,  // Don't use! For internal use by Begin()/NewFrame()
//
//		// Obsolete names
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//		ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 30,  // Obsoleted in 1.90.0: Use ImGuiChildFlags_AlwaysUseWindowPadding in BeginChild() call.
//		ImGuiWindowFlags_NavFlattened = 1 << 31,  // Obsoleted in 1.90.9: Use ImGuiChildFlags_NavFlattened in BeginChild() call.
//#endif
//	};
	enum struct EGuiWindow : u32
	{
		None = 0,
		NoTitleBar = BIT(0),
		NoResize = BIT(1),
		NoMove = BIT(2),
		NoCollapse = BIT(5),
		NoSavedSettings = BIT(8),
		NoMouseInputs = BIT(9),
		MenuBar = BIT(10),
		NoDocking = BIT(19),
	};

	inline constexpr EGuiWindow
		operator|(EGuiWindow x, EGuiWindow y)
	{
		return static_cast<EGuiWindow>
			(static_cast<s32>(x) | static_cast<s32>(y));
	}

	struct GuiPollEventResult
	{
		u8 wants_mouse_capture{ false };
		u8 wants_keyboard_capture{ false };
	};

	struct NIGHT_API IGui
	{
		virtual s32 init() { return 1; }
		virtual GuiPollEventResult poll_event(SDL_Event const& event) { return {}; } // TODO: fix
		virtual void update() {}
		virtual void close() {}

		virtual u8 begin(string const& name, u8* p_open = nullptr, EGuiWindow flags = EGuiWindow::None) { return false; };
		virtual u8 button(string const& name) { { return false; } };
		virtual u8 checkbox(string const& name, u8 const* x) { { return false; } };
		virtual u8 drag_r32(string const& name, r32 const* r, r32 speed = 1.0f, r32 min = 0.0f, r32 max = 0.0f) { { return false; } };
		virtual u8 drag_s32(string const& name, s32 const* i, r32 speed = 1.0f, s32 min = 0, s32 max = 0) { { return false; } };
		virtual u8 drag_ivec4(string const& name, ivec4 const* v, r32 speed = 1.0f, s32 min = 0, s32 max = 0) { { return false; } };
		virtual u8 input_text(string const& name, string& out_buffer, string const& hint = "") { { return false; } };
		virtual u8 tree_node(string const& name, u8 selected = false) { { return false; } };
		virtual void tree_pop() {};
		virtual u8 begin_menu_bar() { { return false; } };
		virtual void end_menu_bar() {};
		virtual u8 begin_menu(string const& name) { { return false; } };
		virtual void end_menu() {};
		virtual u8 menu_item(string const& name, u8 selected = false) { { return false; } };
		virtual u8 begin_list_box(string const& name) { { return false; } };
		virtual void end_list_box() {};
		virtual u8 begin_canvas(string const& name, real height_ratio = 1.0f, Color const& fill = BLACK) { { return false; } };
		virtual u8 canvas_line(vec2 const& p1, vec2 const& p2, Color const& color = WHITE, real thickness = 1.0f) { { return false; } };
		virtual u8 canvas_circle(vec2 const& point, real const& radius, Color const& color = WHITE) { { return false; } };
		virtual u8 canvas_circle_filled(vec2 const& point, real const& radius, Color const& color = WHITE) { { return false; } };
		virtual vec2 canvas_mouse_position() { { return vec2(0); } };
		virtual void end_canvas() {};
		virtual u8 begin_child(string const& name, vec2 size) { { return false; } };
		virtual void end_child() {};
		virtual void begin_group()  {};
		virtual void end_group()  {};
		virtual void same_line()  {};
		virtual void seperator()  {};
		virtual u8 selectable(string const& label, u8 is_selected)  { { return false; } };
		virtual u8 is_item_clicked(EMouse button = EMouse::Left)  { { return false; } };
		virtual u8 is_mouse_down(EMouse button = EMouse::Left)  { { return false; } };
		virtual u8 is_mouse_released(EMouse button = EMouse::Left)  { { return false; } };
		virtual real get_frame_height_with_spacing() { return 0; };
		virtual u8 set_next_window_size(vec2 const& size)  { { return false; } };
		virtual u8 set_next_window_position(vec2 const& position)  { { return false; } };
		virtual void text(string const& text)  {};
		virtual void text_colored(string const& text, Color const& color)  {};
		virtual void end()  {};
	};

	struct NIGHT_API Gui : public IGui
	{
		virtual s32 init() override;

		virtual GuiPollEventResult poll_event(SDL_Event const& event)  override;
		virtual void update()  override;
		virtual void close()  override;

		//void create_window(string id, GuiWindow const& window)  override;
		//void close_window(string id)  override;

		virtual u8 begin(string const& name, u8* p_open = nullptr, EGuiWindow flags = EGuiWindow::None)  override;
		virtual u8 button(string const& name)  override;
		virtual u8 checkbox(string const& name, u8 const* x)  override;
		virtual u8 drag_r32(string const& name, r32 const* r, r32 speed = 1.0f, r32 min = 0.0f, r32 max = 0.0f)  override;
		virtual u8 drag_s32(string const& name, s32 const* i, r32 speed = 1.0f, s32 min = 0, s32 max = 0)  override;
		virtual u8 drag_ivec4(string const& name, ivec4 const* v, r32 speed = 1.0f, s32 min = 0, s32 max = 0)  override;
		virtual u8 input_text(string const& name, string& out_buffer, string const& hint = "")  override;
		virtual u8 tree_node(string const& name, u8 selected = false)  override;
		virtual void tree_pop()  override;
		virtual u8 begin_menu_bar()  override;
		virtual void end_menu_bar()  override;
		virtual u8 begin_menu(string const& name)  override;
		virtual void end_menu()  override;
		virtual u8 menu_item(string const& name, u8 selected = false)  override;
		virtual u8 begin_list_box(string const& name)  override;
		virtual void end_list_box()  override;
		virtual u8 begin_canvas(string const& name, real height_ratio = 1.0f, Color const& fill = BLACK)  override;
		virtual u8 canvas_line(vec2 const& p1, vec2 const& p2, Color const& color = WHITE, real thickness = 1.0f)  override;
		virtual u8 canvas_circle(vec2 const& point, real const& radius, Color const& color = WHITE)  override;
		virtual u8 canvas_circle_filled(vec2 const& point, real const& radius, Color const& color = WHITE)  override;
		virtual vec2 canvas_mouse_position()  override;
		virtual void end_canvas()  override;
		virtual u8 begin_child(string const& name, vec2 size)  override;
		virtual void end_child()  override;
		virtual void begin_group()  override;
		virtual void end_group()  override;
		virtual void same_line()  override;
		virtual void seperator()  override;
		virtual u8 selectable(string const& label, u8 is_selected)  override;
		virtual u8 is_item_clicked(EMouse button = EMouse::Left)  override;
		virtual u8 is_mouse_down(EMouse button = EMouse::Left)  override;
		virtual u8 is_mouse_released(EMouse button = EMouse::Left)  override;
		virtual real get_frame_height_with_spacing()  override;
		virtual u8 set_next_window_size(vec2 const& size)  override;
		virtual u8 set_next_window_position(vec2 const& position)  override;
		virtual void text(string const& text)  override;
		virtual void text_colored(string const& text, Color const& color)  override;
		virtual void end()  override;

	private:

		SDL_Window* _window{ nullptr };
		//SDL_Renderer* _renderer;
		SDL_GLContextState* _context{ nullptr };
	};

}