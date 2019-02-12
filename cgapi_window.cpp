#include "stdafx.h"
#include "cgapi_window.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ������ �ν��Ͻ��� �߰��� 4����Ʈ �޸𸮿��� CG_Wnd ��ü�� �ּҸ� �����´�. 
	CG_Wnd *p_wnd = (CG_Wnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (uMsg == WM_PAINT) {
		ValidateRect(hWnd, NULL);  // WM_PAINT �޽����� �ٽ� �߻����� �ʰ� �����. 
		p_wnd->OnPaint();
		return 0;
	}
	else if (uMsg == WM_CREATE) {
		// CreateWindow �Լ��� ����Ǳ� ���� WM_CREATE �޽����� �߻��ϱ� ������ 
		// ������ �ν��Ͻ��� �߰��� 4����Ʈ���� CG_Wnd ��ü�� �ּҰ� ����� ����Ǿ� ���� 
		// �ʱ� ������ ������ ���������� CG_Wnd ��ü�� �ּҸ� ��Ƽ� ������ 
		p_wnd = (CG_Wnd *)((CREATESTRUCT *)lParam)->lpCreateParams;
		// ���� CreateWindow �Լ��� ������� �ʾƼ� mh_wnd�� ����ε� �ڵ� ���� 
		// ����Ǿ� ���� �ʾƼ� �ڵ� ���� ������ 
		p_wnd->SetHandle(hWnd);
		// ������ �ν��Ͻ��� �߰��� 4����Ʈ�� CG_Wnd ��ü�� �ּҸ� ������ 
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

// �����츦 �ı��� �� ����ϴ� �Լ� 
void CG_Wnd::DestoryWindow()
{
	if (mh_wnd != NULL) ::DestroyWindow(mh_wnd);
}

// �����츦 ������ ������� �ʰ� ��� �����ϵ��� �ϴ� �Լ� 
void CG_Wnd::UpdateWindow()
{
	::UpdateWindow(mh_wnd);
}

// �����츦 ȭ�鿡 ��� ��½�ų �������� �����ϴ� �Լ� 
void CG_Wnd::ShowWindow(int a_show_type)
{
	::ShowWindow(mh_wnd, a_show_type);
}

// �������� Ŭ���̾�Ʈ ������ �������� ������ ũ�⸦ �����ϴ� �Լ� 
void CG_Wnd::ResizeWindow(int a_width, int a_height)
{
	RECT client_rect, win_rect;
	// ���� �������� Ŭ���̾�Ʈ ���� ��ǥ�� ��´�. 
	::GetClientRect(mh_wnd, &client_rect);
	// �׸��� ũ�⿡ �°� ������ ũ�⸦ �����ϱ� ���ؼ� ������ ��ǥ�� ��´�. 
	::GetWindowRect(mh_wnd, &win_rect);
	// Ŭ���̾�Ʈ ������ ũ��� �׸� ũ���� ���̸� ����Ͽ� ������ ũ�⸦ �������Ѵ�. 
	::SetWindowPos(mh_wnd, NULL, 0, 0, win_rect.right - win_rect.left + a_width - client_rect.right,
		win_rect.bottom - win_rect.top + a_height - client_rect.bottom, SWP_NOMOVE);
}

// ������ ȭ���� ��ȿȭ ���Ѽ� WM_PAINT �޽����� �߻���Ű�� �Լ� 
void CG_Wnd::Invalidate()
{
	::InvalidateRect(mh_wnd, NULL, FALSE);
}

// Ÿ�̸Ӹ� ����ϴ� �Լ� 
void CG_Wnd::SetTimer(UINT a_timer_id, UINT a_elapse_time)
{
	::SetTimer(mh_wnd, a_timer_id, a_elapse_time, NULL);
}

// Ÿ�̸Ӹ� �����ϴ� �Լ� 
void CG_Wnd::KillTimer(UINT a_timer_id)
{
	::KillTimer(mh_wnd, a_timer_id);
}

///
void CG_Wnd::GetClientRect(HWND ah_hwnd, RECT *ap_rect)
{
	::GetClientRect(ah_hwnd, ap_rect);
}

// WM_PAINT �޽����� �߻��� �� �۾��� �Լ�. �� �Լ��� �Ļ� Ŭ�������� ��κ� ������ �� 
void CG_Wnd::OnPaint()
{

}

// WM_CREATE �޽����� �߻��� �� �۾��� �Լ�. Render Target ��ü�� �� ������ �Ź� ������ 
// �ϱ� ������ �θ� Ŭ�������� �⺻������ �۾��ϰ� �߽��ϴ�. �׸��� �� �Լ��� ���޵Ǵ� �Ű� ������ 
// WM_CREATE �޽����� lParam �� ���޵Ǵ� ���Դϴ�. 
int CG_Wnd::OnCreate(CREATESTRUCT *ap_create_info)
{
	RECT client_rect;
	GetClientRect(mh_wnd, &client_rect);  // Ŭ���̾�Ʈ ������ ��ǥ�� ��´�. 
										  // ������ �������� Ŭ���̾�Ʈ ������ �׸��� �׸��� ���� Render Target�� �����Ѵ�. 
	//gp_factory->CreateHwndRenderTarget(RenderTargetProperties(), HwndRenderTargetProperties(mh_wnd, SizeU(client_rect.right, client_rect.bottom)), &mp_target);
	return 0;
}

// WM_TIMER �޽����� �߻��� �� �۾��� �Լ�. 
void CG_Wnd::OnTimer(UINT a_timer_id)
{

}

// WM_DESTROY �޽����� �߻��� �� �۾��� �Լ�. Render Target ��ü�� �����ϴ� �۾��� 
// WM_QUIT �޽����� �߻���Ű�� �۾��� �Ź� �ؾ��ϴ� �۾��̶� �� Ŭ�������� ó���մϴ�.  
void CG_Wnd::OnDestroy()
{
	// ����ϴ� Render Target�� �ִٸ� �����Ѵ�. 
	CG_IRelease(&mp_target);
	// WM_QUIT�� �޽��� ť�� �־ ���α׷��� �����Ŵ 
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
	WNDCLASS wc;      //window class ��� wchar_t my_class_name[] = L"CLASSNAME"; �� �κ��� �ۺ��� CG_WinApp�� wcscpy(m_class_name, ap_class_name);�κ� ������ ���� �ʿ䰡 ��������.
	wc.cbClsExtra = NULL;
	//wc.cbWndExtra = NULL;
	wc.cbWndExtra = 4;							//�߰� �޸�
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;    //������ ���
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);   //�������� ȭ��ǥ (�𷡽ð踦 ����ϸ� Ŭ���̾ȵȴ�)
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = mh_instance;
	wc.lpfnWndProc = WndProc;				//4~9�� ����
	wc.lpszClassName = m_class_name;		//�迭�̸�
	wc.lpszMenuName = NULL;					//�޴� ������
	wc.style = CS_HREDRAW | CS_VREDRAW;		//���ø����̼ǿ��� ������� 
											//��� �� ������ ����   (CS=class style)    
	RegisterClass(&wc);						//window class�� ��ϵȴ�. ��밡���ϴ�.
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