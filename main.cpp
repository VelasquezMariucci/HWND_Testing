#include <iostream>
#include <windows.h>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    const int bufferSize = 256;
    wchar_t title[bufferSize];
    wchar_t className[bufferSize];

    // Get window title and class name
    int textLength = GetWindowTextW(hwnd, title, bufferSize);
    GetClassNameW(hwnd, className, bufferSize);

    // Check for Chrome's window class or specific title
    if (IsWindowVisible(hwnd))
    {
        // Check for Chrome window title (may vary based on active tab)
        if (wcsstr(title, L"Google Chrome") != NULL || wcsstr(className, L"Chrome_WidgetWin_1") != NULL)
        {
            wprintf(L"Found Chrome window - HWND: %p - Class: %s - Title: %s\n", hwnd, className, title);
        }
    }

    return TRUE; // Continue enumeration
}


int main()
{


    EnumWindows(EnumWindowsProc, 0);
    return 0;
}
