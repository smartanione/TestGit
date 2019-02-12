#pragma once
#ifndef _CGAPI_STRING_H_
#define _CGAPI_STRING_H_
//원작 TW_String
// CG_String 클래스는 문자열을 편리하게 관리할 수 있도록 만든 클래스입니다. 이 클래스는 문자열 처리를 좀더 빠르게 하기 
// 위해서 문자열의 길이를 같이 사용하는 경우가 많은데 이 때 사용하는 문자열의 길이는 순수 문자열의 길이를 의미합니다. 
// 예를 들어, 이 객체가 "abc" 문자열을 가지고 있으면 문자열의 길이가 저장된 m_length에는 3이 저장 됩니다. 그리고 인자에
// a_length를 사용할때도 이 규칙은 동일하게 적용됩니다.
class CG_String
{
protected:
	wchar_t *mp_string;   // 이 객체가 관리하는 문자열의 주소
	int m_length;         // 이 객체가 관리하는 문자열의 길이 ("abc"이면 3이 저장)

public:
	CG_String();  // 기본 생성자
	CG_String(const char *ap_string);
	CG_String(const wchar_t *ap_string);
	CG_String(const CG_String &ar_string);  // 복사 생성자

	virtual ~CG_String();

	void InitialObject(const wchar_t *ap_string);  // 객체 초기화 함수
	inline int GetLength() { return m_length; }   // 문자열의 길이를 알려주는 함수
	inline const wchar_t *GetString() { return mp_string; } // 문자열의 내용을 알려주는 함수
	void RemoveString();                           // 이 객체가 관리하던 문자열을 제거한다.

												   // 전달된 문자열을 복사해서 관리한다.
	void SetString(const wchar_t *ap_string);
	// 전달된 문자열의 주소를 그대로 사용한다. (복사를 하지않고 원본을 그대로 사용함)
	void AttachString(wchar_t *ap_string, int a_length);
	// 저장된 문자열에 다른 문자열을 추가하는 경우에 사용한다.
	// 예를 들어, "abc"가 저장되어 있는데 AddString("def", 3); 이라고 사용하면 "abcdef"가 됩니다.
	void AddString(const wchar_t *ap_string, int a_length);

	// 전달된 아스키 문자열을 유니코드로 변경하여 클래스 내부에 저장한다. 반환값은 변환된 문자의 갯수이다 (wcslen 값과 일치한다 )
	int AsciiToUnicode(char *ap_src_string);
	//// 전달된 아스키 문자열을 유니코드로 변경하여 ap_dst_string에 반환한다. 반환값은 변환된 문자의 갯수이다 (wcslen 값과 일치한다 )
	//// ap_dest_string 에 NULL을 명시하면 변환했을때의 문자열 길이만 알아낼수 있다.
	//static int AsciiToUnicode(char *ap_src_string, wchar_t *ap_dest_string);
	//// 클래스가 관리하는 문자열을 UTF8 형식으로 변경하여 ap_dest_string에 반환한다. 반환값은 UTF8 문자열에 사용된 메모리 크기이다.
	//// ap_dest_string 에 NULL을 명시하면 변환했을때의 문자열 길이만 알아낼수 있다.
	//int UnicodeToUTF8(char *ap_dest_string);
	//// ap_src_string 문자열을 UTF8 형식으로 변경하여 ap_dest_string에 반환한다. 반환값은 UTF8 문자열에 사용된 메모리 크기이다.
	//// ap_dest_string 에 NULL을 명시하면 변환했을때의 문자열 길이만 알아낼수 있다.
	//static int UnicodeToUTF8(wchar_t *ap_src_string, char *ap_dest_string);
	//// 클래스가 관리하는 문자열을 아스키코드로 변경하여 ap_dst_string에 반환한다. 반환값은 변환된 문자의 갯수이다 (strlen 값과 일치한다 )
	//// ap_dest_string 에 NULL을 명시하면 변환했을때의 문자열 길이만 알아낼수 있다.
	//int UnicodeToAscii(char *ap_dest_string);
	//// 전달된 유니코드 문자열을 아스키코드로 변경하여 ap_dst_string에 반환한다. 반환값은 변환된 문자의 갯수이다 (strlen 값과 일치한다 )
	//// ap_dest_string 에 NULL을 명시하면 변환했을때의 문자열 길이만 알아낼수 있다.
	//static int UnicodeToAscii(wchar_t *ap_src_string, char *ap_dest_string);

	// printf와 같은 형식을 사용하고 싶을 때 사용하는 함수. 이 함수의 결과에 해당하는 문자열은 화면에 출력되지 않고
	// 이 객체의 문자열(mp_string)에 저장됩니다.
	int Format(const wchar_t *ap_format, ...);

	// 대입 연산자 오버로딩 str = L"test";
	void operator=(const CG_String& ar_string);
	// 문자열 확장(append)를 위한 연산자 오버로딩
	void operator+=(const CG_String& ar_str);
	// 문자열을 더하기 위한 연산자 오버로딩
	friend CG_String operator+(const CG_String& ar_str1, const CG_String& ar_str2)
	{
		CG_String str = ar_str1;
		str.AddString(ar_str2.mp_string, ar_str2.m_length);
		return str;
	}
	// 문자열 비교를 위한 연산자 오버로딩 (같은지 확인하기 위한 연산자)
	friend char operator==(const CG_String& ar_str1, const CG_String& ar_str2)
	{
		if (ar_str1.m_length != ar_str2.m_length) return 0;
		return memcmp(ar_str1.mp_string, ar_str2.mp_string, ar_str1.m_length << 1) == 0;
	}
	// 문자열 비교를 위한 연산자 오버로딩 (다른지 확인하기 위한 연산자)
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