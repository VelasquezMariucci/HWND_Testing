//
// Created by esvel on 1/07/2025.
//

// LiveWallpaperBand.cpp
#define GLFW_EXPOSE_NATIVE_WIN32
#include "CLiveWallpaperBand.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <atomic>

// {…GUID definition repeated…}
const GUID CLSID_LiveWallpaperBand =
        {0xd1142884, 0x82a2, 0x4a95, {0x85, 0xb0, 0x19, 0xdf, 0x86, 0xa5, 0x48, 0x9c}};

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------
CLiveWallpaperBand::CLiveWallpaperBand()
  : _refCount(1),
    _site(nullptr),
    _bandWindow(nullptr),
    _glfwWin(nullptr),
    _glfwHwnd(nullptr)
{}

CLiveWallpaperBand::~CLiveWallpaperBand() {
    DestroyGLFWWindow();
    if (_site) _site->Release();
}

//-----------------------------------------------------------------------------
// IUnknown
//-----------------------------------------------------------------------------
IFACEMETHODIMP CLiveWallpaperBand::QueryInterface(REFIID riid, void **ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown)
        *ppv = static_cast<IUnknown *>(static_cast<IDockingWindow *>(this));
    else if (riid == IID_IObjectWithSite)
        *ppv = static_cast<IObjectWithSite *>(this);
    else if (riid == IID_IDeskBand)
        *ppv = static_cast<IDeskBand *>(this);
    else if (riid == IID_IOleWindow)
        *ppv = static_cast<IOleWindow *>(this);
    else if (riid == IID_IDockingWindow)
        *ppv = static_cast<IDockingWindow *>(this);
    else if (riid == IID_IPersistStream)
        *ppv = static_cast<IPersistStream *>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) CLiveWallpaperBand::AddRef() { return InterlockedIncrement(&_refCount); }

IFACEMETHODIMP_(ULONG) CLiveWallpaperBand::Release() {
    ULONG c = InterlockedDecrement(&_refCount);
    if (!c) delete this;
    return c;
}

//-----------------------------------------------------------------------------
// IObjectWithSite
//-----------------------------------------------------------------------------
IFACEMETHODIMP CLiveWallpaperBand::SetSite(IUnknown *pUnkSite) {
    if (_site) {
        _site->Release();
        _site = nullptr;
    }
    if (pUnkSite) {
        pUnkSite->QueryInterface(IID_PPV_ARGS(&_site));
        // ask site for its IOleWindow to get the HWND
        IOleWindow *pOleWin = nullptr;
        if (SUCCEEDED(_site->QueryInterface(IID_PPV_ARGS(&pOleWin)))) {
            pOleWin->GetWindow(&_bandWindow);
            pOleWin->Release();
        }
    }
    return S_OK;
}

IFACEMETHODIMP CLiveWallpaperBand::GetSite(REFIID riid, void **ppvSite) {
    return _site ? _site->QueryInterface(riid, ppvSite) : E_FAIL;
}

//-----------------------------------------------------------------------------
// IOleWindow
//-----------------------------------------------------------------------------
IFACEMETHODIMP CLiveWallpaperBand::GetWindow(HWND *phwnd) {
    *phwnd = _bandWindow;
    return S_OK;
}

//-----------------------------------------------------------------------------
// IDockingWindow / IDeskBand
//-----------------------------------------------------------------------------
IFACEMETHODIMP CLiveWallpaperBand::ShowDW(BOOL fShow) {
    MessageBoxW(nullptr,
                fShow ? L"ShowDW(TRUE)" : L"ShowDW(FALSE)",
                L"Debug", MB_OK);
    if (fShow) CreateGLFWWindow();
    else DestroyGLFWWindow();
    return S_OK;
}

IFACEMETHODIMP CLiveWallpaperBand::CloseDW(DWORD) {
    DestroyGLFWWindow();
    return S_OK;
}

//-----------------------------------------------------------------------------
// IDeskBand
//-----------------------------------------------------------------------------
IFACEMETHODIMP CLiveWallpaperBand::GetBandInfo(
    DWORD /*dwBandID*/,
    DWORD /*dwViewMode*/,
    DESKBANDINFO *pdbi) {
    if (!pdbi) return E_POINTER;
    pdbi->dwMask = DBIM_MINSIZE | DBIM_MAXSIZE | DBIM_INTEGRAL;
    // assign POINTL members directly
    pdbi->ptMinSize.x = 100;
    pdbi->ptMinSize.y = 100;
    pdbi->ptMaxSize.x = 10000;
    pdbi->ptMaxSize.y = 10000;
    pdbi->ptIntegral.x = 1;
    pdbi->ptIntegral.y = 1;
    return S_OK;
}

//-----------------------------------------------------------------------------
// IPersist
//-----------------------------------------------------------------------------
IFACEMETHODIMP CLiveWallpaperBand::GetClassID(CLSID *pclsid) {
    *pclsid = CLSID_LiveWallpaperBand;
    return S_OK;
}

//-----------------------------------------------------------------------------
// GLFW window helpers
//-----------------------------------------------------------------------------
void CLiveWallpaperBand::CreateGLFWWindow() {
    MessageBoxW(nullptr, L"About to create GLFW window", L"Debug", MB_OK);

    if (_glfwWin) return; // already exists

    // 1) Initialize GLFW (once per process)
    static bool init = glfwInit();
    glfwWindowHint(GLFW_CLIENT_API,   GLFW_OPENGL_API);
    glfwWindowHint(GLFW_DECORATED,    GLFW_FALSE);

    // 2) Create the GLFW window
    _glfwWin = glfwCreateWindow(
        800, 600, "LiveWallpaperBand",
        nullptr, nullptr);
    if (!_glfwWin) return;

    // 3) Get its HWND and parent it into the band window
    _glfwHwnd = glfwGetWin32Window(_glfwWin);
    SetParent(_glfwHwnd, _bandWindow);

    // 4) Switch to child style (and make sure it’s visible)
    LONG_PTR style = GetWindowLongPtr(_glfwHwnd, GWL_STYLE);
    style = (style & ~WS_POPUP) | WS_CHILD | WS_VISIBLE;
    SetWindowLongPtr(_glfwHwnd, GWL_STYLE, style);

    // ─── Insert here ───────────────────────────────────────────────────────────
    // 5) Size it to exactly fill the band’s client area
    RECT rc;
    GetClientRect(_bandWindow, &rc);
    SetWindowPos(_glfwHwnd, HWND_TOP,
                 0, 0,
                 rc.right  - rc.left,
                 rc.bottom - rc.top,
                 SWP_SHOWWINDOW);

    // 6) Make its OpenGL context current so we can draw
    glfwMakeContextCurrent(_glfwWin);

    // 7) Perform an initial clear (blue background)
    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(_glfwWin);
    // ──────────────────────────────────────────────────────────────────────────

    // 8) If you want continuous animation, start a timer or thread here
}

void CLiveWallpaperBand::DestroyGLFWWindow() {
    if (!_glfwHwnd) return;
    DestroyWindow(_glfwHwnd);
    _glfwHwnd = nullptr;
}
