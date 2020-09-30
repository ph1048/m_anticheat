#include "main.h"
#include <ddraw.h>

class xDirectDraw : public IDirectDraw7
{
public:
	xDirectDraw(IDirectDraw7* dd) : m_dd(dd)
	{
		printf("dd1\r\n");
	}

	~xDirectDraw(void)
	{
		printf("dd2\r\n");
		delete m_dd;
	}

	/*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj)
	{
		printf("dd3\r\n");
		return m_dd->QueryInterface(riid, ppvObj);
	}

    ULONG STDMETHODCALLTYPE AddRef()
	{
		printf("dd4\r\n");
		return m_dd->AddRef();
	}

    ULONG STDMETHODCALLTYPE Release()
	{
		printf("dd5\r\n");
		ULONG count = m_dd->Release();
		if(0 == count)
			delete this;

		return count;
	}

    /*** IDirectDraw methods ***/
    STDMETHOD(Compact)(THIS)
	{
		printf("dd6\r\n");
		return m_dd->Compact();
	}

    STDMETHOD(CreateClipper)(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter)
	{
		printf("dd7\r\n");
		return m_dd->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
	}

    STDMETHOD(CreatePalette)(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE FAR *lplpDDPalette, IUnknown FAR *pUnkOuter)
	{
		static LPDIRECTDRAWPALETTE l = 0;LPDIRECTDRAWPALETTE ll = 0;
		HRESULT r = m_dd->CreatePalette(dwFlags, lpDDColorArray, &ll, pUnkOuter);
		if(!l)
			l = ll;
		*lplpDDPalette = l;
		return r;
	}

    STDMETHOD(CreateSurface)(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPDIRECTDRAWSURFACE7 FAR *lplpDDSurface, IUnknown FAR *pUnkOuter)
	{
		printf("dd9\r\n");
		static bool done = 0;
		
		return m_dd->CreateSurface(lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
	}

    STDMETHOD(DuplicateSurface)(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDIRECTDRAWSURFACE7 FAR * lplpDupDDSurface)
	{
		printf("dd10\r\n");
		return m_dd->DuplicateSurface(lpDDSurface, lplpDupDDSurface);
	}

    STDMETHOD(EnumDisplayModes)(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback)
	{
		printf("dd11\r\n");
		return m_dd->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
	}

    STDMETHOD(EnumSurfaces)(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSD, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback)
	{
		printf("dd12\r\n");
		return m_dd->EnumSurfaces(dwFlags, lpDDSD, lpContext, lpEnumSurfacesCallback);
	}

    STDMETHOD(FlipToGDISurface)(THIS)
	{
		printf("dd12\r\n");
		return m_dd->FlipToGDISurface();
	}

    STDMETHOD(GetCaps)(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
	{
		printf("dd13\r\n");
		return m_dd->GetCaps(lpDDDriverCaps, lpDDHELCaps);
	}

    STDMETHOD(GetDisplayMode)(LPDDSURFACEDESC2 lpDDSurfaceDesc)
	{
		printf("dd14\r\n");
		return m_dd->GetDisplayMode(lpDDSurfaceDesc);
	}

    STDMETHOD(GetFourCCCodes)(LPDWORD lpNumCodes, LPDWORD lpCodes)
	{
		printf("dd15\r\n");
		return m_dd->GetFourCCCodes(lpNumCodes, lpCodes);
	}

    STDMETHOD(GetGDISurface)(LPDIRECTDRAWSURFACE7 FAR *lplpGDIDDSSurface)
	{
		printf("dd16\r\n");
		return m_dd->GetGDISurface(lplpGDIDDSSurface);
	}

    STDMETHOD(GetMonitorFrequency)(LPDWORD lpdwFrequency)
	{
		printf("dd17\r\n");
		return m_dd->GetMonitorFrequency(lpdwFrequency);
	}

    STDMETHOD(GetScanLine)(LPDWORD lpdwScanLine)
	{
		printf("dd18\r\n");
		return m_dd->GetScanLine(lpdwScanLine);
	}

    STDMETHOD(GetVerticalBlankStatus)(LPBOOL lpbIsInVB)
	{
		printf("dd19\r\n");
		return m_dd->GetVerticalBlankStatus(lpbIsInVB);
	}

    STDMETHOD(Initialize)(GUID FAR *lpGUID)
	{
		printf("dd20\r\n");
		return m_dd->Initialize(lpGUID);
	}

    STDMETHOD(RestoreDisplayMode)(THIS)
	{
		printf("dd21\r\n");
		return m_dd->RestoreDisplayMode();
	}

    STDMETHOD(SetCooperativeLevel)(HWND hWnd, DWORD dwFlags)
	{
		printf("dd22\r\n");
		return m_dd->SetCooperativeLevel(hWnd, dwFlags);
	}

    STDMETHOD(SetDisplayMode)(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
	{
		printf("dd23\r\n");
		return m_dd->SetDisplayMode(dwWidth, dwHeight, dwBPP, dwRefreshRate, dwFlags);
	}

    STDMETHOD(WaitForVerticalBlank)(DWORD dwFlags, HANDLE hEvent)
	{
		//printf("dd24\r\n");
		return m_dd->WaitForVerticalBlank(dwFlags, hEvent);
	}
	
	STDMETHOD(GetAvailableVidMem)(LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree)
	{
		printf("dd25\r\n");
		return m_dd->GetAvailableVidMem(lpDDSCaps2, lpdwTotal, lpdwFree);
	}
	
    /*** Added in the V4 Interface ***/
    STDMETHOD(GetSurfaceFromDC) (HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
	{
		printf("dd26\r\n");
		return m_dd->GetSurfaceFromDC(hdc, lpDDS);
	}
	
    STDMETHOD(RestoreAllSurfaces)(THIS)
	{
		printf("dd27\r\n");
		return m_dd->RestoreAllSurfaces();
	}
	
    STDMETHOD(TestCooperativeLevel)(THIS)
	{
		printf("dd28\r\n");
		return m_dd->TestCooperativeLevel();
	}
	
    STDMETHOD(GetDeviceIdentifier)(LPDDDEVICEIDENTIFIER2 lpdddi, DWORD dwFlags)
	{
		printf("dd29\r\n");
		return m_dd->GetDeviceIdentifier(lpdddi, dwFlags);
	}
	
    STDMETHOD(StartModeTest)(LPSIZE lpModesToTest, DWORD dwNumEntries, DWORD dwFlags)
	{
		printf("dd30\r\n");
		return m_dd->StartModeTest(lpModesToTest, dwNumEntries, dwFlags);
	}
	
    STDMETHOD(EvaluateMode)(DWORD dwFlags, DWORD *pSecondsUntilTimeout)
	{
		printf("dd31\r\n");
		return m_dd->EvaluateMode(dwFlags, pSecondsUntilTimeout);
	}

private:
	IDirectDraw7* m_dd;
};