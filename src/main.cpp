#include <windows.h>
#include <iostream>
#include <functional>

// returns the empty WorkerW that is directly behind the visible icons
HWND GetWallpaperWorkerW()
{
	// Ask Explorer to create the extra WorkerW (23H2 trick still works)
	if (HWND prog = FindWindowW(L"Progman", nullptr))
	{
		SendMessageTimeoutW(prog, 0x052C, 0xD, 0, SMTO_NORMAL, 100, nullptr);
		SendMessageTimeoutW(prog, 0x052C, 0xD, 1, SMTO_NORMAL, 100, nullptr);
	}

	HWND wallpaperW = nullptr, lastWorker = nullptr;

	EnumWindows([](HWND hWnd, LPARAM p)->BOOL
		{
			wchar_t cls[32] = {};
			GetClassNameW(hWnd, cls, 32);

			if (wcscmp(cls, L"WorkerW") == 0)
			{
				// remember every WorkerW we meet
				HWND* last = reinterpret_cast<HWND*>(p);
				*last = hWnd;
				// if THIS WorkerW owns the icon ListView,
				// the *next* WorkerW we meet is the empty wallpaper host
				return FindWindowExW(hWnd, nullptr, L"SHELLDLL_DefView", nullptr) != nullptr;
			}
			return TRUE;            // keep enumerating
		}, reinterpret_cast<LPARAM>(&lastWorker));

	// lastWorker is now the empty “wallpaper” WorkerW
	wallpaperW = lastWorker;
	return wallpaperW;
}


HWND FindDesktopWorker()
{
	HWND desktop = nullptr;
	EnumWindows([](HWND top, LPARAM param)->BOOL {
		if (FindWindowExW(top, nullptr, L"SHELLDLL_DefView", nullptr))
		{
			*reinterpret_cast<HWND*>(param) = top;   // <-- this WorkerW (or Progman on ≤23H2)
			return FALSE;                            // stop enumeration
		}
		return TRUE;
		}, reinterpret_cast<LPARAM>(&desktop));
	return desktop;   // will be 0 on failure
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// Fill background with red
		HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
		FillRect(hdc, &ps.rcPaint, brush);
		DeleteObject(brush);

		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreateSimpleWindow(HINSTANCE hInstance) {
	static const wchar_t CLASS_NAME[] = L"MyWindowClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, L"My Window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
		nullptr, nullptr, hInstance, nullptr
	);

	SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

	return hwnd;
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lparam) {
	wchar_t className[256], windowText[256];

	GetClassNameW(hwnd, className, 256);
	GetWindowText(hwnd, windowText, 256);

	std::wcout << L"Child HWND: " << hwnd << L" | Class: " << className;

	if (wcslen(windowText) > 0) {
		std::wcout << L" | Title: " << windowText;
	}

	std::wcout << std::endl;

	return TRUE;
}

BOOL CALLBACK EnumChildWorkerWFinder(HWND hwnd, LPARAM lParam) {
	wchar_t className[256];
	GetClassNameW(hwnd, className, 256);

	if (wcscmp(className, L"WorkerW") == 0) {
		HWND* outHwnd = reinterpret_cast<HWND*>(lParam);
		*outHwnd = hwnd;
		return FALSE; // Stop enumeration once found
	}

	return TRUE; // Continue searching
}

HWND FindWorkerW() {
	HWND progman = FindWindow(L"Progman", NULL);
	if (!progman) {
		std::wcout << L"Progman not found.\n";
		return nullptr;
	}

	HWND workerW = nullptr;
	EnumChildWindows(progman, EnumChildWorkerWFinder, reinterpret_cast<LPARAM>(&workerW));

	if (workerW) {
		std::wcout << L"Found WorkerW inside Progman: " << workerW << std::endl;
	}
	else {
		std::wcout << L"No WorkerW found inside Progman.\n";
	}

	return workerW;
}

void CreateWorkerW(HWND progman) {
	if (progman) {
		SendMessageA(progman, 0x052C, 0xD, 0);
		SendMessageA(progman, 0x052C, 0xD, 1);
		std::cout << "Message Sent, WorkerW creation triggered." << std::endl;
	}
	else {
		std::cout << "Cannot send message — Progman is null." << std::endl;
	}
}

void DumpAllDefViews()
{
	std::wcout << L"--- Enumerating complete window tree\n";
	std::function<void(HWND)> walk = [&](HWND w)
		{
			for (HWND c = GetWindow(w, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT))
			{
				wchar_t cls[64] = {};
				GetClassNameW(c, cls, 64);

				if (wcscmp(cls, L"SHELLDLL_DefView") == 0)
				{
					HWND parent = GetParent(c);
					wchar_t pcls[64] = {};
					GetClassNameW(parent, pcls, 64);

					std::wcout << L"  SHELLDLL_DefView 0x"
						<< std::hex << (UINT_PTR)c
						<< L"  | parent 0x" << (UINT_PTR)parent
						<< L" (" << pcls << L")\n";
				}
				walk(c);          // recurse
			}
		};
	EnumWindows([](HWND w, LPARAM ctx)->BOOL { (*reinterpret_cast<std::function<void(HWND)>*>(ctx))(w); return TRUE; },
		reinterpret_cast<LPARAM>(&walk));
	std::wcout << L"--- end\n";
}

int main() {
	DumpAllDefViews();

	HWND progman = FindWindow(L"Progman", NULL);
	if (!progman) {
		std::wcout << L"Progman NOT found." << std::endl;
		return 1;
	}

	std::wcout << L"Found Progman HWND: " << progman << std::endl;

	// Print Progman's children
	EnumChildWindows(progman, EnumChildProc, 0);

	// Send message to spawn WorkerW
	CreateWorkerW(progman);

	// Reprint Progman's children
	EnumChildWindows(progman, EnumChildProc, 0);

	HWND workerW_hwnd = FindWorkerW();

	if (workerW_hwnd) {
		std::cout << "Found WorkerW" << std::endl;
	}

	HWND top_hwnd = GetTopWindow(progman);

	std::cout << "Top Most Window inside Progman: " << top_hwnd << std::endl;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	HWND my_hwnd = CreateSimpleWindow(hInstance);
	HWND shellView = FindWindowEx(progman, nullptr, L"SHELLDLL_DefView", nullptr);
	HWND sysListView = FindWindowEx(shellView, nullptr, L"SysListView32", nullptr);
	HWND sysHeader = FindWindowEx(sysListView, nullptr, L"SysHeader32", nullptr);
	HWND desktopOwnerWorkerW = FindDesktopWorker();
	HWND lastDesktopWorkerW = GetWallpaperWorkerW();

	std::cout << "MyWindow: " << my_hwnd << std::endl;
	std::cout << "SHELLDLL_DefView: " << shellView << std::endl;
	std::cout << "SysListView32: " << sysListView << std::endl;
	std::cout << "SysHeader32: " << sysHeader << std::endl;
	std::cout << "SHELLDLL_DefView Owner: " << desktopOwnerWorkerW << std::endl;
	std::cout << "Last Desktop WorkerW: " << lastDesktopWorkerW<< std::endl;

	_sleep(1500);

	LONG_PTR style = GetWindowLongPtr(my_hwnd, GWL_STYLE);
	style |= WS_CHILD;
	SetWindowLongPtr(my_hwnd, GWL_STYLE, style);

	SetParent(my_hwnd, lastDesktopWorkerW);
	ShowWindow(my_hwnd, SW_SHOW);
	//SetWindowPos(my_hwnd, HWND_BOTTOM, 100, 0, 600, 400, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	UpdateWindow(my_hwnd);

	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}