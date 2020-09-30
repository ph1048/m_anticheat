#include <dinput.h>

class MouseDevice : public IDirectInputDevice
{
public:
	MouseDevice(IDirectInputDevice* device) : m_device(device)
	{
	}

	~MouseDevice(void)
	{
		delete m_device;
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
	{
		return m_device->QueryInterface(riid, ppvObj);
	}

	STDMETHOD_(ULONG,AddRef)()
	{
		return m_device->AddRef();
	}

	STDMETHOD_(ULONG,Release)()
	{
		ULONG count = m_device->Release();
		if(0 == count)
			delete this;

		return count;
	}

	/*** IDirectInputDevice methods ***/
	STDMETHOD(GetCapabilities)(LPDIDEVCAPS devCaps)
	{
		return m_device->GetCapabilities(devCaps);
	}

	STDMETHOD(EnumObjects)(LPDIENUMDEVICEOBJECTSCALLBACK callback, LPVOID ref, DWORD flags)	
	{
		return m_device->EnumObjects(callback, ref, flags);
	}

	STDMETHOD(GetProperty)(REFGUID rguid, LPDIPROPHEADER ph)
	{
		return m_device->GetProperty(rguid, ph);
	}

	STDMETHOD(SetProperty)(REFGUID rguid, LPCDIPROPHEADER ph)
	{
		return m_device->SetProperty(rguid, ph);
	}

	STDMETHOD(Acquire)()
	{
		bMouseActive = true;
		dwMouseState = 0;
		ZeroMemory(VirtualKeyboard,sizeof(VirtualKeyboard));
		return m_device->Acquire();
	}

	STDMETHOD(Unacquire)()
	{
		bMouseActive = false;
		dwMouseState = 0;
		ZeroMemory(VirtualKeyboard,sizeof(VirtualKeyboard));
		return m_device->Unacquire();
	}

	STDMETHOD(GetDeviceState)(DWORD size, LPVOID data)
	{
		HRESULT hr = m_device->GetDeviceState(size, data);
		if(!SUCCEEDED(hr))
			return hr;

		*(PDWORD)&((LPDIMOUSESTATE)data)->rgbButtons = dwMouseState;
		return hr;
	}

	STDMETHOD(GetDeviceData)(DWORD size, LPDIDEVICEOBJECTDATA data, LPDWORD numElements, DWORD flags)
	{
		return m_device->GetDeviceData(size, data, numElements, flags);
	}

	STDMETHOD(SetDataFormat)(LPCDIDATAFORMAT df)
	{
		return m_device->SetDataFormat(df);
	}

	STDMETHOD(SetEventNotification)(HANDLE event)
	{
		return m_device->SetEventNotification(event);
	}

	STDMETHOD(SetCooperativeLevel)(HWND window, DWORD level)
	{
		return m_device->SetCooperativeLevel(window, level);
	}

	STDMETHOD(GetObjectInfo)(LPDIDEVICEOBJECTINSTANCE object, DWORD objId, DWORD objHow)
	{
		return m_device->GetObjectInfo(object, objId, objHow);
	}

	STDMETHOD(GetDeviceInfo)(LPDIDEVICEINSTANCE di)
	{
		return m_device->GetDeviceInfo(di);
	}

	STDMETHOD(RunControlPanel)(HWND owner, DWORD flags)
	{
		return m_device->RunControlPanel(owner, flags);
	}

	STDMETHOD(Initialize)(HINSTANCE instance, DWORD version, REFGUID rguid)
	{
		return m_device->Initialize(instance, version, rguid);
	}

private:
	IDirectInputDevice* m_device;
};