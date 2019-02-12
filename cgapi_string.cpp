#include "stdafx.h"
#include "cgapi_string.h"

CG_String::CG_String()
{
	InitialObject(NULL);
}

CG_String::CG_String(const wchar_t *ap_string)
{
	InitialObject(ap_string);
}

CG_String::CG_String(const char *ap_string)
{
	mp_string = NULL;
	AsciiToUnicode((char *)ap_string);
}

CG_String::CG_String(const CG_String &ar_string)
{
	InitialObject(ar_string.mp_string);
}

void CG_String::InitialObject(const wchar_t *ap_string)
{
	if (ap_string == NULL) {
		m_length = 0;
		mp_string = new wchar_t[1];
		mp_string[0] = 0;
	} else {
		m_length = wcslen(ap_string) + 1;
		mp_string = new wchar_t[m_length];
		memcpy(mp_string, ap_string, m_length << 1);
	}
}

CG_String::~CG_String()
{
	delete[] mp_string;
}

void CG_String::RemoveString()
{
	delete[] mp_string;
}

void CG_String::SetString(const wchar_t *ap_string)
{
	delete[] mp_string;
	InitialObject(ap_string);
}

void CG_String::AttachString(wchar_t *ap_string, int a_length)
{
	delete[] mp_string;
	m_length = a_length;
	mp_string = ap_string;
}

void CG_String::AddString(const wchar_t *ap_string, int a_length)
{
	int total_length = m_length + a_length;
	wchar_t *p_string = new wchar_t[total_length + 1];
	memcpy(p_string, mp_string, m_length << 1);
	memcpy(p_string + m_length, ap_string, (a_length + 1) << 1);
	delete[] mp_string;
	mp_string = p_string;
	m_length = total_length;
}

int CG_String::AsciiToUnicode(char *ap_src_string)
{
	m_length = strlen(ap_src_string) + 1;
	mp_string = new wchar_t[m_length];
	memset(mp_string, 0, m_length << 1);
	for (int i = 0; i < m_length; i++) {
		mp_string[i] = ap_src_string[i];
	}
	return m_length - 1;
}

void CG_String::operator=(const CG_String& ar_string)
{
	delete[] mp_string;
	m_length = ar_string.m_length + 1;
	mp_string = new wchar_t[m_length];
	memcpy(mp_string, ar_string.mp_string, m_length << 1);
}

void CG_String::operator+=(const CG_String& ar_string)
{
	AddString(ar_string.mp_string, ar_string.m_length);
}