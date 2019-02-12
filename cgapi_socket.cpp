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
		// ���� ����
		return 1; 
	}
	//���� ����
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
	//0:���� ����, 1:���� ����(��õ��� 10���� �۰ų� ������)
	return retry <= 10;
}

void CG_Socket::ProcessRecvEvent(SOCKET ah_socket)
{
	unsigned char msg_id, key;
	BS body_size;

	//FD_READ �̺�Ʈ�� �����ϰ� �߻����� �ʵ��� FD_READ�̺�Ʈ�� ����
	::WSAAsyncSelect(ah_socket, mh_notify_wnd, m_data_notify_id, FD_CLOSE);

	//�������� ����
	recv(ah_socket, (char *)&key, 1, 0);
	if (key == m_valid_key)
	{
		//msg_id ���� 1����Ʈ
		recv(ah_socket, (char *)&msg_id, 1, 0);
		//body_size ���� 2����Ʈ
		recv(ah_socket, (char *)&body_size, sizeof(BS), 0);
		//�ٵ� ����� 0�� ��쿡�� ������ �ʿ䰡 ����. ������ �����Ͱ� ����.
		if (body_size > 0) {
			// �ٵ� ������� ũ�� ������ recv�Լ��� �ƴ� �������� ��õ� ������ ������ �Լ��� ������ ����
			if (!ReceiveData(ah_socket, body_size)) {
				//�����͸� �����ϴٰ� ������ �߻��� ��� ����� ������ ����
				DisconnectSocket(ah_socket, -2);
				return;
			}
		}
		//���������� ���� �ٵ� ������ ��� ����ڰ� ���ϴ� �۾� ó��
		//ProcessRecvData �Լ��� ��ӹ��� �ڽ� Ŭ�������� �������Ͽ� ���ϴ� �۾� ó��
		if (1 == ProcessRecvData(ah_socket, msg_id, mp_recv_data, body_size)) {
			//���Ͽ� ������ �߻����� �ʾҴٸ� ���� �̺�Ʈ ó���� �����ϵ��� FD_READ �߰�
			::WSAAsyncSelect(ah_socket, mh_notify_wnd, m_data_notify_id, FD_CLOSE | FD_READ);
		}
	}
	else {
		//�������� �߸��� ��� ������ ����
		DisconnectSocket(ah_socket, -1);
	}
}

//�ڽ� Ŭ�������� �� �������ؼ� ����ؾ� ��, ������ Ŭ���̾�Ʈ�� �ٸ��� ����
void CG_Socket::DisconnectSocket(SOCKET ah_socket, int a_error_code)
{

}

// ���ŵ� �����͸� ó���ϴ� �Լ� - �ڽ� Ŭ�������� �� �������ؼ� ����ؾ� ��, ������ Ŭ���̾�Ʈ�� �ٸ��� ����
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
	//����ڸ�ŭ ������ ��ü ����
	for (int i = 0; i < m_max_user_count; i++) {
		delete mp_user_list[i];
	}
	//������ ����
	delete[] mp_user_list;
}

int CG_ServerSocket::StartServer(const wchar_t *ap_ip_address, int a_port, HWND ah_notify_wnd)
{
	mh_notify_wnd = ah_notify_wnd;
	struct sockaddr_in serv_addr;
	mh_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	//���� ���� üũ 0���� ������ ���� ���� ����
	if (mh_listen_socket < 0) return -1;

	char temp[16];
	UnicodeToAscii_IP(temp, (wchar_t *)ap_ip_address);
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(temp);
	serv_addr.sin_port = htons((short unsigned int)a_port);

	if (bind(mh_listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		// �����ϸ� ���� ����, �ʱ�ȭ
		closesocket(mh_listen_socket);
		mh_listen_socket = INVALID_SOCKET;
		return -2; // ���ε� ����
	}
	//Ŭ���̾�Ʈ ���� ���
	listen(mh_listen_socket, 5);
	//���ο� Ŭ���̾�Ʈ �����ϸ� FD_ACCEPT �̺�Ʈ �߻�
	//accept()�Լ��� �����ϸ� �����Ҷ����� ��� ��ٸ�, �񵿱� ��� ����
	WSAAsyncSelect(mh_listen_socket, ah_notify_wnd, m_accept_notify_id, FD_ACCEPT);
	
	return 1; // ����
}
int CG_ServerSocket::ProcessToAccept(WPARAM wParam, LPARAM lParam)
{
	struct sockaddr_in clnt_addr;
	int i, temp = sizeof(clnt_addr);
	//���� ���� �õ��ϴ� Ŭ���̾�Ʈ�� ����� ���� ����
	SOCKET h_client_socket = accept((SOCKET)wParam, (struct sockaddr *)&clnt_addr, &temp);
	if (h_client_socket == INVALID_SOCKET) return -1;
	else {
		CG_UserData *p_user;
		wchar_t temp_ip[16];
		AsciiToUnicode_IP(temp_ip, inet_ntoa(clnt_addr.sin_addr));
		for (i = 0; i < m_max_user_count; i++) {
			p_user = mp_user_list[i];
			//INVALID_SOCKET�̸� �̻�� ���� ��ü
			if (p_user->GetHandle() == INVALID_SOCKET) {
				p_user->SetHandle(h_client_socket);
				p_user->SetIP(temp_ip);
				//fd_read, fd_close�� �߻��ϸ� ���ν����� �޽��� �߻��ϵ���
				WSAAsyncSelect(h_client_socket, mh_notify_wnd, m_data_notify_id, FD_READ | FD_CLOSE);
				//���ο� ����� ���� ó��
				AddWorkForAccept(p_user);
				break;
			}
		}
		//���� �ڼ�
		m_user_count++;
		//���� �ʰ�
		if (i == m_max_user_count) {
			//���� �ʰ� ���� �޽���
			ShowLimitError(temp_ip);
			closesocket(h_client_socket);
			return -2; //����� �ʰ� ����
		}
	}
	return 1;// ���� ����
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
		// �����ڼ� 
		m_user_count--;
		// ���� ���� �� ó��
		AddWorkForCloseUser(p_data, 0);
		p_data->CloseSocket(0); // 0 : �׳� ����
	}
}

void CG_ServerSocket::DisconnectSocket(SOCKET ah_socket, int a_error_code)
{
	CG_UserData *p_user_data = FindUserData(ah_socket);
	// ���� ���� �� ó��
	AddWorkForCloseUser(p_user_data, a_error_code);
	p_user_data->CloseSocket(1); // 1 : LINGER ���� �� ����
}

int CG_ServerSocket::ProcessRecvData(SOCKET ah_socket, char a_msg_id, char *ap_recv_data, BS a_body_size)
{
	//�������� ������ ���������͸� ã��
	CG_UserData *p_user_data = FindUserData(ah_socket);

	//���� �޽��� 251���� Ŭ���̾�Ʈ�� ū�뷮�� �����͸� �����ϱ� ���� ���
	if (a_msg_id == 251) {
		char *p_send_data;
		//���� ���� ��ġ
		BS send_size = p_user_data->GetSendMan()->GetPosition(&p_send_data);
		//������ �����Ͱ� �� �ִٸ� ���� �޽��� ��ȣ 252�� ����Ͽ� Ŭ���̾�Ʈ���� ������ ����
		if (p_user_data->GetSendMan()->IsProcessing()) {
			SendFrameData(ah_socket, 252, p_send_data, send_size);
		}
		else {
			//���ҵ� �������� ������ �κ��̶�� ���� �޽��� ��ȣ 253�� ����Ͽ� Ŭ���̾�Ʈ���� ������ ����
			SendFrameData(ah_socket, 253, p_send_data, send_size);
			//������ �����͸� �����ϰ� ����ߴ� �޸� ����
			p_user_data->GetSendMan()->DeleteData();
			//������ ���ν����� ������ �Ϸ�Ǿ����� �˷��ش�
			//������ �Ϸ�Ǿ����� ���α׷��� � ǥ�ø� �ϰ� �ʹٸ� �ش� �����쿡��
			//LM_SEND_COMPLETED �޽����� üũ
			::PostMessage(mh_notify_wnd, LM_SEND_COMPLETED, (WPARAM)p_user_data, 0);
		}
	}
	//252���� ��뷮�� �����͸� ������ �� ����ϴ� ��ȣ
	else if (a_msg_id == 252) {
		//���ŵ� �����ʹ� ������ �����ϴ� ��ü�� �Ѱܼ� �����͸� ��ħ
		p_user_data->GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//252���� ������ �߰� ������ �����Ͱ� �ִٴ� ��, 251�� Ŭ���̾�Ʈ�� ����, �߰������� ��û
		SendFrameData(ah_socket, 251, NULL, 0);
	}
	//253�� ��뷮�� �����͸� �����Ҷ�
	else if (a_msg_id == 253) {
		//���ŵ� �����ʹ� ������ �����ϴ� ��ü�� �Ѱܼ� �����͸� ��ħ.
		p_user_data->GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//253�� ������ ������ �Ϸ�Ǿ��ٴ� �޽���, ���ν����� �˸�
		//LM_RECV_COMPLETED �޽����� ������ ���ν������� ����� �޸� DeleteData �Լ� ȣ��
		::PostMessage(mh_notify_wnd, LM_RECV_COMPLETED, (WPARAM)p_user_data, 0);
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CG_ClientSocket::CG_ClientSocket(char a_valid_key, int a_connect_notify_id, int a_data_notify_id) : CG_Socket(a_valid_key, a_data_notify_id)
{
	//���� ���¸� �������·� �ʱ�ȭ
	m_connect_flag = 0;
	//���� �ڵ� �ʱ�ȭ
	mh_socket = INVALID_SOCKET;
	//FD_CONNECT �̺�Ʈ �߻��� ����� ������ �޽��� ��ȣ
	m_connect_notify_id = a_connect_notify_id;
}

CG_ClientSocket::~CG_ClientSocket()
{
	//������ ����ϱ� ���� ������ �����Ǿ� �ִٸ� ���� ����
	if (mh_socket != INVALID_SOCKET) {
		closesocket(mh_socket);
	}
}

int CG_ClientSocket::ConnectToServer(const wchar_t *ap_ip_address, int a_port_num, HWND ah_notify_wnd)
{
	//������ �õ����̰ų� ���ӵ� ���¶�� ������ �õ����� �ʴ´�.
	if (m_connect_flag != 0) {
		return 0;
	}
	//���� �̺�Ʈ, ������ �޽��� ���� ������ �ڵ�
	mh_notify_wnd = ah_notify_wnd;
	//������ ����� ���� ����
	mh_socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serv_addr;
	char temp[16];
	UnicodeToAscii_IP(temp, (wchar_t *)ap_ip_address);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(temp);
	serv_addr.sin_port = htons(a_port_num);
	
	//������ �����ϴ� connect()�Լ��� ��������� ���� �� �־� �񵿱� ����
	//���� ���� ����� FD_CONNECT �̺�Ʈ�� �߻��ϸ� ���ν����� m_connect_notify_id�� �ش��ϴ� �޽��� ����
	WSAAsyncSelect(mh_socket, ah_notify_wnd, m_connect_notify_id, FD_CONNECT);
	//���� �õ�
	connect(mh_socket, (sockaddr *)&serv_addr, sizeof(serv_addr));
	// ���� ���¸� ���� '�õ���'���� ����
	m_connect_flag = 1;
	
	return 1;
}

int CG_ClientSocket::ResultOfConnection(LPARAM lParam)
{
	//���� ���� ���� == ������ ���������� �����
	if (WSAGETSELECTERROR(lParam) == 0) {
		//���� ���¸� '����'���� ����
		m_connect_flag = 2;
		//���ӵ� �������� �������� �����Ͱ� ���ŵǰų� ������ �����Ǿ����� ���ν����� �޽��� �߻�
		WSAAsyncSelect(mh_socket, mh_notify_wnd, m_data_notify_id, FD_READ | FD_CLOSE);
		//���� ����
		return 1;
	}
	else {
		//���� ����
		closesocket(mh_socket);
		//���� �ʱ�ȭ
		mh_socket = INVALID_SOCKET;
		//���ӻ��� '����'�� ����
		m_connect_flag = 0;
	}
	//���� ����
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
	//���� �޽��� 251���� Ŭ���̾�Ʈ�� ū�뷮�� �����͸� �����ϱ� ���� ���
	if (a_msg_id == 251) {
		char *p_send_data;
		//���� ���� ��ġ
		BS send_size = GetSendMan()->GetPosition(&p_send_data);
		//������ �����Ͱ� �� �ִٸ� ���� �޽��� ��ȣ 252�� ����Ͽ� Ŭ���̾�Ʈ���� ������ ����
		if (GetSendMan()->IsProcessing()) {
			CG_Socket::SendFrameData(ah_socket, 252, p_send_data, send_size);
		}
		else {
			//���ҵ� �������� ������ �κ��̶�� ���� �޽��� ��ȣ 253�� ����Ͽ� Ŭ���̾�Ʈ���� ������ ����
			CG_Socket::SendFrameData(ah_socket, 253, p_send_data, send_size);
			//������ �����͸� �����ϰ� ����ߴ� �޸� ����
			GetSendMan()->DeleteData();
			//������ ���ν����� ������ �Ϸ�Ǿ����� �˷��ش�
			//������ �Ϸ�Ǿ����� ���α׷��� � ǥ�ø� �ϰ� �ʹٸ� �ش� �����쿡��
			//LM_SEND_COMPLETED �޽����� üũ
			::PostMessage(mh_notify_wnd, LM_SEND_COMPLETED, 0, 0);
		}
	}
	//252���� ��뷮�� �����͸� ������ �� ����ϴ� ��ȣ
	else if (a_msg_id == 252) {
		//���ŵ� �����ʹ� ������ �����ϴ� ��ü�� �Ѱܼ� �����͸� ��ħ
		GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//252���� ������ �߰� ������ �����Ͱ� �ִٴ� ��, 251�� Ŭ���̾�Ʈ�� ����, �߰������� ��û
		CG_Socket::SendFrameData(ah_socket, 251, NULL, 0);
	}
	//253�� ��뷮�� �����͸� �����Ҷ�
	else if (a_msg_id == 253) {
		//���ŵ� �����ʹ� ������ �����ϴ� ��ü�� �Ѱܼ� �����͸� ��ħ.
		GetRecvMan()->AddData(ap_recv_data, a_body_size);
		//253�� ������ ������ �Ϸ�Ǿ��ٴ� �޽���, ���ν����� �˸�
		//LM_RECV_COMPLETED �޽����� ������ ���ν������� ����� �޸� DeleteData �Լ� ȣ��
		::PostMessage(mh_notify_wnd, LM_RECV_COMPLETED, 0, 0);
	}
	//���ŵ� �����͸� ���������� ó����, ����, ���� �����͸� ó���ϴ� �߿� ������ ���������� 0 ��ȯ
	//0�� ��ȯ�ϸ� �񵿱� �۾��� �ߴ�
	return 1;
}

int CG_ClientSocket::SendFrameData(char a_message_id, char *ap_body_data, BS a_body_size)
{
	return CG_Socket::SendFrameData(mh_socket, a_message_id, ap_body_data, a_body_size);
}
//////////////////////////////////////////////////////////////////////////
