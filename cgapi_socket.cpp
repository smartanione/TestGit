#include "stdafx.h"
#include "cgapi_socket.h"

//////////////////////////////////////////////////////////////////////////
CG_ExchangeManager::CG_ExchangeManager()
{
	mp_data = NULL;
	m_total_size = 0;
	m_cur_size = 0;
}

CG_ExchangeManager::~CG_ExchangeManager()
{
	DeleteData();
}

char *CG_ExchangeManager::MemoryAlloc(int a_data_size)
{
	if (m_total_size != a_data_size) {
		if (mp_data != NULL) delete[] mp_data;
		mp_data = new char[a_data_size];
		m_total_size = a_data_size;
	}
	m_cur_size = 0;
	return mp_data;
}

void CG_ExchangeManager::DeleteData()
{
	if (mp_data != NULL) {
		delete[] mp_data;
		mp_data = NULL;
		m_total_size = 0;
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
int CG_SendManager::GetPosition(char **ap_data, int a_data_size)
{
	*ap_data = mp_data + m_cur_size;
	if (m_cur_size + a_data_size < m_total_size) {
		m_cur_size += a_data_size;
	}
	else {
		a_data_size = m_total_size - m_cur_size;
		m_cur_size = m_total_size;
	}
	return a_data_size;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
int CG_RecvManager::AddData(char *ap_data, int a_size)
{
	memcpy(mp_data + m_cur_size, ap_data, a_size);
	m_cur_size += a_size;
	return m_cur_size;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

CG_Socket::CG_Socket(unsigned char a_valid_key, int a_data_notify_id)
{
	m_valid_key = a_valid_key;
	mp_send_data = new char[8192];
	mh_notify_wnd = NULL;
	m_data_notify_id = a_data_notify_id;

	WSADATA temp;
	::WSAStartup(0x0202, &temp);
}

CG_Socket::~CG_Socket()
{
	delete[] mp_send_data;
	delete[] mp_recv_data;
	::WSACleanup();
}

int CG_Socket::SendFrameData(SOCKET ah_socket, unsigned char a_message_id, const char *ap_body_data, BS a_body_size)
{
	*(unsigned char *)(mp_send_data + 1) = a_message_id;
	*(BS *)(mp_send_data + 2) = a_body_size;
	memcpy(mp_send_data + HEAD_SIZE, ap_body_data, a_body_size);
	if (send(ah_socket, mp_send_data, HEAD_SIZE + a_body_size, 0) == (a_body_size + HEAD_SIZE)) {
		// 전송 성공
		return 1; 
	}
	//전송 실패
	return 0;
}

int CG_Socket::ReceiveData(SOCKET ah_socket, BS a_body_size)
{
	BS total_size = 0;
	int read_size, retry = 0;

	while (total_size < a_body_size) {
		read_size = recv(ah_socket, mp_recv_data + total_size, a_body_size - total_size, 0);
		if (read_size == SOCKET_ERROR) {
			retry++;
			Sleep(50);
			if (retry > 10) break;
		}
		else {
			if (read_size > 0) total_size = total_size + (BS)read_size;
			if (total_size < a_body_size) Sleep(5);
			retry = 0;
		}
	}
	//0:수신 실패, 1:수신 성공(재시도가 10보다 작거나 같으면)
	return retry <= 10;
}

void CG_Socket::ProcessRecvEvent(SOCKET ah_socket)
{
	unsigned char msg_id, key;
	BS body_size;

	//FD_READ 이벤트가 과도하게 발생하지 않도록 FD_READ이벤트를 제거
	::WSAAsyncSelect(ah_socket, mh_notify_wnd, m_data_notify_id, FD_CLOSE);

	//인증값을 수신
	recv(ah_socket, (char *)&key, 1, 0);
	if (key == m_valid_key)
	{
		//msg_id 수신 1바이트
		recv(ah_socket, (char *)&msg_id, 1, 0);
		//body_size 수신 2바이트
		recv(ah_socket, (char *)&body_size, sizeof(BS), 0);
		//바디 사이즈가 0인 경우에는 수신할 필요가 없다. 전송할 데이터가 없음.
		if (body_size > 0) {
			// 바디 사이즈는 크기 때문에 recv함수가 아닌 안정적인 재시도 수신이 가능한 함수로 데이터 수신
			if (!ReceiveData(ah_socket, body_size)) {
				//데이터를 수신하다가 오류가 발생한 경우 연결된 소켓을 해제
				DisconnectSocket(ah_socket, -2);
				return;
			}
		}
		//정상적으로 헤드와 바디를 수신한 경우 사용자가 원하는 작업 처리
		//ProcessRecvData 함수는 상속받은 자식 클래스에서 재정의하여 원하는 작업 처리
		if (1 == ProcessRecvData(ah_socket, msg_id, mp_recv_data, body_size)) {
			//소켓에 문제가 발생하지 않았다면 수신 이벤트 처리가 가능하도록 FD_READ 추가
			::WSAAsyncSelect(ah_socket, mh_notify_wnd, m_data_notify_id, FD_CLOSE | FD_READ);
		}
	}
	else {
		//인증값이 잘못된 경우 접속을 해제
		DisconnectSocket(ah_socket, -1);
	}
}

//자식 클래스에서 꼭 재정의해서 사용해야 함, 서버와 클라이언트가 다르기 때문
void CG_Socket::DisconnectSocket(SOCKET ah_socket, int a_error_code)
{

}

// 수신된 데이터를 처리하는 함수 - 자식 클래스에서 꼭 재정의해서 사용해야 함, 서버와 클라이언트가 다르기 때문
int CG_Socket::ProcessRecvData(SOCKET ah_socket, char a_msg_id, char *ap_recv_data, BS a_body_size)
{
	return 0;
}

void CG_Socket::AsciiToUnicode_IP(wchar_t *ap_dest_ip, char *ap_src_ip)
{
	int ip_length = strlen(ap_src_ip) + 1;
	memset(ap_dest_ip, 0, ip_length << 1);
	for (int i = 0; i < ip_length; i++) {
		ap_dest_ip[i] = ap_src_ip[i];
	}

}

void CG_Socket::UnicodeToAscii_IP(char *ap_dest_ip, wchar_t *ap_src_ip)
{
	int ip_length = wcslen(ap_src_ip) + 1;
	for (int i = 0; i < ip_length; i++) {
		ap_dest_ip[i] = (char)ap_src_ip[i];
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CG_UserData::CG_UserData()
{
	mh_socket = INVALID_SOCKET;
	m_ip_address[0] = 0;
	mp_send_man = new CG_SendManager;
	mp_recv_man = new CG_RecvManager;
}

CG_UserData::~CG_UserData()
{
	if (mh_socket != INVALID_SOCKET) closesocket(mh_socket);
	delete mp_send_man;
	delete mp_recv_man;
}

wchar_t *CG_UserData::GetIP()
{
	return m_ip_address;
}

void CG_UserData::SetIP(const wchar_t *ap_ip_address)
{
	//wcscpy(m_ip_address, ap_ip_address);
	wcscpy_s(m_ip_address, wcslen(ap_ip_address) + 1, ap_ip_address);
}

void CG_UserData::CloseSocket(char a_linger_flag)
{
	if (mh_socket != INVALID_SOCKET) {
		if (a_linger_flag) {
			LINGER temp = { TRUE, 0 };
			setsockopt(mh_socket, SOL_SOCKET, SO_LINGER, (char *)&temp, sizeof(temp));
		}
		closesocket(mh_socket);
		mh_socket = INVALID_SOCKET;
	}
}
//////////////////////////////////////////////////////////////////////////
CG_ServerSocket::CG_ServerSocket(char a_valid_key, int a_max_user_count, CG_UserData *ap_user_data, int a_accept_notify_id, int a_data_notify_id) : CG_Socket(a_valid_key, a_data_notify_id)
//CG_ServerSocket::CG_ServerSocket(char a_valid_key, int a_max_user_count, int a_accept_notify_id, int a_data_notify_id) : CG_Socket(a_valid_key, a_data_notify_id)
{
	m_user_count = 0;
	m_max_user_count = a_max_user_count;
	mh_listen_socket = INVALID_SOCKET;
	mp_user_list = new CG_UserData *[m_max_user_count];
	for (int i = 0; i < m_max_user_count; i++) {
		mp_user_list[i] = ap_user_data->CreateObject();
		//mp_user_list[i] = new CG_UserData();
	}
	m_accept_notify_id = a_accept_notify_id;
	delete ap_user_data;
}

CG_ServerSocket::~CG_ServerSocket()
{
	if (mh_listen_socket != INVALID_SOCKET) {
		closesocket(mh_listen_socket);
	}
	//사용자만큼 생성된 객체 제거
	for (int i = 0; i < m_max_user_count; i++) {
		delete mp_user_list[i];
	}
	//포인터 제거
	delete[] mp_user_list;
}

int CG_ServerSocket::StartServer(const wchar_t *ap_ip_address, int a_port, HWND ah_notify_wnd)
{
	mh_notify_wnd = ah_notify_wnd;
	struct sockaddr_in serv_addr;
	mh_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	//소켓 생성 체크 0보다 작으면 소켓 생성 실패
	if (mh_listen_socket < 0) return -1;

	char temp[16];
	UnicodeToAscii_IP(temp, (wchar_t *)ap_ip_address);
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(temp);
	serv_addr.sin_port = htons((short unsigned int)a_port);

	if (bind(mh_listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		// 실패하면 소켓 제거, 초기화
		closesocket(mh_listen_socket);
		mh_listen_socket = INVALID_SOCKET;
		return -2; // 바인드 오류
	}
	//클라이언트 접속 허락
	listen(mh_listen_socket, 5);
	//새로운 클라이언트 접속하면 FD_ACCEPT 이벤트 발생
	//accept()함수를 실행하면 접속할때까지 계속 기다림, 비동기 사용 이유
	WSAAsyncSelect(mh_listen_socket, ah_notify_wnd, m_accept_notify_id, FD_ACCEPT);
	
	return 1; // 성공
}
int CG_ServerSocket::ProcessToAccept(WPARAM wParam, LPARAM lParam)
{
	struct sockaddr_in clnt_addr;
	int i, temp = sizeof(clnt_addr);
	//새로 접속 시도하는 클라이언트와 통신할 소켓 생성
	SOCKET h_client_socket = accept((SOCKET)wParam, (struct sockaddr *)&clnt_addr, &temp);
	if (h_client_socket == INVALID_SOCKET) return -1;
	else {
		CG_UserData *p_user;
		wchar_t temp_ip[16];
		AsciiToUnicode_IP(temp_ip, inet_ntoa(clnt_addr.sin_addr));
		for (i = 0; i < m_max_user_count; i++) {
			p_user = mp_user_list[i];
			//INVALID_SOCKET이면 미사용 유저 객체
			if (p_user->GetHandle() == INVALID_SOCKET) {
				p_user->SetHandle(h_client_socket);
				p_user->SetIP(temp_ip);
				//fd_read, fd_close가 발생하면 프로시저에 메시지 발생하도록
				WSAAsyncSelect(h_client_socket, mh_notify_wnd, m_data_notify_id, FD_READ | FD_CLOSE);
				//새로운 사용자 접속 처리
				AddWorkForAccept(p_user);
				break;
			}
		}
		//접속 자수
		m_user_count++;
		//접속 초과
		if (i == m_max_user_count) {
			//접속 초과 에러 메시지
			ShowLimitError(temp_ip);
			closesocket(h_client_socket);
			return -2; //사용자 초과 종료
		}
	}
	return 1;// 정상 종료
}

void CG_ServerSocket::AddWorkForCloseUser(CG_UserData  *ap_user, int a_error_code)
{
	//
}

void CG_ServerSocket::ProcessClientEvent(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTEVENT(lParam) == FD_READ) {
		ProcessRecvEvent((SOCKET)wParam);
	}
	else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
		CG_UserData *p_data = FindUserData((SOCKET)wParam);
		// 접속자수 
		m_user_count--;
		// 접속 해제 전 처리
		AddWorkForCloseUser(p_data, 0);
		p_data->CloseSocket(0); // 0 : 그냥 종료
	}
}

void CG_ServerSocket::DisconnectSocket(SOCKET ah_socket, int a_error_code)
{
	CG_UserData *p_user_data = FindUserData(ah_socket);
	// 접속 해제 전 처리
	AddWorkForCloseUser(p_user_data, a_error_code);
	p_user_data->CloseSocket(1); // 1 : LINGER 설정 후 종료
}

int CG_ServerSocket::ProcessRecvData(SOCKET ah_socket, char a_msg_id, char *ap_recv_data, BS a_body_size)
{
	//소켓으로 접속한 유저데이터를 찾음
	CG_UserData *p_user_data = FindUserData(ah_socket);

	//예약 메시지 251번은 클라이언트에 큰용량의 데이터를 전송하기 위해 사용
	if (a_msg_id == 251) {
		char *p_send_data;
		//현재 전송 위치
		BS send_size = p_user_data->GetSendMan()->GetPosition(&p_send_data);
		//전송할 데이터가 더 있다면 예약 메시지 번호 252를 사용하여 클라이언트에게 데이터 전송
		if (p_user_data->GetSendMan()->IsProcessing()) {
			SendFrameData(ah_socket, 252, p_send_data, send_size);
		}
		else {
			//분할된 데이터의 마지막 부분이라면 예약 메시지 번호 253을 사용하여 클라이언트에게 데이터 전송
			SendFrameData(ah_socket, 253, p_send_data, send_size);
			//마지막 데이터를 전송하고 사용했던 메모리 삭제
			p_user_data->GetSendMan()->DeleteData();
			//윈도우 프로시저에 전송이 완료되었음을 알려준다
			//전송이 완료되었을때 프로그램에 어떤 표시를 하고 싶다면 해당 윈도우에서
			//LM_SEND_COMPLETED 메시지를 체크
			::PostMessage(mh_notify_wnd, LM_SEND_COMPLETED, (WPARAM)p_user_data, 0);
		}
	}
	//252번은 대용량의 데이터를 수신할 때 사용하는 번호
	else if (a_msg_id == 252) {
		//수신된 데이터는 수신을 관리하는 객체로 넘겨서 데이터를 합침
		p_user_data->GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//252번은 아직도 추가 수신할 데이터가 있다는 뜻, 251을 클라이언트에 전송, 추가데이터 요청
		SendFrameData(ah_socket, 251, NULL, 0);
	}
	//253은 대용량의 데이터를 수신할때
	else if (a_msg_id == 253) {
		//수신된 데이터는 수신을 관리하는 객체로 넘겨서 데이터를 합침.
		p_user_data->GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//253은 데이터 수신이 완료되었다는 메시지, 프로시저에 알림
		//LM_RECV_COMPLETED 메시지를 수신한 프로시저에서 사용한 메모리 DeleteData 함수 호출
		::PostMessage(mh_notify_wnd, LM_RECV_COMPLETED, (WPARAM)p_user_data, 0);
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CG_ClientSocket::CG_ClientSocket(char a_valid_key, int a_connect_notify_id, int a_data_notify_id) : CG_Socket(a_valid_key, a_data_notify_id)
{
	//접속 상태를 해제상태로 초기화
	m_connect_flag = 0;
	//소켓 핸들 초기화
	mh_socket = INVALID_SOCKET;
	//FD_CONNECT 이벤트 발생시 사용할 윈도우 메시지 번호
	m_connect_notify_id = a_connect_notify_id;
}

CG_ClientSocket::~CG_ClientSocket()
{
	//서버와 통신하기 위한 소켓이 생성되어 있다면 소켓 제거
	if (mh_socket != INVALID_SOCKET) {
		closesocket(mh_socket);
	}
}

int CG_ClientSocket::ConnectToServer(const wchar_t *ap_ip_address, int a_port_num, HWND ah_notify_wnd)
{
	//접속을 시도중이거나 접속된 상태라면 접속을 시도하지 않는다.
	if (m_connect_flag != 0) {
		return 0;
	}
	//소켓 이벤트, 윈도우 메시지 받을 윈도우 핸들
	mh_notify_wnd = ah_notify_wnd;
	//서버와 통신할 소켓 생성
	mh_socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serv_addr;
	char temp[16];
	UnicodeToAscii_IP(temp, (wchar_t *)ap_ip_address);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(temp);
	serv_addr.sin_port = htons(a_port_num);
	
	//서버에 접속하는 connect()함수는 응답없음에 빠질 수 있어 비동기 설정
	//서버 접속 결과인 FD_CONNECT 이벤트가 발생하면 프로시저에 m_connect_notify_id에 해당하는 메시지 전송
	WSAAsyncSelect(mh_socket, ah_notify_wnd, m_connect_notify_id, FD_CONNECT);
	//접속 시도
	connect(mh_socket, (sockaddr *)&serv_addr, sizeof(serv_addr));
	// 접속 상태를 접속 '시도중'으로 변경
	m_connect_flag = 1;
	
	return 1;
}

int CG_ClientSocket::ResultOfConnection(LPARAM lParam)
{
	//접속 에러 없음 == 서버에 성공적으로 연결됨
	if (WSAGETSELECTERROR(lParam) == 0) {
		//접속 상태를 '접속'으로 변경
		m_connect_flag = 2;
		//접속된 소켓으로 서버에서 데이터가 수신되거나 연결이 해제되었을때 프로시저에 메시지 발생
		WSAAsyncSelect(mh_socket, mh_notify_wnd, m_data_notify_id, FD_READ | FD_CLOSE);
		//접속 성공
		return 1;
	}
	else {
		//접속 실패
		closesocket(mh_socket);
		//소켓 초기화
		mh_socket = INVALID_SOCKET;
		//접속상태 '해제'로 변경
		m_connect_flag = 0;
	}
	//접속 실패
	return 0;
}

int CG_ClientSocket::ProcessServerEvent(WPARAM wParam, LPARAM lParam)
{
	int state = 1;
	if (WSAGETSELECTEVENT(lParam) == FD_READ) {
		ProcessRecvEvent((SOCKET)wParam);
	}
	else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
		state = 0;
		m_connect_flag = 0;
		closesocket(mh_socket);
		mh_socket = INVALID_SOCKET;
	}
	return state;
}

void CG_ClientSocket::DisconnectSocket(SOCKET ah_socket, int a_error_code)
{
	m_connect_flag = 0;
	LINGER temp = { TRUE,0 };
	setsockopt(mh_socket, SOL_SOCKET, SO_LINGER, (char *)&temp, sizeof(temp));
	closesocket(mh_socket);
	mh_socket = INVALID_SOCKET;
}

int CG_ClientSocket::ProcessRecvData(SOCKET ah_socket, char a_msg_id, char *ap_recv_data, BS a_body_size)
{
	//예약 메시지 251번은 클라이언트에 큰용량의 데이터를 전송하기 위해 사용
	if (a_msg_id == 251) {
		char *p_send_data;
		//현재 전송 위치
		BS send_size = GetSendMan()->GetPosition(&p_send_data);
		//전송할 데이터가 더 있다면 예약 메시지 번호 252를 사용하여 클라이언트에게 데이터 전송
		if (GetSendMan()->IsProcessing()) {
			CG_Socket::SendFrameData(ah_socket, 252, p_send_data, send_size);
		}
		else {
			//분할된 데이터의 마지막 부분이라면 예약 메시지 번호 253을 사용하여 클라이언트에게 데이터 전송
			CG_Socket::SendFrameData(ah_socket, 253, p_send_data, send_size);
			//마지막 데이터를 전송하고 사용했던 메모리 삭제
			GetSendMan()->DeleteData();
			//윈도우 프로시저에 전송이 완료되었음을 알려준다
			//전송이 완료되었을때 프로그램에 어떤 표시를 하고 싶다면 해당 윈도우에서
			//LM_SEND_COMPLETED 메시지를 체크
			::PostMessage(mh_notify_wnd, LM_SEND_COMPLETED, 0, 0);
		}
	}
	//252번은 대용량의 데이터를 수신할 때 사용하는 번호
	else if (a_msg_id == 252) {
		//수신된 데이터는 수신을 관리하는 객체로 넘겨서 데이터를 합침
		GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//252번은 아직도 추가 수신할 데이터가 있다는 뜻, 251을 클라이언트에 전송, 추가데이터 요청
		CG_Socket::SendFrameData(ah_socket, 251, NULL, 0);
	}
	//253은 대용량의 데이터를 수신할때
	else if (a_msg_id == 253) {
		//수신된 데이터는 수신을 관리하는 객체로 넘겨서 데이터를 합침.
		GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//253은 데이터 수신이 완료되었다는 메시지, 프로시저에 알림
		//LM_RECV_COMPLETED 메시지를 수신한 프로시저에서 사용한 메모리 DeleteData 함수 호출
		::PostMessage(mh_notify_wnd, LM_RECV_COMPLETED, 0, 0);
	}
	//수신된 데이터를 정상적으로 처리함, 만약, 수신 데이터를 처리하던 중에 소켓을 제거했으면 0 반환
	//0을 반환하면 비동기 작업이 중단
	return 1;
}

int CG_ClientSocket::SendFrameData(char a_message_id, char *ap_body_data, BS a_body_size)
{
	return CG_Socket::SendFrameData(mh_socket, a_message_id, ap_body_data, a_body_size);
}
//////////////////////////////////////////////////////////////////////////
