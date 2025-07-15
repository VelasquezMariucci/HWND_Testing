//
// Created by esvel on 1/07/2025.
//

// LiveWallpaperBand.h
#pragma once
#include <windows.h>
#include <Unknwn.h>
#include <shobjidl.h>      // IDeskBand, IObjectWithSite, IOleWindow, IDockingWindow
#include <objidl.h>        // IPersistStream
#include <GLFW/glfw3.h>

// {D1142884-82A2-4A95-85B0-19DF86A5489C}
extern const GUID CLSID_LiveWallpaperBand;

class CLiveWallpaperBand :
        public IDeskBand,
        public IObjectWithSite,
        public IPersistStream // for completeness; you can drop it if you don't need settings
{
public:
    CLiveWallpaperBand();
    ~CLiveWallpaperBand();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);

    IFACEMETHODIMP_(ULONG) AddRef();

    IFACEMETHODIMP_(ULONG) Release();

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *pUnkSite);

    IFACEMETHODIMP GetSite(REFIID riid, void **ppvSite);

    // IOleWindow
    IFACEMETHODIMP GetWindow(HWND *phwnd);

    IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }

    // IDockingWindow
    IFACEMETHODIMP ShowDW(BOOL fShow);

    IFACEMETHODIMP CloseDW(DWORD dwReserved);

    IFACEMETHODIMP ResizeBorderDW(
        LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved) { return E_NOTIMPL; }

    // IDeskBand
    IFACEMETHODIMP GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi) override;

    // IPersist
    IFACEMETHODIMP GetClassID(CLSID *pclsid);

    // IPersistStream (to satisfy IPersistStream; stub implementations)
    IFACEMETHODIMP IsDirty() { return S_FALSE; }
    IFACEMETHODIMP Load(IStream *pStm) { return E_NOTIMPL; }
    IFACEMETHODIMP Save(IStream *pStm, BOOL fClearDirty) { return E_NOTIMPL; }
    IFACEMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize) { return E_NOTIMPL; }

private:
    // COM housekeeping
    LONG                _refCount;
    IInputObjectSite   *_site;        // the site you SetSite()

    // our “host” window provided by Explorer/TestHost
    HWND                _bandWindow;

    // GLFW‐window state
    GLFWwindow*         _glfwWin;     // must be the GLFWwindow*
    HWND                _glfwHwnd;    // the Win32 HWND for that window


    void CreateGLFWWindow(); // call inside ShowDW(TRUE)
    void DestroyGLFWWindow(); // call inside CloseDW
};
