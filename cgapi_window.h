#ifndef _CGAPI_WINDOW_H
#define	_CGAPI_WINDOW_H

#include <d2d1.h>		// Direct2D�� ����ϱ� ���� ��� ���ϰ� ���̺귯�� ������ ���Խ�Ų��.
#include <Wincodec.h>	// IWICImagingFactory�� ����ϱ� ���ؼ� �߰�
using namespace D2D1;

#define CG_SetIRect(x, l, t, r, b){ x.left = l; x.top = t; x.right = r; x.bottom = b; }
#define CG_SetFRect(x, l, t, r, b){ x.left = (float)(l); x.top = (float)(t); x.right = (float)(r); x.bottom = (float)(b); }
#define CG_PosInRect(pos, l, t, r, b) (l <= pos.x && t <= pos.y && r >= pos.x && b >= pos.y) 
#define CG_SetFPos(pt, x_pos, y_pos){ pt.x = (FLOAT)x_pos; pt.y = (FLOAT)y_pos; }
#define CG_SetFSize(size, x_pos, y_pos){ size.width = (FLOAT)x_pos; size.height = (FLOAT)y_pos; }
#define CG_COLOR(color) (color/256.0f)
#define CG_RGB(color, red, green, blue) { color.r = (FLOAT)(red/255.0); color.g = (FLOAT)(green/255.0); color.b = (FLOAT)(blue/255.0); color.a = 1.0f; }
#define CG_FRGB(color, red, green, blue) { color.r = (FLOAT)red; color.g = (FLOAT)green; color.b = (FLOAT)blue; color.a = 1.0f; }
#define CG_Release(x) { if(x != NULL){ x->Release(); x = NULL; }}
#define CG_IMAGE_BMP       0
#define CG_IMAGE_PNG       1
#define CG_IMAGE_JPEG      2

template<class Interface>
inline void CG_IRelease(Interface **ap_interface_object)
{
	if (*ap_interface_object != NULL) {
		(*ap_interface_object)->Release();
		(*ap_interface_object) = NULL;
	}
}

//���� TW
class CG_Wnd
{
protected:
	// �� ��ü�� ������ �������� �ڵ�
	HWND mh_wnd;
	// Direct2D���� �������� Ŭ���̾�Ʈ ������ �׸��� �׸� ��ü 
	ID2D1HwndRenderTarget *mp_target;

public:
	CG_Wnd();  // ��ü ������
	virtual ~CG_Wnd();  // ��ü �ı���

						// ��ü�� �����ϴ� mh_wnd ���� ��ų� ���ο� �ڵ� ���� �����ϴ� �Լ�
	HWND GetHandle() { return mh_wnd; }
	void SetHandle(HWND ah_wnd) { mh_wnd = ah_wnd; }
	// �����츦 ������ �� ����ϴ� �Լ�
	void Create(const wchar_t *ap_class_name, const wchar_t *ap_title_name, HINSTANCE ah_instance, int a_x = 0, int a_y = 0, int a_width = 1024, int a_height = 768, DWORD a_style = WS_OVERLAPPEDWINDOW);
	//void CreateEx(const wchar_t *ap_title_name, int a_x = 50, int a_y = 50, int a_width = 500, int a_height = 300, DWORD a_style = WS_OVERLAPPEDWINDOW, DWORD a_ex_style = WS_EX_TOPMOST | WS_EX_ACCEPTFILES);
	// �����츦 �ı��� �� ����ϴ� �Լ�
	void DestoryWindow();
	// �����츦 ������ ������� �ʰ� ��� �����ϵ��� �ϴ� �Լ�
	void UpdateWindow();
	// �����츦 ȭ�鿡 ��� ��½�ų �������� �����ϴ� �Լ�
	void ShowWindow(int a_show_type = SW_SHOW);
	// �������� Ŭ���̾�Ʈ ������ �������� ������ ũ�⸦ �����ϴ� �Լ�
	void ResizeWindow(int a_width, int a_height);
	// ������ ȭ���� ��ȿȭ ���Ѽ� WM_PAINT �޽����� �߻���Ű�� �Լ�
	void Invalidate();
	// Ÿ�̸Ӹ� ����ϴ� �Լ�
	void SetTimer(UINT a_timer_id, UINT a_elapse_time);
	// Ÿ�̸Ӹ� �����ϴ� �Լ�
	void KillTimer(UINT a_timer_id);
	// ȭ�� ������ ���ϱ�
	void GetClientRect(HWND ah_hwnd, RECT *ap_rect);

	// WM_PAINT �޽����� �߻��� �� �۾��� �Լ�. �� �Լ��� �Ļ� Ŭ�������� ��κ� ������ ��
	virtual void OnPaint();
	// WM_CREATE �޽����� �߻��� �� �۾��� �Լ�.
	virtual int OnCreate(CREATESTRUCT *ap_create_info);
	// WM_TIMER �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnTimer(UINT a_timer_id);
	// WM_COMMAND �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, HWND ah_ctrl);
	// WM_DESTROY �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnDestroy();
	// WM_LBUTTONDOWN �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnLButtonDown(int a_flag, POINT a_pos);
	// WM_LBUTTONUP �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnLButtonUp(int a_flag, POINT a_pos);
	// WM_MOUSEMOVE �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnMouseMove(int a_flag, POINT a_pos);
	// WM_LBUTTONDBLCLK �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnLButtonDblClk(int a_flag, POINT a_pos);
	// WM_RBUTTONDOWN �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnRButtonDown(int a_flag, POINT a_pos);
	// WM_RBUTTONUP �޽����� �߻��� �� �۾��� �Լ�.
	virtual void OnRButtonUp(int a_flag, POINT a_pos);
	// WM_MOUSEWHEEL �޽����� �߻��� �� �۾��� �Լ�.
	virtual int OnMouseWheel(unsigned short a_flag, short a_z_delta, POINT a_pos);
	// CG_Wnd Ŭ������ �������� ���� ������ �޽����� ó���ϴ� ��쿡 ����ϴ� �Լ�
	virtual LRESULT UserMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


class CG_WinApp
{
protected:
	CG_Wnd *mp_wnd;				// ���� ������ ��ü�� �ּҸ� ����
	HINSTANCE mh_instance;		// WinMain�� ���޵Ǵ� ù ��° ���ڰ� ����
	wchar_t m_class_name[32];	// '������ Ŭ����' �̸��� ������ ����, WinMain �Լ��� ���� ��ȯ���� WM_QUIT�޽����� wParam ������ ����ϱ����ؼ� �� ���� ������ ����
	int m_exit_state;

public:
	CG_WinApp(HINSTANCE ah_instance, const wchar_t *ap_class_name, int a_use_ext_direct2d = 0);
	virtual ~CG_WinApp();

	int GetExitState() { return m_exit_state; }
	const wchar_t *GetWindowClassName() { return m_class_name; }
	CG_Wnd *GetMainWindow() { return mp_wnd; }
	HINSTANCE GetInstanceHandle() { return mh_instance; }

	virtual void InitApplication();
	virtual void InitInstance();
	virtual void Run();
	virtual void ExitInstance();
	virtual void ExitApplication();
	virtual int NormalProcess();
};

#endif