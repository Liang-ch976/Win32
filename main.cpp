#include "win32.h"

namespace {
constexpr int kInput = 1001;
constexpr int kButton = 1002;
constexpr int kResult = 1003;
constexpr int kCheckBox = 1004;
constexpr int kCombo = 1005;
constexpr int kSlider = 1006;
constexpr int kSearch = 1007;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	win32::Window window;
	if (!window.create(L"数位判断", 520, 440)) return 1;

	win32::Static prompt;
	prompt.create(window.handle(), L"数位判断");
	prompt.set_font(CreateFontW(-24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
	                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI"));

	win32::Static hint;
	hint.create(window.handle(), L"输入内容并选择判断模式");

	win32::Edit input;
	input.create(window.handle(), kInput);

	win32::Button button;
	button.create(window.handle(), L"开始判断", kButton);

	win32::Static result;
	result.create(window.handle(), L"结果会显示在这里");

	win32::SearchBox search;
	search.create(window.handle(), kSearch, L"筛选结果");

	win32::CheckBox check_box;
	check_box.create(window.handle(), L"启用高级判断", kCheckBox);

	win32::ComboBox combo;
	combo.create(window.handle(), kCombo);
	combo.add(L"十进制");
	combo.add(L"十六进制");
	combo.select(0);

	win32::Static slider_label;
	slider_label.create(window.handle(), L"进度");

	win32::Slider slider;
	slider.create(window.handle(), kSlider);
	slider.range(0, 100);
	slider.value(60);

	win32::InfoBar info;
	info.create(window.handle(), L"提示：输入纯数字内容后点击“开始判断”");

	win32::Grid grid(window.handle(), 2, 300, 32, 28, 18);
	grid.add(prompt, win32::Grid::Row{0}, win32::Grid::Column{0}, 300, 32);
	grid.add(hint, win32::Grid::Row{1}, win32::Grid::Column{0}, 300, 24);
	grid.add(input, win32::Grid::Row{2}, win32::Grid::Column{0}, 300, 32);
	grid.add(button, win32::Grid::Row{2}, win32::Grid::Column{1}, 125, 32);
	grid.add(result, win32::Grid::Row{3}, win32::Grid::Column{0}, 300, 30);
	grid.add(search, win32::Grid::Row{3}, win32::Grid::Column{1}, 125, 30);
	grid.add(check_box, win32::Grid::Row{4}, win32::Grid::Column{0}, 180, 26);
	grid.add(combo, win32::Grid::Row{4}, win32::Grid::Column{1}, 170, 150);
	grid.add(slider_label, win32::Grid::Row{5}, win32::Grid::Column{0}, 50, 30);
	grid.add(slider, win32::Grid::Row{5}, win32::Grid::Column{1}, 250, 30);
	grid.add(info, win32::Grid::Row{6}, win32::Grid::Column{0}, 443, 32);

	window.on_message([&](HWND handle, UINT message, WPARAM w_param, LPARAM l_param) -> LRESULT {
		if (message == WM_COMMAND && LOWORD(w_param) == kButton && HIWORD(w_param) == BN_CLICKED) {
			const std::wstring value = input.text();
			bool all_digits = !value.empty();
			for (size_t index = 0; index < value.size(); ++index) {
				if (value[index] < L'0' || value[index] > L'9') all_digits = false;
			}
			result.set_text(all_digits ? L"这是只由数字组成的非空字符串" : L"请输入仅包含 0-9 的内容");
			return 0;
		}
		if (message == WM_HSCROLL && reinterpret_cast<HWND>(l_param) == slider.handle()) {
			result.set_text(L"滑块当前值：" + std::to_wstring(slider.value()));
			return 0;
		}
		if (message == WM_DESTROY) {
			win32::Window::quit();
			return 0;
		}
		return DefWindowProcW(handle, message, w_param, l_param);
	});

	return window.run();
}
