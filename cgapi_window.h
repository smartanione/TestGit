#ifndef _CGAPI_WINDOW_H
#define	_CGAPI_WINDOW_H

#include <d2d1.h>		// Direct2D를 사용하기 위한 헤더 파일과 라이브러리 파일을 포함시킨다.
#include <Wincodec.h>	// IWICImagingFactory를 사용하기 위해서 추가
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

//원작 TW
class CG_Wnd
{
protected:
	// 이 객체가 관리할 윈도우의 핸들
	HWND mh_wnd;
	// Direct2D에서 윈도우의 클라이언트 영역에 그림을 그릴 객체 
	ID2D1HwndRenderTarget *mp_target;

public:
	CG_Wnd();  // 객체 생성자
	virtual ~CG_Wnd();  // 객체 파괴자

						// 객체가 관리하는 mh_wnd 값을 얻거나 새로운 핸들 값을 설정하는 함수
	HWND GetHandle() { return mh_wnd; }
	void SetHandle(HWND ah_wnd) { mh_wnd = ah_wnd; }
	// 윈도우를 생성할 때 사용하는 함수
	void Create(const wchar_t *ap_class_name, const wchar_t *ap_title_name, HINSTANCE ah_instance, int a_x = 0, int a_y = 0, int a_width = 1024, int a_height = 768, DWORD a_style = WS_OVERLAPPEDWINDOW);
	//void CreateEx(const wchar_t *ap_title_name, int a_x = 50, int a_y = 50, int a_width = 500, int a_height = 300, DWORD a_style = WS_OVERLAPPEDWINDOW, DWORD a_ex_style = WS_EX_TOPMOST | WS_EX_ACCEPTFILES);
	// 윈도우를 파괴할 때 사용하는 함수
	void DestoryWindow();
	// 윈도우를 갱신을 대기하지 않고 즉시 갱신하도록 하는 함수
	void UpdateWindow();
	// 윈도우를 화면에 어떻게 출력시킬 것인지를 결정하는 함수
	void ShowWindow(int a_show_type = SW_SHOW);
	// 윈도우의 클라이언트 영역을 기준으로 윈도우 크기를 변경하는 함수
	void ResizeWindow(int a_width, int a_height);
	// 윈도우 화면을 무효화 시켜서 WM_PAINT 메시지를 발생시키는 함수
	void Invalidate();
	// 타이머를 등록하는 함수
	void SetTimer(UINT a_timer_id, UINT a_elapse_time);
	// 타이머를 제거하는 함수
	void KillTimer(UINT a_timer_id);
	// 화면 사이즈 구하기
	void GetClientRect(HWND ah_hwnd, RECT *ap_rect);

	// WM_PAINT 메시지가 발생할 때 작업할 함수. 이 함수는 파생 클래스에서 대부분 재정의 함
	virtual void OnPaint();
	// WM_CREATE 메시지가 발생할 때 작업할 함수.
	virtual int OnCreate(CREATESTRUCT *ap_create_info);
	// WM_TIMER 메시지가 발생할 때 작업할 함수.
	virtual void OnTimer(UINT a_timer_id);
	// WM_COMMAND 메시지가 발생할 때 작업할 함수.
	virtual void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, HWND ah_ctrl);
	// WM_DESTROY 메시지가 발생할 때 작업할 함수.
	virtual void OnDestroy();
	// WM_LBUTTONDOWN 메시지가 발생할 때 작업할 함수.
	virtual void OnLButtonDown(int a_flag, POINT a_pos);
	// WM_LBUTTONUP 메시지가 발생할 때 작업할 함수.
	virtual void OnLButtonUp(int a_flag, POINT a_pos);
	// WM_MOUSEMOVE 메시지가 발생할 때 작업할 함수.
	virtual void OnMouseMove(int a_flag, POINT a_pos);
	// WM_LBUTTONDBLCLK 메시지가 발생할 때 작업할 함수.
	virtual void OnLButtonDblClk(int a_flag, POINT a_pos);
	// WM_RBUTTONDOWN 메시지가 발생할 때 작업할 함수.
	virtual void OnRButtonDown(int a_flag, POINT a_pos);
	// WM_RBUTTONUP 메시지가 발생할 때 작업할 함수.
	virtual void OnRButtonUp(int a_flag, POINT a_pos);
	// WM_MOUSEWHEEL 메시지가 발생할 때 작업할 함수.
	virtual int OnMouseWheel(unsigned short a_flag, short a_z_delta, POINT a_pos);
	// CG_Wnd 클래스가 정의하지 않은 윈도우 메시지를 처리하는 경우에 사용하는 함수
	virtual LRESULT UserMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


class CG_WinApp
{
protected:
	CG_Wnd *mp_wnd;				// 메인 윈도우 객체의 주소를 저장
	HINSTANCE mh_instance;		// WinMain에 전달되는 첫 번째 인자값 저장
	wchar_t m_class_name[32];	// '윈도우 클래스' 이름을 저장할 변수, WinMain 함수의 최종 반환값을 WM_QUIT메시지의 wParam 값으로 사용하기위해서 이 값을 저장할 변수
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