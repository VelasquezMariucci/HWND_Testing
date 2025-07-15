//
// Created by esvel on 1/07/2025.
//

// LiveWallpaperBandFactory.h
#pragma once
#include <windows.h>
#include <objbase.h>   // IClassFactory
#include "CLiveWallpaperBand.h"

class CLiveWallpaperBandFactory : public IClassFactory
{
    LONG _refCount;

public:
    CLiveWallpaperBandFactory() : _refCount(1) {}
    ~CLiveWallpaperBandFactory() {}

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    IFACEMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&_refCount);
    }
    IFACEMETHODIMP_(ULONG) Release() {
        ULONG c = InterlockedDecrement(&_refCount);
        if (!c) delete this;
        return c;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) {
        if (pUnkOuter) return CLASS_E_NOAGGREGATION;
        CLiveWallpaperBand* band = new CLiveWallpaperBand();
        HRESULT hr = band->QueryInterface(riid, ppv);
        band->Release();  // balance ref from constructor
        return hr;
    }

    IFACEMETHODIMP LockServer(BOOL fLock) {
        if (fLock) CoAddRefServerProcess();
        else      CoReleaseServerProcess();
        return S_OK;
    }
};

