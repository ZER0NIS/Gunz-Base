/************************************************************************
  MChattingFilter

  desc : 입력한 스트링의 유효 여부를 판단하는 클래스
  date : 2008-02-14
  name : 임동환
*************************************************************************/


#pragma once

#include <list>
#include <string>

using namespace std;



/// (class) MAbuseWord
class MAbuseWord
{
/// 멤버 변수 선언
private:
	/// 필터링 레벨
	unsigned int nFilteringLevel;

	/// 금칙단어
	string strAbuseWord;



/// 멤버 함수 선언
public:
	/// 표준 생성자
	MAbuseWord()
	{
		nFilteringLevel = 0;
		strAbuseWord = "";
	}

	/// 생성자
	MAbuseWord( int nLevel, const char* szWord)
	{
		nFilteringLevel = nLevel;
		strAbuseWord = szWord;
	}


	/// 필터링 레벨을 구함
	unsigned int GetFilteringLevel()	{ return nFilteringLevel;			}

	/// 금칙단어를 구함
	const char* GetAbuseWord()			{ return strAbuseWord.c_str();		}
};




/// (class) MChattingFilter
class MChattingFilter
{
/// 멤버 변수 선언
private :
	/// 금칙단어 리스트
	list<MAbuseWord*>	m_AbuseWordList;

	/// 마지막으로 필터링된 금칙단어
	string				m_strLastFilterdWord;



/// 멤버 함수 선언
public:
	/// 표준 생성자
	MChattingFilter();

	/// 표준 소멸자
	~MChattingFilter();


	/// 외부 파일로부터 금칙단어 리스트를 읽어들인다
	bool LoadFromFile( MZFileSystem* pfs, const char* szFileName);

	/// 로딩된 금칙단어가 몇개인지 반환한다
	int GetNumAbuseWords();

private:
	void GetLine( char*& prfBuf, char* szLevel, char* szText);
	void SkipBlock( char*& prfBuf);


protected:
	/// 스트링을 소문자화하고 특수문자를 제거한다.
	bool PreTranslateStr( const string& strInText, string& strOutText);
	

public:
	/// 입력한 스트링의 유효 여부를 구함
	virtual bool IsValidStr( const char* szString, unsigned int nLevel, bool bCheckSpcChar =false);


	/// 마지막으로 필터링된 금칙단어를 구한다
	const char* GetLastFilteredStr();


	/// 인스턴스를 구한다
	static MChattingFilter* GetInstance();
	
	///
	void Clear();
};



/// 인스턴스를 구함
inline MChattingFilter* MGetChattingFilter()
{
	return MChattingFilter::GetInstance();
}


