// WinClass.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "WinClass.h"


class MyWnd : public CG_Wnd
{
protected:

public:
	//MyWnd() {}
	//~MyWnd() {}
	virtual int OnCreate(CREATESTRUCT *ap_create_info)
	{
		CG_Wnd::OnCreate(ap_create_info);
		return 0;
	}
	virtual void OnDestroy()
	{
		CG_Wnd::OnDestroy();
	}

	virtual LRESULT UserMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}
};

class MyApp : public CG_WinApp
{
public:
	MyApp(HINSTANCE ah_instance, const wchar_t *ap_class_name) : CG_WinApp(ah_instance, ap_class_name){}

	virtual void InitInstance()
	{
		mp_wnd = new MyWnd;
		mp_wnd->Create(GetWindowClassName(), L"TEST", GetInstanceHandle(), 0, 0, 1024, 768, WS_OVERLAPPEDWINDOW); // title name
		mp_wnd->ShowWindow();
		mp_wnd->UpdateWindow();
	}
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstancd, LPSTR lpCmdLine, int nCmdShow)
{
	MyApp cg_app(hInstance, L"TEST"); // classname
	return cg_app.NormalProcess();
}