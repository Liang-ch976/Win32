# Win32 控件库

这是一个基于原生 Win32 API 的轻量 C++ 控件封装。所有库实现已经合并到 `win32.h`，不需要安装第三方库，也不需要 XAML 或 Windows App SDK。

它提供统一的 `create`、`text`、`set_text`、`enable`、`show` 等操作，并使用 Segoe UI、浅色背景和新版 Common Controls，视觉上接近简化版 WinUI 3。

## 文件说明

| 文件 | 作用 |
| --- | --- |
| `win32.h` | 全部控件和窗口封装，项目只需要包含它 |
| `main.cpp` | 可直接运行的数位判断示例 |
| `app.manifest` | 请求 Common Controls 6.0 视觉样式 |
| `resource.rc` | 将 manifest 编译进 exe |

## 环境要求

- Windows 10 或 Windows 11
- C++17 编译器
- MinGW-w64 或 Visual Studio C++
- MinGW 编译时需要 `windres.exe`

## MinGW 编译

在当前目录打开 PowerShell 或终端，依次执行：

```bat
windres resource.rc -O coff -o resource.o
g++ -std=c++17 -municode main.cpp resource.o -o win32_controls.exe -luser32 -lgdi32 -lcomctl32 -luxtheme -lcomdlg32 -lshell32
```

如果使用 MSYS2，请在 **MinGW 64-bit** 终端中执行以上命令，并确保 `g++` 和 `windres` 已加入环境变量。

运行：

```bat
.\win32_controls.exe
```

`resource.o` 只需要在 `app.manifest` 或 `resource.rc` 发生变化后重新生成；修改 C++ 代码时直接重新执行第二条命令即可。

清理构建产物：

```powershell
Remove-Item resource.o, win32_controls.exe -ErrorAction SilentlyContinue
```

## Visual Studio 编译

打开“x64 Native Tools Command Prompt for VS”，先编译资源文件，再编译 C++ 源码：

```bat
rc resource.rc
cl /std:c++17 /EHsc main.cpp resource.res user32.lib gdi32.lib comctl32.lib uxtheme.lib comdlg32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /OUT:win32_controls.exe
```

运行：

```bat
win32_controls.exe
```

修改 `app.manifest` 或 `resource.rc` 后，需要重新执行 `rc resource.rc`；只修改 C++ 代码时，直接重新执行 `cl` 命令即可。将 `resource.rc` 加入 Visual Studio 工程也可以达到相同效果。

## 最小示例

```cpp
#include "win32.h"

constexpr int kButton = 1001;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	win32::Window window;
	if (!window.create(L"我的窗口", 480, 260)) return 1;

	win32::Label label;
	label.create(window.handle(), L"请输入内容：");

	win32::TextBox input;
	input.create(window.handle(), 1002);

	win32::Button button;
	button.create(window.handle(), L"确定", kButton);

	win32::Grid grid(window.handle(), 2, 280, 32, 24, 12);
	grid.add(label, win32::Grid::Row{0}, win32::Grid::Column{0});
	grid.add(input, win32::Grid::Row{1}, win32::Grid::Column{0});
	grid.add(button, win32::Grid::Row{1}, win32::Grid::Column{1}, 110, 32);

	window.on_message([&](HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) -> LRESULT {
		if (message == WM_COMMAND && LOWORD(w_param) == kButton && HIWORD(w_param) == BN_CLICKED) {
			label.set_text(input.text());
			return 0;
		}
		if (message == WM_DESTROY) {
			win32::Window::quit();
			return 0;
		}
		return DefWindowProcW(hwnd, message, w_param, l_param);
	});

	return window.run();
}
```

## 基本使用流程

1. 包含 `win32.h`。
2. 创建 `win32::Window`，调用 `create` 创建窗口。
3. 创建控件，并把 `window.handle()` 作为父窗口句柄传入；布局示例统一使用 `Grid::Row` 和 `Grid::Column`。
4. 为按钮、复选框、下拉框等控件分配不重复的整数 ID。
5. 使用 `window.on_message` 处理控件通知消息。
6. 最后调用 `window.run()` 启动消息循环。

控件的坐标以父窗口客户区左上角为原点，单位是像素，格式通常为 `x, y, width, height`。

## 控件 API

### 基础控件

| 控件 | 创建方式 | 常用方法 |
| --- | --- | --- |
| `Label` / `Static` | `create(parent, text, x, y, width, height)` | `set_text`、`text` |
| `TextBox` / `Edit` | `create(parent, id, x, y, width, height, multiline)` | `text`、`set_text` |
| `Button` | `create(parent, text, id, x, y, width, height)` | `set_text`、`enable` |
| `CheckBox` | `create(parent, text, id, x, y, width, height)` | `checked`、`set_checked` |
| `RadioButton` | `create(parent, text, id, x, y, width, height)` | `checked`、`set_checked` |
| `ComboBox` | `create(parent, id, x, y, width, height)` | `add`、`select`、`selected` |
| `ListBox` | `create(parent, id, x, y, width, height)` | `add`、`select`、`selected` |
| `SearchBox` | `create(parent, id, x, y, width, height, placeholder)` | `text`、`set_text` |
| `Link` | `create(parent, text, id, x, y, width, height)` | `set_text` |
| `ListView` | `create(parent, id, x, y, width, height)` | `add_column`、`add_row`、`set_cell` |

示例：

```cpp
win32::ComboBox mode;
mode.create(window.handle(), 2001);
mode.add(L"十进制");
mode.add(L"十六进制");
mode.select(0);

win32::CheckBox option;
option.create(window.handle(), L"启用高级模式", 2002);
option.set_checked(true);

win32::Grid grid(window.handle(), 2, 180, 32, 24, 12);
grid.add(mode, win32::Grid::Row{0}, win32::Grid::Column{0}, 180, 140);
grid.add(option, win32::Grid::Row{1}, win32::Grid::Column{0}, 180, 26);
```

### 数值和状态控件

```cpp
win32::Slider slider;
slider.create(window.handle(), 3001);
slider.range(0, 100);
slider.value(60);

win32::ProgressBar progress;
progress.create(window.handle(), 3002);
progress.range(0, 100);
progress.value(60);

win32::Grid grid(window.handle(), 1, 240, 30, 24, 8);
grid.add(slider, win32::Grid::Row{0}, win32::Grid::Column{0}, 240, 30);
grid.add(progress, win32::Grid::Row{1}, win32::Grid::Column{0}, 240, 20);
```

拖动 `Slider` 时，窗口会收到 `WM_HSCROLL`；可以通过 `slider.value()` 读取当前值。

### 扩展控件

`win32.h` 还包含以下控件：

- `GroupBox`：控件分组边框
- `TabView`：选项卡
- `Separator`：水平分隔线
- `TreeView`：树形目录
- `RichEdit`：多行富文本输入框
- `DateTimePicker`：日期选择器
- `MonthCalendar`：月历
- `StatusBar`：窗口底部状态栏
- `ToolBar`：工具栏
- `UpDown`：数值微调按钮
- `InfoBar`：提示信息条
- `ColorButton`：强调色按钮
- `ToolTip`：鼠标悬停提示
- `ScrollBar`：水平或垂直滚动条
- `HotKeyBox`：快捷键输入框
- `IPAddress`：IPv4 地址输入框
- `FontDialog`：系统字体选择对话框
- `ColorDialog`：系统颜色选择对话框

树形控件示例：

```cpp
win32::TreeView tree;
tree.create(window.handle(), 4001);
HTREEITEM root = tree.add_root(L"项目");
tree.add(L"源代码", root);
tree.add(L"资源文件", root);

win32::Grid grid(window.handle(), 1, 180, 150, 24, 8);
grid.add(tree, win32::Grid::Row{0}, win32::Grid::Column{0}, 180, 150);
```

状态栏示例：

```cpp
win32::StatusBar status;
status.create(window.handle());
status.text(L"就绪");
```

## 消息处理

控件通知统一在窗口消息回调中处理：

```cpp
window.on_message([&](HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) -> LRESULT {
	if (message == WM_COMMAND) {
		const int id = LOWORD(w_param);
		const int notify_code = HIWORD(w_param);
		if (id == 2002 && notify_code == BN_CLICKED) {
			// 复选框状态变化
			return 0;
		}
	}
	if (message == WM_DESTROY) {
		win32::Window::quit();
		return 0;
	}
	return DefWindowProcW(hwnd, message, w_param, l_param);
});
```

常用通知：

- 按钮：`WM_COMMAND` + `BN_CLICKED`
- 复选框：`WM_COMMAND` + `BN_CLICKED`
- 下拉框选择：`WM_COMMAND` + `CBN_SELCHANGE`
- 列表框选择：`WM_COMMAND` + `LBN_SELCHANGE`
- 滑块变化：`WM_HSCROLL`
- 树形控件变化：`WM_NOTIFY`

## 美化说明

库会自动：

- 初始化 Common Controls
- 使用 Segoe UI 字体
- 使用浅灰色窗口背景和白色输入区域
- 使用 Explorer 视觉主题
- 将主按钮绘制为蓝色圆角按钮
- 在 Windows 11 上启用顶层窗口 DWM Mica 背景，控件保持清晰

`app.manifest` 很重要。如果没有它，Windows 可能使用旧版 Common Controls，按钮、下拉框、进度条等控件会呈现类似 Windows 95 的经典外观。

需要注意：这是 Win32 控件库，不是真正的 WinUI 3。它可以获得相近的颜色、字体和布局观感，但不提供 XAML、数据绑定、自动布局和 WinUI 3 控件模板。

### 毛玻璃效果

`Window::create` 创建窗口后会自动调用 `Theme::enable_mica`。Mica 只设置在顶层窗口，控件不会使用 Mica。程序不扩展客户区，因此会保留标准标题栏、最小化、最大化和关闭按钮：

- Windows 11：使用 DWM Mica 系统背景，并兼容旧版 Windows 11 的 Mica 属性
- Windows 10 或不支持 Mica 的系统：保持普通浅色背景，不影响程序运行
- Mica 只应用于窗口背景；输入框、按钮和其他子控件保持不透明和清晰
- 不需要修改 `main.cpp`，也不需要额外的第三方 DLL

如果希望关闭毛玻璃，可以在 `win32.h` 的 `Window::create` 中删除或注释这一行：

```cpp
Theme::enable_mica(handle_);
```

## 常见问题

### 编译时提示找不到 `windres`

说明当前 MinGW 环境没有加入 PATH。可以使用 MinGW-w64 的 `bin` 目录，或者改用 Visual Studio 编译。

### 程序仍然像 Windows 95

请确认编译时链接了 `resource.o`，并且 `resource.o` 是由最新的 `resource.rc` 生成的：

```bat
windres resource.rc -O coff -o resource.o
```

### 控件创建失败

检查父窗口是否有效、控件 ID 是否重复，以及 `x`、`y`、`width`、`height` 是否在窗口客户区范围内。日期、树形、工具栏等控件依赖 Common Controls 初始化，而 `Window::create` 会自动完成初始化。

## 完整控件清单

所有类型都位于 `win32` 命名空间中，统一包含方式如下：

```cpp
#include "win32.h"
```

### 窗口和基础控件

| 类型 | 说明 |
| --- | --- |
| `Window` | 顶层窗口、消息循环、Mica 背景 |
| `Control` | 所有控件的基类 |
| `Static` / `Label` | 静态文本 |
| `Edit` / `TextBox` | 单行或多行文本输入 |
| `Button` | 蓝色圆角主按钮 |
| `CheckBox` | 复选框 |
| `RadioButton` | 单选按钮 |
| `ComboBox` | 下拉选择框 |
| `SearchBox` | 带占位文字的搜索框 |
| `Link` | 系统超链接控件 |

### 数据和导航控件

| 类型 | 说明 |
| --- | --- |
| `ListBox` | 简单列表 |
| `ListView` | 多列列表或表格 |
| `TreeView` | 树形目录 |
| `TabView` | 选项卡 |
| `ProgressBar` | 进度显示 |
| `Slider` | 滑块输入 |
| `ScrollBar` | 滚动条 |
| `UpDown` | 数值微调 |
| `ToolBar` | 工具栏 |
| `StatusBar` | 状态栏 |
| `Separator` | 分隔线 |
| `InfoBar` | 信息提示条 |
| `GroupBox` | 分组容器 |
| `Panel` / `Card` | 面板和卡片容器 |
| `ToggleSwitch` | 开关语义的复选控件 |

### 系统和输入控件

| 类型 | 说明 |
| --- | --- |
| `DateTimePicker` | 日期选择 |
| `MonthCalendar` | 月历 |
| `RichEdit` | 富文本输入 |
| `HotKeyBox` | 快捷键输入 |
| `IPAddress` | IPv4 地址输入 |
| `ToolTip` | 鼠标悬停提示 |
| `ColorButton` | 颜色操作按钮 |

## Control 通用 API

所有继承自 `Control` 的对象都可以使用以下方法：

```cpp
control.show();                 // 显示
control.show(false);            // 隐藏
control.enable(false);          // 禁用
control.enable();               // 启用
control.focus();                // 获取键盘焦点
control.invalidate();           // 请求重绘
control.set_text(L"新文本");     // 设置文字
std::wstring value = control.text();
control.set_font(win32::Theme::font());
```

位置和尺寸操作：

```cpp
control.set_position(20, 40);
control.set_size(300, 32);
control.set_bounds(20, 40, 300, 32);
RECT screen_rect = control.bounds();
RECT client_rect = control.client_bounds();
```

控件的 `create` 接口仍保留坐标参数以兼容 Win32，但使用时可以传入 `0, 0`，再交给 `Grid` 统一布局。`bounds()` 返回屏幕坐标。

## Window API

```cpp
win32::Window window;
window.create(L"应用程序", 800, 520);
window.title(L"新标题");
window.center();
window.resize_client(760, 480);
window.always_on_top(true);
window.minimize();
window.maximize();
window.restore();
```

通常只需要在最后调用一次：

```cpp
return window.run();
```

关闭窗口时必须退出消息循环：

```cpp
if (message == WM_DESTROY) {
	win32::Window::quit();
	return 0;
}
```

## Layout 布局工具

`Layout` 不依赖 XAML，也不提供自动响应式布局，但可以减少手动计算坐标：

```cpp
win32::Grid grid(window.handle(), 2, 180, 32, 24, 12);
grid.add(label, win32::Grid::Row{0}, win32::Grid::Column{0});
grid.add(input, win32::Grid::Row{1}, win32::Grid::Column{0});
grid.add(button, win32::Grid::Row{1}, win32::Grid::Column{1});
```

控件创建时先使用 `0, 0`，再通过 `Grid::Row` 和 `Grid::Column` 指定网格位置：

```cpp
title.create(window.handle(), L"标题");
input.create(window.handle(), 1001);
ok_button.create(window.handle(), L"确定", 1002);
cancel_button.create(window.handle(), L"取消", 1003);

win32::Grid grid(window.handle(), 2, 180, 32, 24, 12);
grid.add(title, win32::Grid::Row{0}, win32::Grid::Column{0});
grid.add(input, win32::Grid::Row{1}, win32::Grid::Column{0}, 280, 32);
grid.add(ok_button, win32::Grid::Row{1}, win32::Grid::Column{1}, 120, 32);
grid.add(cancel_button, win32::Grid::Row{2}, win32::Grid::Column{1}, 120, 32);
```

网格会根据父窗口、边距、单元格尺寸和间距自动计算控件位置，不需要传入 `x, y`。`Grid::Row{0}` 和 `Grid::Column{0}` 就是类似 WinUI 3 中 `Grid.Row="0"` 和 `Grid.Column="0"` 的 C++ 写法。

布局工具不会自动监听窗口大小变化。如果窗口需要跟随缩放，请在 `WM_SIZE` 中重新调用布局方法。

## 常用控件示例

### 列表视图

```cpp
win32::ListView table;
table.create(window.handle(), 5001);
table.add_column(L"名称", 180, 0);
table.add_column(L"状态", 120, 1);
table.add_column(L"数量", 80, 2);
int row = table.add_row(L"项目 A");
table.set_cell(row, 1, L"正常");
table.set_cell(row, 2, L"12");

win32::Grid grid(window.handle(), 1, 420, 180, 20, 8);
grid.add(table, win32::Grid::Row{0}, win32::Grid::Column{0}, 420, 180);
```

### 树形视图

```cpp
win32::TreeView tree;
tree.create(window.handle(), 5002);
HTREEITEM root = tree.add_root(L"工作区");
HTREEITEM source = tree.add(L"源代码", root);
tree.add(L"main.cpp", source);
tree.add(L"win32.h", source);

win32::Grid grid(window.handle(), 1, 260, 180, 20, 8);
grid.add(tree, win32::Grid::Row{0}, win32::Grid::Column{0}, 260, 180);
```

### 日期和月历

```cpp
win32::DateTimePicker date;
date.create(window.handle(), 5003);

SYSTEMTIME today{};
if (date.get(today)) {
	// today.wYear、today.wMonth、today.wDay 可读取日期
}

win32::MonthCalendar calendar;
calendar.create(window.handle(), 5004);

win32::Grid grid(window.handle(), 2, 240, 180, 20, 8);
grid.add(date, win32::Grid::Row{0}, win32::Grid::Column{0}, 240, 30);
grid.add(calendar, win32::Grid::Row{1}, win32::Grid::Column{0}, 240, 180);
```

### 富文本和搜索框

```cpp
win32::RichEdit editor;
editor.create(window.handle(), 5005);
editor.set_text(L"支持多行文本。\r\n");

win32::SearchBox search;
search.create(window.handle(), 5006, L"搜索文件");

win32::Grid grid(window.handle(), 1, 420, 120, 20, 8);
grid.add(editor, win32::Grid::Row{0}, win32::Grid::Column{0}, 420, 120);
grid.add(search, win32::Grid::Row{1}, win32::Grid::Column{0}, 260, 32);
```

### 工具栏和状态栏

```cpp
win32::ToolBar toolbar;
toolbar.create(window.handle(), 5007);
toolbar.add_button(5010);
toolbar.autosize();

win32::StatusBar status;
status.create(window.handle());
status.parts({300, -1});
status.text(L"就绪", 0);
status.text(L"UTF-8", 1);
```

工具栏按钮如果没有设置图像列表，系统可能只显示空白按钮。生产项目应使用 `TB_SETIMAGELIST` 设置图标资源，并在 `WM_COMMAND` 中处理对应 ID。

## 对话框和系统工具

### 消息框

```cpp
win32::MessageDialog::show(window.handle(), L"操作完成");
if (win32::MessageDialog::confirm(window.handle(), L"确定退出吗？")) {
	win32::Window::quit();
}
```

### 文件打开和保存

过滤器使用成对的“显示名称”和“匹配模式”，每一段以空字符分隔，最后使用两个空字符结束：

```cpp
const std::wstring filter = L"文本文件\0*.txt\0所有文件\0*.*\0\0";
std::wstring opened = win32::FileDialog::open(window.handle(), filter);
std::wstring saved = win32::FileDialog::save(window.handle(), filter);
```

用户取消时返回空字符串，因此不能只用字符串是否为空判断文件是否存在。

### 剪贴板

```cpp
win32::Clipboard::set_text(window.handle(), input.text());
std::wstring clipboard_text = win32::Clipboard::get_text(window.handle());
```

### 字体和颜色

```cpp
LOGFONTW selected_font{};
selected_font.lfHeight = -16;
wcscpy_s(selected_font.lfFaceName, L"Segoe UI");
if (win32::FontDialog::show(window.handle(), selected_font)) {
	label.set_font(CreateFontIndirectW(&selected_font));
}

COLORREF color = RGB(39, 125, 224);
if (win32::ColorDialog::show(window.handle(), color)) {
	// color 可用于自定义绘制
}
```

## 定时器

定时器使用窗口消息循环，不需要单独创建线程：

```cpp
win32::Timer timer;
timer.start(window.handle(), 6001, 1000, [&]() {
	result.set_text(L"计时器正在运行");
});
```

窗口关闭前停止定时器：

```cpp
timer.stop();
```

同一个窗口上的定时器 ID 应保持唯一。回调中不要执行耗时操作，否则会阻塞整个窗口消息循环。

## Mica 和控件透明度

当前实现把 Mica 仅应用于顶层窗口。控件不会继承 Mica，也不会透出桌面：

- 顶层窗口负责 DWM Mica 背景
- 标签、复选框和分组框使用普通背景
- 输入框、列表框和富文本框使用白色不透明背景
- 主按钮使用蓝色圆角自绘
- 标题栏和系统窗口按钮由 Windows 默认绘制

Mica 是 Windows 11 的系统效果，是否可见还受以下因素影响：

1. 系统版本必须支持 Mica。
2. Windows 设置中的“透明效果”不能被关闭。
3. 窗口不能使用会覆盖客户区的全屏不透明背景刷。
4. 必须把 `resource.o` 链接到最终 exe。

关闭 Mica 后，程序会回退到浅色背景，不影响控件功能。Mica 不是普通的 alpha 透明，它由 DWM 根据桌面内容和系统主题合成，无法保证每台机器显示完全相同。

## 主题和颜色

默认主题使用以下视觉方向：

| 项目 | 默认值 |
| --- | --- |
| 字体 | Segoe UI |
| 窗口背景 | RGB(247, 249, 252) |
| 控件表面 | RGB(255, 255, 255) |
| 普通文字 | RGB(32, 35, 42) |
| 主色 | RGB(39, 125, 224) |
| 按下状态 | RGB(22, 91, 170) |
| 禁用状态 | RGB(190, 198, 210) |

`Theme::round` 使用窗口区域裁剪实现圆角，适合编辑框、搜索框、下拉框和列表表面。复选框、单选框、分组框不应手动套用圆角区域，否则可能裁剪系统绘制的勾选框或边框。

## 编写新控件的建议

如果要在 `win32.h` 中继续添加控件，建议遵循以下模式：

1. 继承 `Control`。
2. 提供 `create(parent, id, x, y, width, height)`。
3. 创建后调用 `Theme::apply(handle_)`。
4. 对需要圆角的表面调用 `Theme::round(handle_)`。
5. 使用 `SendMessageW` 封装控件专用 API。
6. 创建失败时返回 `false`。
7. 不要在控件内部调用 `GetMessageW`，消息循环只能由 `Window` 管理。

示例骨架：

```cpp
class MyControl : public win32::Control {
public:
	bool create(HWND parent, int id, int width, int height) {
		handle_ = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE,
								0, 0, width, height, parent,
								reinterpret_cast<HMENU>(id),
								GetModuleHandleW(nullptr), nullptr);
		win32::Theme::apply(handle_);
		return valid();
	}
};

MyControl control;
control.create(window.handle(), 6001, 240, 32);
win32::Grid grid(window.handle(), 1, 240, 32);
grid.add(control, win32::Grid::Row{0}, win32::Grid::Column{0}, 240, 32);
```

## 当前实现的边界

这是一个轻量 Win32 封装，不是完整 UI 框架：

- 没有 XAML 或声明式界面
- 没有自动数据绑定
- 没有完整响应式布局系统
- 没有主题资源字典
- 没有高 DPI 单位自动换算
- 没有跨平台支持
- Mica 依赖 Windows 11 和 DWM

如果项目需要真正的 WinUI 3 控件模板、XAML、自动布局和 Windows App SDK，应创建 WinUI 3 项目，而不是继续扩展这个单头文件。
