#include "stdafx.h"
#include "cgapi_window.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 윈도우 인스턴스에 추가된 4바이트 메모리에서 CG_Wnd 객체의 주소를 가져온다. 
	CG_Wnd *p_wnd = (CG_Wnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (uMsg == WM_PAINT) {
		ValidateRect(hWnd, NULL);  // WM_PAINT 메시지가 다시 발생하지 않게 만든다. 
		p_wnd->OnPaint();
		return 0;
	}
	else if (uMsg == WM_CREATE) {
		// CreateWindow 함수가 종료되기 전에 WM_CREATE 메시지가 발생하기 때문에 
		// 윈도우 인스턴스에 추가된 4바이트에도 CG_Wnd 객체의 주소가 제대로 저장되어 있지 
		// 않기 때문에 윈도우 생성정보에 CG_Wnd 객체의 주소를 담아서 보냈음 
		p_wnd = (CG_Wnd *)((CREATESTRUCT *)lParam)->lpCreateParams;
		// 아직 CreateWindow 함수가 종료되지 않아서 mh_wnd에 제대로된 핸들 값이 
		// 저장되어 있지 않아서 핸들 값을 설정함 
		p_wnd->SetHandle(hWnd);
		// 윈도우 인스턴스에 추가된 4바이트에 CG_Wnd 객체의 주소를 저장함 
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)p_wnd);
		return p_wnd->OnCreate((CREATESTRUCT *)lParam);
	}
	else if (uMsg == WM_TIMER) {
		
p_wnd->OnTimer((UINT)wParam);
		return 0;
	}
	else if (uMsg == WM_DESTROY) p_wnd->OnDestroy();

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

CG_Wnd::CG_Wnd()
{
	mh_wnd = NULL;
	mp_target = NULL;
}

CG_Wnd::~CG_Wnd()
{
}
 
void CG_Wnd::Create(const wchar_t *ap_class_name, const wchar_t *ap_title_name, HINSTANCE ap_hinstance, int a_x, int a_y, int a_width, int a_height, DWORD a_style)
{
	mh_wnd = ::CreateWindow(ap_class_name, ap_title_name, a_style, a_x, a_y, a_width, a_height, NULL, NULL, ap_hinstance, (void *)this);
}

// 윈도우를 파괴할 때 사용하는 함수 
void CG_Wnd::DestoryWindow()
{
	if (mh_wnd != NULL) ::DestroyWindow(mh_wnd);
}

// 윈도우를 갱신을 대기하지 않고 즉시 갱신하도록 하는 함수 
void CG_Wnd::UpdateWindow()
{
	::UpdateWindow(mh_wnd);
}

// 윈도우를 화면에 어떻게 출력시킬 것인지를 결정하는 함수 
void CG_Wnd::ShowWindow(int a_show_type)
{
	::ShowWindow(mh_wnd, a_show_type);
}

// 윈도우의 클라이언트 영역을 기준으로 윈도우 크기를 변경하는 함수 
void CG_Wnd::ResizeWindow(int a_width, int a_height)
{
	RECT client_rect, win_rect;
	// 현재 윈도우의 클라이언트 영역 좌표를 얻는다. 
	::GetClientRect(mh_wnd, &client_rect);
	// 그림의 크기에 맞게 윈도우 크기를 변경하기 위해서 윈도우 좌표를 얻는다. 
	::GetWindowRect(mh_wnd, &win_rect);
	// 클라이언트 영역의 크기와 그림 크기의 차이를 계산하여 윈도우 크기를 재조정한다. 
	::SetWindowPos(mh_wnd, NULL, 0, 0, win_rect.right - win_rect.left + a_width - client_rect.right,
		win_rect.bottom - win_rect.top + a_height - client_rect.bottom, SWP_NOMOVE);
}

// 윈도우 화면을 무효화 시켜서 WM_PAINT 메시지를 발생시키는 함수 
void CG_Wnd::Invalidate()
{
	::InvalidateRect(mh_wnd, NULL, FALSE);
}

// 타이머를 등록하는 함수 
void CG_Wnd::SetTimer(UINT a_timer_id, UINT a_elapse_time)
{
	::SetTimer(mh_wnd, a_timer_id, a_elapse_time, NULL);
}

// 타이머를 제거하는 함수 
void CG_Wnd::KillTimer(UINT a_timer_id)
{
	::KillTimer(mh_wnd, a_timer_id);
}

///
void CG_Wnd::GetClientRect(HWND ah_hwnd, RECT *ap_rect)
{
	::GetClientRect(ah_hwnd, ap_rect);
}

// WM_PAINT 메시지가 발생할 때 작업할 함수. 이 함수는 파생 클래스에서 대부분 재정의 함 
void CG_Wnd::OnPaint()
{

}

// WM_CREATE 메시지가 발생할 때 작업할 함수. Render Target 객체는 이 시점에 매번 만들어야 
// 하기 때문에 부모 클래스에서 기본적으로 작업하게 했습니다. 그리고 이 함수에 전달되는 매개 변수는 
// WM_CREATE 메시지의 lParam 에 전달되는 값입니다. 
int CG_Wnd::OnCreate(CREATESTRUCT *ap_create_info)
{
	RECT client_rect;
	GetClientRect(mh_wnd, &client_rect);  // 클라이언트 영역의 좌표를 얻는다. 
										  // 지정한 윈도우의 클라이언트 영역에 그림을 그리기 위한 Render Target을 생성한다. 
	//gp_factory->CreateHwndRenderTarget(RenderTargetProperties(), HwndRenderTargetProperties(mh_wnd, SizeU(client_rect.right, client_rect.bottom)), &mp_target);
	return 0;
}

// WM_TIMER 메시지가 발생할 때 작업할 함수. 
void CG_Wnd::OnTimer(UINT a_timer_id)
{

}

// WM_DESTROY 메시지가 발생할 때 작업할 함수. Render Target 객체를 제거하는 작업과 
// WM_QUIT 메시지를 발생시키는 작업은 매번 해야하는 작업이라서 이 클래스에서 처리합니다.  
void CG_Wnd::OnDestroy()
{
	// 사용하던 Render Target이 있다면 제거한다. 
	CG_IRelease(&mp_target);
	// WM_QUIT를 메시지 큐에 넣어서 프로그램을 종료시킴 
	PostQuitMessage(0);
}

void CG_Wnd::OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, HWND ah_ctrl)
{
}

void CG_Wnd::OnLButtonDown(int a_flag, POINT a_pos)
{
}

void CG_Wnd::OnLButtonUp(int a_flag, POINT a_pos)
{
}

void CG_Wnd::OnMouseMove(int a_flag, POINT a_pos)
{
}

void CG_Wnd::OnLButtonDblClk(int a_flag, POINT a_pos)
{

}


void CG_Wnd::OnRButtonDown(int a_flag, POINT a_pos)
{

}

void CG_Wnd::OnRButtonUp(int a_flag, POINT a_pos)
{

}

int CG_Wnd::OnMouseWheel(unsigned short a_flag, short a_z_delta, POINT a_pos)
{
	return 0;
}

LRESULT CG_Wnd::UserMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CG_WinApp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CG_WinApp::CG_WinApp(HINSTANCE ah_instance, const wchar_t *ap_class_name, int a_use_ext_direct2d)
{
	mh_instance = ah_instance;
	wcscpy_s(m_class_name, wcslen(ap_class_name) + 1, ap_class_name);
	InitApplication();
	InitInstance();
	Run();
}

CG_WinApp::~CG_WinApp()
{

}

void CG_WinApp::InitApplication()
{
	WNDCLASS wc;      //window class 등록 wchar_t my_class_name[] = L"CLASSNAME"; 이 부분은 퍼블릭의 CG_WinApp의 wcscpy(m_class_name, ap_class_name);부분 때문에 적을 필요가 없어진다.
	wc.cbClsExtra = NULL;
	//wc.cbWndExtra = NULL;
	wc.cbWndExtra = 4;							//추가 메모리
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;    //배경색을 흰색
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);   //아이콘은 화살표 (모래시계를 사용하면 클릭이안된다)
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = mh_instance;
	wc.lpfnWndProc = WndProc;				//4~9줄 내용
	wc.lpszClassName = m_class_name;		//배열이름
	wc.lpszMenuName = NULL;					//메뉴 사용안함
	wc.style = CS_HREDRAW | CS_VREDRAW;		//어플리케이션에서 드로잉을 
											//어떻게 할 것인지 설정   (CS=class style)    
	RegisterClass(&wc);						//window class가 등록된다. 사용가능하다.
}

void CG_WinApp::InitInstance()
{
	//mp_wnd = new CG_Wnd();
	//mp_wnd->Create(m_class_name, m_class_name, mh_instance);
	//mp_wnd->ShowWindow();
	//mp_wnd->UpdateWindow();
}

void CG_WinApp::Run()
{
	MSG msg;	
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void CG_WinApp::ExitInstance()
{
	if (mp_wnd != NULL) {
		mp_wnd->DestoryWindow();
		delete mp_wnd;
		mp_wnd = NULL;
	}
}

void CG_WinApp::ExitApplication()
{

}

int CG_WinApp::NormalProcess()
{
	InitApplication();
	InitInstance();
	Run();
	ExitInstance();
	ExitApplication();
	return GetExitState();
}