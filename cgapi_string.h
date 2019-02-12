#pragma once
#ifndef _CGAPI_STRING_H_
#define _CGAPI_STRING_H_
//���� TW_String
// CG_String Ŭ������ ���ڿ��� ���ϰ� ������ �� �ֵ��� ���� Ŭ�����Դϴ�. �� Ŭ������ ���ڿ� ó���� ���� ������ �ϱ� 
// ���ؼ� ���ڿ��� ���̸� ���� ����ϴ� ��찡 ������ �� �� ����ϴ� ���ڿ��� ���̴� ���� ���ڿ��� ���̸� �ǹ��մϴ�. 
// ���� ���, �� ��ü�� "abc" ���ڿ��� ������ ������ ���ڿ��� ���̰� ����� m_length���� 3�� ���� �˴ϴ�. �׸��� ���ڿ�
// a_length�� ����Ҷ��� �� ��Ģ�� �����ϰ� ����˴ϴ�.
class CG_String
{
protected:
	wchar_t *mp_string;   // �� ��ü�� �����ϴ� ���ڿ��� �ּ�
	int m_length;         // �� ��ü�� �����ϴ� ���ڿ��� ���� ("abc"�̸� 3�� ����)

public:
	CG_String();  // �⺻ ������
	CG_String(const char *ap_string);
	CG_String(const wchar_t *ap_string);
	CG_String(const CG_String &ar_string);  // ���� ������

	virtual ~CG_String();

	void InitialObject(const wchar_t *ap_string);  // ��ü �ʱ�ȭ �Լ�
	inline int GetLength() { return m_length; }   // ���ڿ��� ���̸� �˷��ִ� �Լ�
	inline const wchar_t *GetString() { return mp_string; } // ���ڿ��� ������ �˷��ִ� �Լ�
	void RemoveString();                           // �� ��ü�� �����ϴ� ���ڿ��� �����Ѵ�.

												   // ���޵� ���ڿ��� �����ؼ� �����Ѵ�.
	void SetString(const wchar_t *ap_string);
	// ���޵� ���ڿ��� �ּҸ� �״�� ����Ѵ�. (���縦 �����ʰ� ������ �״�� �����)
	void AttachString(wchar_t *ap_string, int a_length);
	// ����� ���ڿ��� �ٸ� ���ڿ��� �߰��ϴ� ��쿡 ����Ѵ�.
	// ���� ���, "abc"�� ����Ǿ� �ִµ� AddString("def", 3); �̶�� ����ϸ� "abcdef"�� �˴ϴ�.
	void AddString(const wchar_t *ap_string, int a_length);

	// ���޵� �ƽ�Ű ���ڿ��� �����ڵ�� �����Ͽ� Ŭ���� ���ο� �����Ѵ�. ��ȯ���� ��ȯ�� ������ �����̴� (wcslen ���� ��ġ�Ѵ� )
	int AsciiToUnicode(char *ap_src_string);
	//// ���޵� �ƽ�Ű ���ڿ��� �����ڵ�� �����Ͽ� ap_dst_string�� ��ȯ�Ѵ�. ��ȯ���� ��ȯ�� ������ �����̴� (wcslen ���� ��ġ�Ѵ� )
	//// ap_dest_string �� NULL�� ����ϸ� ��ȯ�������� ���ڿ� ���̸� �˾Ƴ��� �ִ�.
	//static int AsciiToUnicode(char *ap_src_string, wchar_t *ap_dest_string);
	//// Ŭ������ �����ϴ� ���ڿ��� UTF8 �������� �����Ͽ� ap_dest_string�� ��ȯ�Ѵ�. ��ȯ���� UTF8 ���ڿ��� ���� �޸� ũ���̴�.
	//// ap_dest_string �� NULL�� ����ϸ� ��ȯ�������� ���ڿ� ���̸� �˾Ƴ��� �ִ�.
	//int UnicodeToUTF8(char *ap_dest_string);
	//// ap_src_string ���ڿ��� UTF8 �������� �����Ͽ� ap_dest_string�� ��ȯ�Ѵ�. ��ȯ���� UTF8 ���ڿ��� ���� �޸� ũ���̴�.
	//// ap_dest_string �� NULL�� ����ϸ� ��ȯ�������� ���ڿ� ���̸� �˾Ƴ��� �ִ�.
	//static int UnicodeToUTF8(wchar_t *ap_src_string, char *ap_dest_string);
	//// Ŭ������ �����ϴ� ���ڿ��� �ƽ�Ű�ڵ�� �����Ͽ� ap_dst_string�� ��ȯ�Ѵ�. ��ȯ���� ��ȯ�� ������ �����̴� (strlen ���� ��ġ�Ѵ� )
	//// ap_dest_string �� NULL�� ����ϸ� ��ȯ�������� ���ڿ� ���̸� �˾Ƴ��� �ִ�.
	//int UnicodeToAscii(char *ap_dest_string);
	//// ���޵� �����ڵ� ���ڿ��� �ƽ�Ű�ڵ�� �����Ͽ� ap_dst_string�� ��ȯ�Ѵ�. ��ȯ���� ��ȯ�� ������ �����̴� (strlen ���� ��ġ�Ѵ� )
	//// ap_dest_string �� NULL�� ����ϸ� ��ȯ�������� ���ڿ� ���̸� �˾Ƴ��� �ִ�.
	//static int UnicodeToAscii(wchar_t *ap_src_string, char *ap_dest_string);

	// printf�� ���� ������ ����ϰ� ���� �� ����ϴ� �Լ�. �� �Լ��� ����� �ش��ϴ� ���ڿ��� ȭ�鿡 ��µ��� �ʰ�
	// �� ��ü�� ���ڿ�(mp_string)�� ����˴ϴ�.
	int Format(const wchar_t *ap_format, ...);

	// ���� ������ �����ε� str = L"test";
	void operator=(const CG_String& ar_string);
	// ���ڿ� Ȯ��(append)�� ���� ������ �����ε�
	void operator+=(const CG_String& ar_str);
	// ���ڿ��� ���ϱ� ���� ������ �����ε�
	friend CG_String operator+(const CG_String& ar_str1, const CG_String& ar_str2)
	{
		CG_String str = ar_str1;
		str.AddString(ar_str2.mp_string, ar_str2.m_length);
		return str;
	}
	// ���ڿ� �񱳸� ���� ������ �����ε� (������ Ȯ���ϱ� ���� ������)
	friend char operator==(const CG_String& ar_str1, const CG_String& ar_str2)
	{
		if (ar_str1.m_length != ar_str2.m_length) return 0;
		return memcmp(ar_str1.mp_string, ar_str2.mp_string, ar_str1.m_length << 1) == 0;
	}
	// ���ڿ� �񱳸� ���� ������ �����ε� (�ٸ��� Ȯ���ϱ� ���� ������)
	friend char operator!=(const CG_String& ar_str1, const CG_String& ar_str2)
	{
		if (ar_str1.m_length != ar_str2.m_length) return 1;
		return memcmp(ar_str1.mp_string, ar_str2.mp_string, ar_str1.m_length << 1) != 0;
	}

	friend char operator!=(const CG_String& ar_str1, const wchar_t *ap_str2)
	{
		if (ar_str1.m_length != wcslen(ap_str2)) return 1;
		return memcmp(ar_str1.mp_string, ap_str2, ar_str1.m_length << 1) != 0;
	}

	operator LPCWSTR() const
	{
		return mp_string;
	}

	operator LPWSTR()
	{
		return mp_string;
	}
};



#endif