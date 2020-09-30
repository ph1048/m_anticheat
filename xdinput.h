#define DIRECTINPUT_VERSION 0x0700
#include "main.h"
#include <dinput.h>
#include "mouse.h"

class xDirectInput : public IDirectInput
{
public:
	xDirectInput(IDirectInput* di) : m_di(di)
	{
	}

	~xDirectInput(void)
	{
		delete m_di;
	}

	/*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj)
	{
		return m_di->QueryInterface(riid, ppvObj);
	}

    ULONG STDMETHODCALLTYPE AddRef()
	{
		return m_di->AddRef();
	}

    ULONG STDMETHODCALLTYPE Release()
	{
		ULONG count = m_di->Release();
		if(0 == count)
			delete this;

		return count;
	}

	/*** IDirectInput methods ***/
    STDMETHOD(CreateDevice)(REFGUID rguid, IDirectInputDevice** device, LPUNKNOWN unknown)
	{
		HRESULT hr = m_di->CreateDevice(rguid, device, unknown);

		if(SUCCEEDED(hr))
		{
			*device = new MouseDevice(*device);
		}

		return hr;
	}

    STDMETHOD(EnumDevices)(DWORD devType,LPDIENUMDEVICESCALLBACK callback, LPVOID ref, DWORD flags)
	{
		return m_di->EnumDevices(devType, callback, ref, flags);
	}

    STDMETHOD(GetDeviceStatus)(REFGUID rguid)
	{
		return m_di->GetDeviceStatus(rguid);
	}

    STDMETHOD(RunControlPanel)(HWND owner, DWORD flags)
	{
		return m_di->RunControlPanel(owner, flags);
	}

    STDMETHOD(Initialize)(HINSTANCE instance, DWORD version)
	{
		return m_di->Initialize(instance, version);
	}

private:
	IDirectInput* m_di;
};


