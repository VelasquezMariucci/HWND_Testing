#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// TCHAR string type
using tstring = basic_string<TCHAR>;

// Automatic stream selection
#ifdef UNICODE
#define tcout wcout
#else
#define tcout cout
#endif

struct WindowInfo {
    tstring title;
    tstring className;
    RECT windowRect;
    DWORD processId;
    DWORD threadId;
};

BOOL CALLBACK getWindowInfo(HWND hwnd, LPARAM lParam) {
    const DWORD TITLE_SIZE = 1024;
    TCHAR windowTitle[TITLE_SIZE];
    TCHAR windowClassName[TITLE_SIZE];

    GetWindowText(hwnd, windowTitle, TITLE_SIZE);
    int length = GetWindowTextLength(hwnd);

    GetClassName(hwnd, windowClassName, TITLE_SIZE);

    RECT rect;
    GetWindowRect(hwnd, &rect);

    DWORD processId;
    DWORD threadId = GetWindowThreadProcessId(hwnd, &processId);

    if (!IsWindowVisible(hwnd) || length == 0 || tstring(windowTitle) == TEXT("Program Manager")) {
        return TRUE;
    }

    WindowInfo winInfo = { windowTitle, windowClassName, rect, processId, threadId };
    vector<WindowInfo>* vec = reinterpret_cast<vector<WindowInfo>*>(lParam);
    vec->push_back(winInfo);

    return TRUE;
}

void printWindowInfo(const vector<WindowInfo>& windows) {
    for (const auto& win : windows) {
        tcout << TEXT("Title: ") << win.title << endl;
        tcout << TEXT("Class: ") << win.className << endl;
        tcout << TEXT("Position: (") << win.windowRect.left << TEXT(", ") << win.windowRect.top
              << TEXT(") - (") << win.windowRect.right << TEXT(", ") << win.windowRect.bottom << TEXT(")") << endl;
        tcout << TEXT("Process ID: ") << win.processId << endl;
        tcout << TEXT("Thread ID: ") << win.threadId << endl;
        tcout << TEXT("------------------------------------------") << endl;
    }
}

int main() {
    vector<WindowInfo> windows;

    EnumWindows(getWindowInfo, reinterpret_cast<LPARAM>(&windows));

    printWindowInfo(windows);

    cin.get();
    return 0;
}
