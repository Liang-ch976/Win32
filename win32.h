#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <string>
#include <vector>
#include <functional>
#include <cstring>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")

namespace win32 {

class Theme {
public:
	static void initialize() {
		INITCOMMONCONTROLSEX controls{sizeof(INITCOMMONCONTROLSEX),
									  ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_LINK_CLASS};
		InitCommonControlsEx(&controls);
	}

	static void apply(HWND handle) {
		if (!handle) return;
		SetWindowTheme(handle, L"Explorer", nullptr);
		SendMessageW(handle, WM_SETFONT, reinterpret_cast<WPARAM>(font()), TRUE);
	}

	static void round(HWND handle, int radius = 10) {
		if (!handle) return;
		RECT bounds{};
		GetWindowRect(handle, &bounds);
		SetWindowRgn(handle, CreateRoundRectRgn(0, 0, bounds.right - bounds.left,
			bounds.bottom - bounds.top, radius, radius), TRUE);
	}

	static void accent(HWND handle, COLORREF color = RGB(39, 125, 224)) {
		if (!handle) return;
		SendMessageW(handle, PBM_SETBARCOLOR, 0, color);
		SendMessageW(handle, PBM_SETSTATE, PBST_NORMAL, 0);
	}

	static HFONT font() {
		static HFONT value = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
										 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
										 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
		return value;
	}

	static HBRUSH background_brush() {
		static HBRUSH value = CreateSolidBrush(RGB(247, 249, 252));
		return value;
	}

	static bool glass_enabled() { return glass_enabled_; }

	static HBRUSH control_brush() {
		static HBRUSH value = CreateSolidBrush(RGB(255, 255, 255));
		return value;
	}

	static void enable_mica(HWND handle) {
		glass_enabled_ = false;
		HMODULE dwmapi = LoadLibraryW(L"dwmapi.dll");
		// Use the supported system backdrop without extending over the title bar.
		using DwmSetWindowAttributeFunction = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
		if (dwmapi) {
			auto set_attribute = reinterpret_cast<DwmSetWindowAttributeFunction>(
				GetProcAddress(dwmapi, "DwmSetWindowAttribute"));
			if (set_attribute) {
				const BOOL dark_mode = FALSE;
				const DWORD corner = 2;
				const DWORD backdrop = 2;
				const BOOL mica_effect = TRUE;
				set_attribute(handle, 20, &dark_mode, sizeof(dark_mode));
				set_attribute(handle, 33, &corner, sizeof(corner));
				set_attribute(handle, 1029, &mica_effect, sizeof(mica_effect));
				if (set_attribute(handle, 38, &backdrop, sizeof(backdrop)) == S_OK) {
					glass_enabled_ = true;
					return;
				}
			}
		}

	}

	static bool color_message(UINT message, WPARAM w_param, LRESULT& result) {
		if (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORLISTBOX) {
			HDC device = reinterpret_cast<HDC>(w_param);
			SetBkColor(device, RGB(255, 255, 255));
			SetTextColor(device, RGB(32, 35, 42));
			result = reinterpret_cast<LRESULT>(control_brush());
			return true;
		}
		if (message == WM_CTLCOLORSTATIC || message == WM_CTLCOLORBTN) {
			HDC device = reinterpret_cast<HDC>(w_param);
			SetBkMode(device, TRANSPARENT);
			SetTextColor(device, RGB(32, 35, 42));
			// Controls stay opaque; only the top-level window uses the glass backdrop.
			result = reinterpret_cast<LRESULT>(background_brush());
			return true;
		}
		return false;
	}

private:
	inline static bool glass_enabled_ = false;

	public:
	static void draw_button(const DRAWITEMSTRUCT* item) {
		RECT bounds = item->rcItem;
		const bool pressed = (item->itemState & ODS_SELECTED) != 0;
		const bool disabled = (item->itemState & ODS_DISABLED) != 0;
		const COLORREF color = disabled ? RGB(190, 198, 210) : (pressed ? RGB(22, 91, 170) : RGB(39, 125, 224));
		HBRUSH brush = CreateSolidBrush(color);
		HPEN pen = CreatePen(PS_SOLID, 1, color);
		HGDIOBJ old_brush = SelectObject(item->hDC, brush);
		HGDIOBJ old_pen = SelectObject(item->hDC, pen);
		RoundRect(item->hDC, bounds.left, bounds.top, bounds.right, bounds.bottom, 12, 12);
		SelectObject(item->hDC, old_brush);
		SelectObject(item->hDC, old_pen);
		DeleteObject(brush);
		DeleteObject(pen);
		SetBkMode(item->hDC, TRANSPARENT);
		SetTextColor(item->hDC, RGB(255, 255, 255));
		if ((item->itemState & ODS_FOCUS) != 0) {
			HPEN focus_pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
			HGDIOBJ old_pen = SelectObject(item->hDC, focus_pen);
			SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
			RoundRect(item->hDC, bounds.left + 2, bounds.top + 2, bounds.right - 2, bounds.bottom - 2, 10, 10);
			SelectObject(item->hDC, old_pen);
			DeleteObject(focus_pen);
		}
		wchar_t title[256]{};
		GetWindowTextW(item->hwndItem, title, 256);
		if (disabled) SetTextColor(item->hDC, RGB(235, 239, 245));
		DrawTextW(item->hDC, title, -1, &bounds, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
};

class Control {
public:
	Control() = default;
	explicit Control(HWND handle) : handle_(handle) {}

	HWND handle() const { return handle_; }
	bool valid() const { return handle_ != nullptr; }
	void show(bool visible = true) { ShowWindow(handle_, visible ? SW_SHOW : SW_HIDE); }
	void enable(bool enabled = true) { EnableWindow(handle_, enabled); }
	void focus() { SetFocus(handle_); }
	void invalidate(bool erase = true) { InvalidateRect(handle_, nullptr, erase); }
	void set_visible(bool visible) { show(visible); }
	void set_position(int x, int y) { SetWindowPos(handle_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER); }
	void set_size(int width, int height) { SetWindowPos(handle_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER); }
	void set_bounds(int x, int y, int width, int height) {
		SetWindowPos(handle_, nullptr, x, y, width, height, SWP_NOZORDER);
	}
	RECT bounds() const {
		RECT value{};
		GetWindowRect(handle_, &value);
		return value;
	}
	RECT client_bounds() const {
		RECT value{};
		GetClientRect(handle_, &value);
		return value;
	}
	void set_style(LONG_PTR style) { SetWindowLongPtrW(handle_, GWL_STYLE, style); }
	LONG_PTR style() const { return GetWindowLongPtrW(handle_, GWL_STYLE); }
	void set_user_data(LONG_PTR value) { SetWindowLongPtrW(handle_, GWLP_USERDATA, value); }
	LONG_PTR user_data() const { return GetWindowLongPtrW(handle_, GWLP_USERDATA); }
	void set_text(const std::wstring& text) { SetWindowTextW(handle_, text.c_str()); }
	void set_font(HFONT font = Theme::font()) {
		SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
	}
	std::wstring text() const {
		const int length = GetWindowTextLengthW(handle_);
		std::wstring value(static_cast<size_t>(length) + 1, L'\0');
		if (length > 0) GetWindowTextW(handle_, value.data(), length + 1);
		value.resize(static_cast<size_t>(length));
		return value;
	}

protected:
	HWND handle_ = nullptr;
};

class Window : public Control {
public:
	using Handler = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

	bool create(const std::wstring& title, int width = 640, int height = 400,
				DWORD style = WS_OVERLAPPEDWINDOW, DWORD ex_style = 0,
				HINSTANCE instance = GetModuleHandleW(nullptr)) {
		Theme::initialize();
		instance_ = instance;
		WNDCLASSEXW klass{sizeof(WNDCLASSEXW)};
		klass.lpfnWndProc = &Window::window_proc;
		klass.hInstance = instance_;
		klass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		klass.hbrBackground = nullptr;
		klass.lpszClassName = class_name();
		RegisterClassExW(&klass);
		handle_ = CreateWindowExW(ex_style, class_name(), title.c_str(), style,
								  CW_USEDEFAULT, CW_USEDEFAULT, width, height,
								  nullptr, nullptr, instance_, this);
		if (valid()) {
			SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(Theme::font()), TRUE);
			SetWindowTheme(handle_, L"Explorer", nullptr);
			Theme::enable_mica(handle_);
		}
		return valid();
	}

	void on_message(Handler handler) { handler_ = std::move(handler); }
	int run(int show_command = SW_SHOWNORMAL) {
		ShowWindow(handle_, show_command);
		UpdateWindow(handle_);
		MSG message{};
		while (GetMessageW(&message, nullptr, 0, 0) > 0) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
		return static_cast<int>(message.wParam);
	}
	static void quit(int code = 0) { PostQuitMessage(code); }
	void title(const std::wstring& value) { set_text(value); }
	std::wstring title() const { return text(); }
	void center() {
		RECT work{};
		SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
		RECT current = bounds();
		const int width = current.right - current.left;
		const int height = current.bottom - current.top;
		SetWindowPos(handle_, nullptr, (work.right - work.left - width) / 2 + work.left,
			(work.bottom - work.top - height) / 2 + work.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	void resize_client(int width, int height) {
		RECT frame{0, 0, width, height};
		AdjustWindowRectEx(&frame, static_cast<DWORD>(style()), FALSE, 0);
		set_size(frame.right - frame.left, frame.bottom - frame.top);
	}
	void minimize() { ShowWindow(handle_, SW_MINIMIZE); }
	void maximize() { ShowWindow(handle_, SW_MAXIMIZE); }
	void restore() { ShowWindow(handle_, SW_RESTORE); }
	void always_on_top(bool enabled = true) {
		SetWindowPos(handle_, enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

private:
	static LPCWSTR class_name() { return L"Win32Controls.Window"; }
	static LRESULT CALLBACK window_proc(HWND handle, UINT message,
										WPARAM w_param, LPARAM l_param) {
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
		if (message == WM_NCCREATE) {
			auto* create = reinterpret_cast<CREATESTRUCTW*>(l_param);
			window = static_cast<Window*>(create->lpCreateParams);
			window->handle_ = handle;
			SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		if (message == WM_ERASEBKGND) {
			if (Theme::glass_enabled()) return 1;
			RECT client{};
			GetClientRect(handle, &client);
			FillRect(reinterpret_cast<HDC>(w_param), &client, Theme::background_brush());
			return 1;
		}
		if (message == WM_DWMCOMPOSITIONCHANGED) {
			Theme::enable_mica(handle);
			InvalidateRect(handle, nullptr, TRUE);
		}
		LRESULT themed_result = 0;
		if (Theme::color_message(message, w_param, themed_result)) return themed_result;
		if (message == WM_DRAWITEM) {
			Theme::draw_button(reinterpret_cast<const DRAWITEMSTRUCT*>(l_param));
			return TRUE;
		}
		if (window && window->handler_) return window->handler_(handle, message, w_param, l_param);
		if (message == WM_NCDESTROY) SetWindowLongPtrW(handle, GWLP_USERDATA, 0);
		return DefWindowProcW(handle, message, w_param, l_param);
	}

	HINSTANCE instance_ = nullptr;
	Handler handler_;
};

class Button : public Control {
public:
	bool create(HWND parent, const std::wstring& title, int id) {
		return create(parent, title, id, 0, 0);
	}

	bool create(HWND parent, const std::wstring& title, int id, int x, int y,
				int width = 100, int height = 30, DWORD style = BS_PUSHBUTTON) {
		handle_ = CreateWindowW(L"BUTTON", title.c_str(), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | style,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
};

class Static : public Control {
public:
	bool create(HWND parent, const std::wstring& text) {
		return create(parent, text, 0, 0);
	}

	bool create(HWND parent, const std::wstring& text, int x, int y,
				int width = 180, int height = 24) {
		handle_ = CreateWindowW(L"STATIC", text.c_str(), WS_CHILD | WS_VISIBLE,
								x, y, width, height, parent, nullptr,
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
};
using Label = Static;

class Edit : public Control {
public:
	bool create(HWND parent, int id) {
		return create(parent, id, 0, 0);
	}

	bool create(HWND parent, int id, int x, int y, int width = 220, int height = 26,
				bool multiline = false) {
		DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
		if (multiline) style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL;
		handle_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", style,
								  x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								  GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
};
using TextBox = Edit;

class SearchBox : public Edit {
public:
	bool create(HWND parent, int id, const std::wstring& placeholder) {
		return create(parent, id, 0, 0, 260, 32, placeholder);
	}

	bool create(HWND parent, int id, int x, int y, int width = 260, int height = 32,
				const std::wstring& placeholder = L"搜索") {
		if (!Edit::create(parent, id, x, y, width, height, false)) return false;
		SendMessageW(handle_, EM_SETCUEBANNER, TRUE, reinterpret_cast<LPARAM>(placeholder.c_str()));
		return true;
	}
};

class Link : public Control {
public:
	bool create(HWND parent, const std::wstring& title, int id, int x, int y,
				int width = 180, int height = 24) {
		handle_ = CreateWindowW(L"SysLink", title.c_str(), WS_CHILD | WS_VISIBLE,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
			Theme::apply(handle_);
			return valid();
	}
};

class CheckBox : public Control {
public:
	bool create(HWND parent, const std::wstring& title, int id) {
		return create(parent, title, id, 0, 0);
	}

	bool create(HWND parent, const std::wstring& title, int id, int x, int y,
				int width = 160, int height = 26) {
		handle_ = CreateWindowW(L"BUTTON", title.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	bool checked() const { return SendMessageW(handle_, BM_GETCHECK, 0, 0) == BST_CHECKED; }
	void set_checked(bool value) { SendMessageW(handle_, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0); }
};

class ComboBox : public Control {
public:
	bool create(HWND parent, int id) {
		return create(parent, id, 0, 0);
	}

	bool create(HWND parent, int id, int x, int y, int width = 220, int height = 150) {
		handle_ = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	void add(const std::wstring& item) { SendMessageW(handle_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str())); }
	int selected() const { return static_cast<int>(SendMessageW(handle_, CB_GETCURSEL, 0, 0)); }
	void select(int index) { SendMessageW(handle_, CB_SETCURSEL, index, 0); }
};

class GroupBox : public Control {
public:
	bool create(HWND parent, const std::wstring& title, int x = 0, int y = 0,
				int width = 260, int height = 120) {
		handle_ = CreateWindowW(L"BUTTON", title.c_str(), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
								x, y, width, height, parent, nullptr,
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
};

class RadioButton : public Control {
public:
	bool create(HWND parent, const std::wstring& title, int id, int x = 0, int y = 0,
				int width = 160, int height = 26, bool auto_check = true) {
		const DWORD check_style = auto_check ? BS_AUTORADIOBUTTON : BS_RADIOBUTTON;
		handle_ = CreateWindowW(L"BUTTON", title.c_str(), WS_CHILD | WS_VISIBLE | check_style,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	bool checked() const { return SendMessageW(handle_, BM_GETCHECK, 0, 0) == BST_CHECKED; }
	void set_checked(bool value) { SendMessageW(handle_, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0); }
};

class ListBox : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 220, int height = 120) {
		handle_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
								  WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
								  x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								  GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
						Theme::round(handle_, 10);
		return valid();
	}
	int add(const std::wstring& item) { return static_cast<int>(SendMessageW(handle_, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()))); }
	int selected() const { return static_cast<int>(SendMessageW(handle_, LB_GETCURSEL, 0, 0)); }
	void select(int index) { SendMessageW(handle_, LB_SETCURSEL, index, 0); }
};

class ListView : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 320, int height = 180) {
		handle_ = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
								 WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
								 x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								 GetModuleHandleW(nullptr), nullptr);
			Theme::apply(handle_);
			Theme::round(handle_, 10);
			return valid();
	}
	void add_column(const std::wstring& title, int width, int index = 0) {
		LVCOLUMNW column{};
		column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		column.pszText = const_cast<LPWSTR>(title.c_str());
		column.cx = width;
		column.iSubItem = index;
		ListView_InsertColumn(handle_, index, &column);
	}
	int add_row(const std::wstring& value, int index = -1) {
		LVITEMW item{};
		item.mask = LVIF_TEXT;
		item.iItem = index < 0 ? ListView_GetItemCount(handle_) : index;
		item.pszText = const_cast<LPWSTR>(value.c_str());
		return ListView_InsertItem(handle_, &item);
	}
	void set_cell(int row, int column, const std::wstring& value) {
		ListView_SetItemText(handle_, row, column, const_cast<LPWSTR>(value.c_str()));
	}
};

class InfoBar : public Static {
public:
	bool create(HWND parent, const std::wstring& message) {
		return create(parent, message, 0, 0);
	}

	bool create(HWND parent, const std::wstring& message, int x, int y,
				int width = 420, int height = 32) {
		if (!Static::create(parent, message, x, y, width, height)) return false;
		SetWindowLongPtrW(handle_, GWL_STYLE, GetWindowLongPtrW(handle_, GWL_STYLE) | SS_CENTERIMAGE);
		return true;
	}
};

class ColorButton : public Button {
public:
	bool create(HWND parent, const std::wstring& title, int id, int x = 0, int y = 0,
				int width = 110, int height = 32) {
		return Button::create(parent, title, id, x, y, width, height);
	}
};

class ProgressBar : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 220, int height = 20) {
		handle_ = CreateWindowW(PROGRESS_CLASSW, L"", WS_CHILD | WS_VISIBLE,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
			Theme::accent(handle_);
		return valid();
	}
	void range(int minimum, int maximum) { SendMessageW(handle_, PBM_SETRANGE32, minimum, maximum); }
	void value(int amount) { SendMessageW(handle_, PBM_SETPOS, amount, 0); }
};

class Slider : public Control {
public:
	bool create(HWND parent, int id) {
		return create(parent, id, 0, 0);
	}

	bool create(HWND parent, int id, int x, int y, int width = 220, int height = 30) {
		handle_ = CreateWindowW(TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	void range(int minimum, int maximum) { SendMessageW(handle_, TBM_SETRANGE, TRUE, MAKELONG(minimum, maximum)); }
	int value() const { return static_cast<int>(SendMessageW(handle_, TBM_GETPOS, 0, 0)); }
	void value(int amount) { SendMessageW(handle_, TBM_SETPOS, TRUE, amount); }
};

class TabView : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 300, int height = 200) {
		handle_ = CreateWindowW(WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	int add(const std::wstring& title) {
		TCITEMW item{TCIF_TEXT, 0, 0, const_cast<LPWSTR>(title.c_str()), 0, 0, 0};
		return static_cast<int>(SendMessageW(handle_, TCM_INSERTITEMW, 999, reinterpret_cast<LPARAM>(&item)));
	}
	int selected() const { return static_cast<int>(SendMessageW(handle_, TCM_GETCURSEL, 0, 0)); }
};

class Separator : public Static {
public:
	bool create(HWND parent, int x = 0, int y = 0, int width = 300, int height = 1) {
		handle_ = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
								x, y, width, height, parent, nullptr,
								GetModuleHandleW(nullptr), nullptr);
		return valid();
	}
};

class DateTimePicker : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 150, int height = 26) {
		handle_ = CreateWindowW(DATETIMEPICK_CLASSW, L"", WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	bool get(SYSTEMTIME& value) const { return DateTime_GetSystemtime(handle_, &value) == GDT_VALID; }
	void set(const SYSTEMTIME& value) { DateTime_SetSystemtime(handle_, GDT_VALID, &value); }
};

class MonthCalendar : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 240, int height = 180) {
		handle_ = CreateWindowW(MONTHCAL_CLASSW, L"", WS_CHILD | WS_VISIBLE,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
};

class TreeView : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 240, int height = 180) {
		handle_ = CreateWindowExW(WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"",
								  WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
								  x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								  GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	HTREEITEM add_root(const std::wstring& title) { return add(title, TVI_ROOT); }
	HTREEITEM add(const std::wstring& title, HTREEITEM parent = TVI_ROOT) {
		TVINSERTSTRUCTW item{};
		item.hParent = parent;
		item.hInsertAfter = TVI_LAST;
		item.item.mask = TVIF_TEXT;
		item.item.pszText = const_cast<LPWSTR>(title.c_str());
		return reinterpret_cast<HTREEITEM>(SendMessageW(handle_, TVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item)));
	}
};

class RichEdit : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 300, int height = 160,
				bool read_only = false) {
		LoadLibraryW(L"Msftedit.dll");
		DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
		if (read_only) style |= ES_READONLY;
		handle_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"RICHEDIT50W", L"", style,
								  x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								  GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
};

class StatusBar : public Control {
public:
	bool create(HWND parent, int id = 1) {
		handle_ = CreateWindowW(STATUSCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
								0, 0, 0, 0, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	void text(const std::wstring& value, int part = 0) {
		SendMessageW(handle_, SB_SETTEXTW, part, reinterpret_cast<LPARAM>(value.c_str()));
	}
	void parts(const std::vector<int>& right_edges) {
		SendMessageW(handle_, SB_SETPARTS, static_cast<WPARAM>(right_edges.size()),
					 reinterpret_cast<LPARAM>(right_edges.data()));
	}
};

class UpDown : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 24, int height = 26) {
		handle_ = CreateWindowW(UPDOWN_CLASSW, L"", WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ARROWKEYS,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	void range(int minimum, int maximum) { SendMessageW(handle_, UDM_SETRANGE32, minimum, maximum); }
	int value() const { return static_cast<int>(SendMessageW(handle_, UDM_GETPOS32, 0, 0)); }
	void value(int amount) { SendMessageW(handle_, UDM_SETPOS32, 0, amount); }
};

class ToolBar : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 400, int height = 30) {
		handle_ = CreateWindowW(TOOLBARCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | CCS_TOP,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		if (valid()) {
			SendMessageW(handle_, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
			Theme::apply(handle_);
		}
		return valid();
	}
	void add_button(int id, BYTE state = TBSTATE_ENABLED, BYTE style = BTNS_BUTTON) {
		TBBUTTON button{};
		button.idCommand = id;
		button.fsState = state;
		button.fsStyle = style;
		SendMessageW(handle_, TB_ADDBUTTONS, 1, reinterpret_cast<LPARAM>(&button));
	}
	void autosize() { SendMessageW(handle_, TB_AUTOSIZE, 0, 0); }
};

class ToolTip : public Control {
public:
	bool create(HWND parent, const std::wstring& text, HWND target = nullptr) {
		handle_ = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
								 WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
								 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								 parent, nullptr, GetModuleHandleW(nullptr), nullptr);
			if (!valid()) return false;
			Theme::apply(handle_);
			TOOLINFOW info{sizeof(TOOLINFOW), TTF_IDISHWND | TTF_SUBCLASS};
			info.hwnd = parent;
			info.uId = reinterpret_cast<UINT_PTR>(target ? target : parent);
			info.lpszText = const_cast<LPWSTR>(text.c_str());
			SendMessageW(handle_, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&info));
			return true;
	}
};

class ScrollBar : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 220, int height = 18,
				bool vertical = false) {
			const DWORD direction = vertical ? SBS_VERT : SBS_HORZ;
			handle_ = CreateWindowW(L"SCROLLBAR", L"", WS_CHILD | WS_VISIBLE | direction,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
			Theme::apply(handle_);
			return valid();
		}
		void range(int minimum, int maximum, int page = 1) {
			SCROLLINFO info{sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE};
			info.nMin = minimum;
			info.nMax = maximum;
			info.nPage = page;
			SetScrollInfo(handle_, SB_CTL, &info, TRUE);
		}
		void value(int position) { SetScrollPos(handle_, SB_CTL, position, TRUE); }
		int value() const { return GetScrollPos(handle_, SB_CTL); }
};

class HotKeyBox : public Control {
public:
	bool create(HWND parent, int id, int x = 0, int y = 0, int width = 180, int height = 28) {
		handle_ = CreateWindowW(HOTKEY_CLASSW, L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
			Theme::apply(handle_);
			return valid();
		}
		void set_hot_key(WORD key, WORD modifiers) { SendMessageW(handle_, HKM_SETHOTKEY, MAKEWORD(key, modifiers), 0); }
};

class IPAddress : public Control {
public:
	bool create(HWND parent, int id, int x, int y, int width = 180, int height = 28) {
		handle_ = CreateWindowW(L"SysIPAddress32", L"", WS_CHILD | WS_VISIBLE,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		Theme::apply(handle_);
		return valid();
	}
	void set_address(BYTE first, BYTE second, BYTE third, BYTE fourth) {
		SendMessageW(handle_, IPM_SETADDRESS, 0, MAKEIPADDRESS(first, second, third, fourth));
	}
};

class FontDialog {
public:
	static bool show(HWND owner, LOGFONTW& font) {
		CHOOSEFONTW dialog{sizeof(CHOOSEFONTW)};
		dialog.hwndOwner = owner;
		dialog.lpLogFont = &font;
		dialog.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
		return ChooseFontW(&dialog) != FALSE;
	}
};

class ColorDialog {
public:
	static bool show(HWND owner, COLORREF& color) {
		static COLORREF custom_colors[16]{};
		CHOOSECOLORW dialog{sizeof(CHOOSECOLORW)};
		dialog.hwndOwner = owner;
		dialog.rgbResult = color;
		dialog.lpCustColors = custom_colors;
		dialog.Flags = CC_FULLOPEN | CC_RGBINIT;
		if (!ChooseColorW(&dialog)) return false;
		color = dialog.rgbResult;
		return true;
	}
};

struct Color {
	BYTE red = 0;
	BYTE green = 0;
	BYTE blue = 0;
	BYTE alpha = 255;
	COLORREF native() const { return RGB(red, green, blue); }
	static Color from_rgb(BYTE red, BYTE green, BYTE blue) { return {red, green, blue, 255}; }
};

struct Size {
	int width = 0;
	int height = 0;
};

struct Point {
	int x = 0;
	int y = 0;
};

struct Bounds {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	RECT rect() const { return {x, y, x + width, y + height}; }
};

class Layout {
public:
	class Row;

	class Column {
	public:
		Column(HWND parent, int margin = 24, int gap = 8, int width = 0)
			: parent_(parent), x_(margin), current_y_(margin), gap_(gap), width_(width) {
			if (width_ == 0) {
				RECT area{};
				GetClientRect(parent_, &area);
				width_ = area.right - area.left - margin * 2;
			}
		}

		Row row(int gap = 8);

		void add(Control& control, int height = 0) {
			if (!control.valid()) return;
			RECT current = control.bounds();
			const int item_width = width_ > 0 ? width_ : current.right - current.left;
			const int item_height = height > 0 ? height : current.bottom - current.top;
			control.set_bounds(x_, current_y_, item_width, item_height);
			current_y_ += item_height + gap_;
		}

	private:
		HWND parent_;
		int x_;
		int current_y_;
		int gap_;
		int width_;

		void advance(int height) { current_y_ += height + gap_; }
		friend class Row;
	};

	class Row {
	public:
		Row(Column& column, int gap = 8)
			: column_(&column), current_x_(column.x_), y_(column.current_y_), gap_(gap) {}

		Row(int x, int y, int gap = 8)
			: current_x_(x), y_(y), gap_(gap) {}

		void add(Control& control, int width = 0, int height = 0) {
			if (!control.valid()) return;
			RECT current = control.bounds();
			const int item_width = width > 0 ? width : current.right - current.left;
			const int item_height = height > 0 ? height : current.bottom - current.top;
			control.set_bounds(current_x_, y_, item_width, item_height);
			current_x_ += item_width + gap_;
			if (item_height > row_height_) row_height_ = item_height;
		}

		void finish() {
			if (column_) {
				column_->advance(row_height_);
				column_ = nullptr;
			}
		}

	private:
		Column* column_ = nullptr;
		int current_x_;
		int y_;
		int gap_;
		int row_height_ = 0;
	};

	static void fill(Control& control, HWND parent, int margin = 0) {
		RECT area{};
		GetClientRect(parent, &area);
		control.set_bounds(margin, margin,
			area.right - area.left - margin * 2,
			area.bottom - area.top - margin * 2);
	}

	static void center(Control& control, HWND parent) {
		RECT area{};
		GetClientRect(parent, &area);
		RECT current = control.bounds();
		const int width = current.right - current.left;
		const int height = current.bottom - current.top;
		control.set_position((area.right - width) / 2, (area.bottom - height) / 2);
	}

	static void right(Control& control, HWND parent, int margin = 0) {
		RECT area{};
		GetClientRect(parent, &area);
		RECT current = control.bounds();
		const int width = current.right - current.left;
		control.set_position(area.right - width - margin, current.top);
	}

	static void bottom(Control& control, HWND parent, int margin = 0) {
		RECT area{};
		GetClientRect(parent, &area);
		RECT current = control.bounds();
		const int height = current.bottom - current.top;
		control.set_position(current.left, area.bottom - height - margin);
	}

	static void stack_vertical(const std::vector<Control*>& controls, int x, int y,
							  int gap = 8, int width = 0, int height = 0) {
		int current_y = y;
		for (Control* control : controls) {
			if (!control || !control->valid()) continue;
			RECT current = control->bounds();
			const int item_width = width > 0 ? width : current.right - current.left;
			const int item_height = height > 0 ? height : current.bottom - current.top;
			control->set_bounds(x, current_y, item_width, item_height);
			current_y += item_height + gap;
		}
	}

	static void stack_horizontal(const std::vector<Control*>& controls, int x, int y,
								int gap = 8, int width = 0, int height = 0) {
		int current_x = x;
		for (Control* control : controls) {
			if (!control || !control->valid()) continue;
			RECT current = control->bounds();
			const int item_width = width > 0 ? width : current.right - current.left;
			const int item_height = height > 0 ? height : current.bottom - current.top;
			control->set_bounds(current_x, y, item_width, item_height);
			current_x += item_width + gap;
		}
	}
};

inline Layout::Row Layout::Column::row(int gap) { return Row(*this, gap); }

class Grid {
public:
	struct Row {
		int value;
	};

	struct Column {
		int value;
	};

	Grid(HWND parent, int column_count, int cell_width, int cell_height,
		 int margin = 24, int gap = 8)
		: margin_(margin), gap_(gap), cell_width_(cell_width), cell_height_(cell_height),
		  column_count_(column_count) {
		(void)parent;
	}

	void add(Control& control, Row row, Column column, int width = 0, int height = 0) {
		if (!control.valid()) return;
		const int item_width = width > 0 ? width : cell_width_;
		const int item_height = height > 0 ? height : cell_height_;
		const int x = margin_ + column.value * (cell_width_ + gap_);
		const int y = margin_ + row.value * (cell_height_ + gap_);
		control.set_bounds(x, y, item_width, item_height);
	}

private:
	int margin_;
	int gap_;
	int cell_width_;
	int cell_height_;
	int column_count_;
};

class Panel : public Control {
public:
	bool create(HWND parent, int x, int y, int width, int height,
				COLORREF color = RGB(255, 255, 255)) {
			color_ = color;
			handle_ = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
								x, y, width, height, parent, nullptr,
								GetModuleHandleW(nullptr), nullptr);
			if (valid()) {
				SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
				Theme::apply(handle_);
			}
			return valid();
		}
		void color(COLORREF value) { color_ = value; invalidate(); }
		COLORREF color() const { return color_; }
		static LRESULT CALLBACK procedure(HWND handle, UINT message, WPARAM w_param, LPARAM l_param) {
			Panel* panel = reinterpret_cast<Panel*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
			if (message == WM_ERASEBKGND) return 1;
			if (message == WM_PAINT && panel) {
				PAINTSTRUCT paint{};
				HDC device = BeginPaint(handle, &paint);
				HBRUSH brush = CreateSolidBrush(panel->color_);
				FillRect(device, &paint.rcPaint, brush);
				DeleteObject(brush);
				EndPaint(handle, &paint);
				return 0;
			}
			return DefWindowProcW(handle, message, w_param, l_param);
		}

private:
	COLORREF color_ = RGB(255, 255, 255);
};

class Card : public Panel {
public:
	bool create(HWND parent, int x, int y, int width, int height) {
		return Panel::create(parent, x, y, width, height, RGB(255, 255, 255));
	}
};

class ToggleSwitch : public Control {
public:
	bool create(HWND parent, const std::wstring& title, int id, int x, int y,
				int width = 180, int height = 28) {
			handle_ = CreateWindowW(L"BUTTON", title.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
								x, y, width, height, parent, reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
				Theme::apply(handle_);
				return valid();
		}
		bool on() const { return SendMessageW(handle_, BM_GETCHECK, 0, 0) == BST_CHECKED; }
		void on(bool value) { SendMessageW(handle_, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0); }
};

class MessageDialog {
public:
	static int show(HWND owner, const std::wstring& message, const std::wstring& title = L"提示",
				UINT type = MB_OK | MB_ICONINFORMATION) {
			return MessageBoxW(owner, message.c_str(), title.c_str(), type);
		}
	static bool confirm(HWND owner, const std::wstring& message,
					   const std::wstring& title = L"确认") {
			return show(owner, message, title, MB_YESNO | MB_ICONQUESTION) == IDYES;
		}
};

class Clipboard {
public:
	static bool set_text(HWND owner, const std::wstring& value) {
		if (!OpenClipboard(owner)) return false;
		EmptyClipboard();
		const size_t bytes = (value.size() + 1) * sizeof(wchar_t);
		HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
		if (!memory) {
			CloseClipboard();
			return false;
		}
		void* data = GlobalLock(memory);
		memcpy(data, value.c_str(), bytes);
		GlobalUnlock(memory);
		SetClipboardData(CF_UNICODETEXT, memory);
		CloseClipboard();
		return true;
	}

	static std::wstring get_text(HWND owner) {
		if (!OpenClipboard(owner)) return {};
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (!handle) {
			CloseClipboard();
			return {};
		}
		const wchar_t* data = static_cast<const wchar_t*>(GlobalLock(handle));
		std::wstring result = data ? data : L"";
		if (data) GlobalUnlock(handle);
		CloseClipboard();
		return result;
	}
};

class SystemMetrics {
public:
	static int screen_width() { return GetSystemMetrics(SM_CXSCREEN); }
	static int screen_height() { return GetSystemMetrics(SM_CYSCREEN); }
	static int caption_height() { return GetSystemMetrics(SM_CYCAPTION); }
	static int border_width() { return GetSystemMetrics(SM_CXFRAME); }
	static int scrollbar_width() { return GetSystemMetrics(SM_CXVSCROLL); }
	static int scrollbar_height() { return GetSystemMetrics(SM_CYHSCROLL); }
	static Size screen_size() { return {screen_width(), screen_height()}; }
};

class Cursor {
public:
	static void arrow() { SetCursor(LoadCursorW(nullptr, IDC_ARROW)); }
	static void hand() { SetCursor(LoadCursorW(nullptr, IDC_HAND)); }
	static void wait() { SetCursor(LoadCursorW(nullptr, IDC_WAIT)); }
	static void text() { SetCursor(LoadCursorW(nullptr, IDC_IBEAM)); }
	static void restore() { arrow(); }
};

class Keyboard {
public:
	static bool down(int virtual_key) {
		return (GetAsyncKeyState(virtual_key) & 0x8000) != 0;
	}
	static bool ctrl() { return down(VK_CONTROL); }
	static bool shift() { return down(VK_SHIFT); }
	static bool alt() { return down(VK_MENU); }
	static bool escape() { return down(VK_ESCAPE); }
};

class WindowIcon {
public:
	static void set(HWND window, HICON icon) {
		SendMessageW(window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
		SendMessageW(window, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
	}
	static void set_application(HWND window, LPCWSTR resource = IDI_APPLICATION) {
		set(window, LoadIconW(nullptr, resource));
	}
};

class WindowClass {
public:
	static void set_cursor(HWND window, HCURSOR cursor) {
		SetClassLongPtrW(window, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
	}
	static void set_background(HWND window, HBRUSH brush) {
		SetClassLongPtrW(window, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(brush));
	}
};

class FileDialog {
public:
	static std::wstring open(HWND owner, const std::wstring& filter = L"所有文件\0*.*\0\0") {
		wchar_t path[MAX_PATH]{};
		OPENFILENAMEW dialog{sizeof(OPENFILENAMEW)};
		dialog.hwndOwner = owner;
		dialog.lpstrFile = path;
		dialog.nMaxFile = MAX_PATH;
		dialog.lpstrFilter = filter.c_str();
		dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		return GetOpenFileNameW(&dialog) ? std::wstring(path) : std::wstring();
	}

	static std::wstring save(HWND owner, const std::wstring& filter = L"所有文件\0*.*\0\0") {
		wchar_t path[MAX_PATH]{};
		OPENFILENAMEW dialog{sizeof(OPENFILENAMEW)};
		dialog.hwndOwner = owner;
		dialog.lpstrFile = path;
		dialog.nMaxFile = MAX_PATH;
		dialog.lpstrFilter = filter.c_str();
		dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		return GetSaveFileNameW(&dialog) ? std::wstring(path) : std::wstring();
	}
};

class Timer {
public:
	using Callback = std::function<void()>;
	bool start(HWND owner, UINT_PTR id, UINT interval, Callback callback) {
		owner_ = owner;
		id_ = id;
		callback_ = std::move(callback);
		SetPropW(owner_, L"win32.timer", reinterpret_cast<HANDLE>(this));
		return SetTimer(owner_, id_, interval, &Timer::timer_proc) != 0;
	}
	void stop() {
		if (owner_) KillTimer(owner_, id_);
		if (owner_) RemovePropW(owner_, L"win32.timer");
		owner_ = nullptr;
		callback_ = nullptr;
	}

private:
	static void CALLBACK timer_proc(HWND owner, UINT, UINT_PTR id, DWORD) {
		Timer* timer = reinterpret_cast<Timer*>(GetPropW(owner, L"win32.timer"));
		if (timer && timer->id_ == id && timer->callback_) timer->callback_();
	}
	HWND owner_ = nullptr;
	UINT_PTR id_ = 0;
	Callback callback_;
};

} // namespace win32
