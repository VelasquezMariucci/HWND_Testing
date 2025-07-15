//
// Created by esvel on 1/07/2025.
//

// DllExports.cpp
// DllExports.cpp
#include <CLiveWallpaperBandFactory.h>
#include <windows.h>
#include "CLiveWallpaperBand.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
long g_cDllRef = 0;
HMODULE g_hModule = nullptr;

//-----------------------------------------------------------------------------
// DllMain
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  /*lpReserved*/)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
// DllCanUnloadNow
//-----------------------------------------------------------------------------
STDAPI DllCanUnloadNow()
{
    return (g_cDllRef == 0) ? S_OK : S_FALSE;
}

//-----------------------------------------------------------------------------
// DllGetClassObject
//   (unchanged — your factory lives here)
//-----------------------------------------------------------------------------
STDAPI DllGetClassObject(REFCLSID clsid,
                         REFIID   riid,
                         void   **ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    // Only our CLSID is supported
    if (IsEqualCLSID(clsid, CLSID_LiveWallpaperBand))
    {
        // Create the factory, QI for the requested interface, then Release()
        auto* factory = new CLiveWallpaperBandFactory();
        if (!factory) return E_OUTOFMEMORY;

        HRESULT hr = factory->QueryInterface(riid, ppv);
        factory->Release();
        return hr;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

//-----------------------------------------------------------------------------
// DllRegisterServer
//-----------------------------------------------------------------------------
STDAPI DllRegisterServer()
{
    // 1) Get this DLL’s path
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(g_hModule, path, _countof(path));

    // 2) Build the CLSID key name
    wchar_t keyCLSID[64];
    wsprintfW(keyCLSID,
             L"CLSID\\{%08lX-%04X-%04X-%02X%02X-%02X%02X-%02X%02X-%02X%02X}",
             CLSID_LiveWallpaperBand.Data1,
             CLSID_LiveWallpaperBand.Data2,
             CLSID_LiveWallpaperBand.Data3,
             CLSID_LiveWallpaperBand.Data4[0],
             CLSID_LiveWallpaperBand.Data4[1],
             CLSID_LiveWallpaperBand.Data4[2],
             CLSID_LiveWallpaperBand.Data4[3],
             CLSID_LiveWallpaperBand.Data4[4],
             CLSID_LiveWallpaperBand.Data4[5],
             CLSID_LiveWallpaperBand.Data4[6],
             CLSID_LiveWallpaperBand.Data4[7]);

    // 3) Create HKCR\CLSID\{Your-GUID}
    HKEY hKey = nullptr;
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, keyCLSID,
                        0, nullptr,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE, nullptr,
                        &hKey, nullptr) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }
    // Friendly name
    const wchar_t* friendly = L"Live Wallpaper DeskBand";
    RegSetValueExW(hKey, nullptr, 0, REG_SZ,
                   (BYTE*)friendly,
                   (DWORD)((wcslen(friendly)+1)*sizeof(wchar_t)));

    // 4) InprocServer32 subkey
    HKEY hSub = nullptr;
    RegCreateKeyExW(hKey, L"InprocServer32",
                    0, nullptr,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE, nullptr,
                    &hSub, nullptr);
    RegSetValueExW(hSub, nullptr, 0, REG_SZ,
                   (BYTE*)path,
                   (DWORD)((wcslen(path)+1)*sizeof(wchar_t)));
    RegSetValueExW(hSub, L"ThreadingModel", 0, REG_SZ,
                   (BYTE*)L"Apartment",
                   (DWORD)((wcslen(L"Apartment")+1)*sizeof(wchar_t)));
    RegCloseKey(hSub);

    // 5) Implemented Categories\{CATID_DeskBand}
    wchar_t catKey[128];
    wsprintfW(catKey,
             L"CLSID\\%s\\Implemented Categories\\{00021492-0000-0000-C000-000000000046}",
             keyCLSID + 6);  // skip the leading "CLSID\"
    HKEY hCat = nullptr;
    RegCreateKeyExW(HKEY_CLASSES_ROOT, catKey,
                    0, nullptr,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE, nullptr,
                    &hCat, nullptr);
    RegCloseKey(hCat);

    RegCloseKey(hKey);
    return S_OK;
}

//-----------------------------------------------------------------------------
// DllUnregisterServer
//-----------------------------------------------------------------------------
STDAPI DllUnregisterServer()
{
    // Build the same CLSID key name
    wchar_t keyCLSID[64];
    wsprintfW(keyCLSID,
             L"CLSID\\{%08lX-%04X-%04X-%02X%02X-%02X%02X-%02X%02X-%02X%02X}",
             CLSID_LiveWallpaperBand.Data1,
             CLSID_LiveWallpaperBand.Data2,
             CLSID_LiveWallpaperBand.Data3,
             CLSID_LiveWallpaperBand.Data4[0],
             CLSID_LiveWallpaperBand.Data4[1],
             CLSID_LiveWallpaperBand.Data4[2],
             CLSID_LiveWallpaperBand.Data4[3],
             CLSID_LiveWallpaperBand.Data4[4],
             CLSID_LiveWallpaperBand.Data4[5],
             CLSID_LiveWallpaperBand.Data4[6],
             CLSID_LiveWallpaperBand.Data4[7]);

    // Recursively delete the entire CLSID tree
    RegDeleteTreeW(HKEY_CLASSES_ROOT, keyCLSID);
    return S_OK;
}

