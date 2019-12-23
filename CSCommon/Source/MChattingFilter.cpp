/************************************************************************
MChattingFilter

desc : 입력한 스트링의 유효 여부를 판단하는 클래스
date : 2008-02-14
name : 임동환
*************************************************************************/

#include "stdafx.h"
#include "MChattingFilter.h"
#include "MZFileSystem.h"



/************************************************************************
  MChattingFilter

  desc : 생성자
*************************************************************************/
MChattingFilter::MChattingFilter()
{
	m_strLastFilterdWord = "";
}


/************************************************************************
  ~MChattingFilter

  desc : 소멸자
*************************************************************************/
MChattingFilter::~MChattingFilter()
{
	Clear();
}

void MChattingFilter::Clear()
{
	// 금칙단어 리스트 삭제
	while ( m_AbuseWordList.empty() == false)
	{
		delete *m_AbuseWordList.begin();
		m_AbuseWordList.pop_front();
	}
}


/************************************************************************
  LoadFromFile

  desc : 외부 파일로부터 금칙단어 리스트를 읽어들인다
  arg  : pfs = 시스템 파일 포인트
         szFileName = 파일 이름
  ret  : true(success) or false(fail)
*************************************************************************/
bool MChattingFilter::LoadFromFile( MZFileSystem* pfs, const char* szFileName)
{
	if ( szFileName == 0)
		return false;


	// 파일 열기
	MZFile mzf;
	if ( !mzf.Open( szFileName, pfs)) 
		return false;


	// 파일 내용 읽어들이기
	char *buffer;
	char* tembuf;
	buffer = new char[ mzf.GetLength() + 1];
	mzf.Read( buffer, mzf.GetLength());
	buffer[ mzf.GetLength()] = 0;
	tembuf = buffer;


	// 금칙단어 리스트 삭제
	while ( m_AbuseWordList.empty() == false)
	{
		delete *m_AbuseWordList.begin();
		m_AbuseWordList.pop_front();
	}

#ifdef _DEBUG
	FILE* fp = fopen( "abuse_db.txt", "w" );
#endif


	// 금칙단어 검색
	while ( 1)
	{
		char szLevel[ 256];
		char szText[ 256];

		GetLine( tembuf, szLevel, szText);


		if ( (int)strlen( szLevel) == 0)
			continue;

		if ( _stricmp( szLevel, "END") == 0)
			break;


		MAbuseWord* pAbuseWord = new MAbuseWord( atoi( szLevel), szText);
		m_AbuseWordList.push_back( pAbuseWord);


#ifdef _DEBUG
		fprintf( fp, "insert into abuselist(workd) values('%%%s%%')\n"
			, szText );
#endif


		SkipBlock( tembuf);
	}

#ifdef _DEBUG
	fclose( fp );
#endif


	mzf.Close();
	tembuf = 0;
	delete [] buffer;


	return true;
}


void MChattingFilter::GetLine( char*& prfBuf, char* szLevel, char* szText)
{
	bool bType = true;
	int  nTypeCount = 0;
	int  nTextCount = 0;

	*szLevel = 0;
	*szText = 0;

	while ( 1)
	{
		char ch = *prfBuf++;

		if ( (ch == 0) || (ch == '\n') || (ch == '\r'))
			break;
			

		if ( ch == ',')
		{
			bType = false;

			continue;
		}


		if ( bType)
		{
			*(szLevel + nTypeCount++) = ch;
			*(szLevel + nTypeCount) = 0;
		}
		else
		{
			*(szText + nTextCount++) = ch;
			*(szText + nTextCount) = 0;
		}
	}
}


void MChattingFilter::SkipBlock( char*& prfBuf)
{
	for ( ; *prfBuf != '\n'; ++prfBuf )
		NULL;
		
	++prfBuf;
}


/************************************************************************
  PreTranslateStr

  desc : 스트링을 소문자화하고 특수문자를 제거한다.
  arg  : strInText = 처리할 스트링
         strOutText = 처리된 스트링
  ret  : true = 허용되지 않는 특수문자를 포함하고 있음
         false = 비허용 특수문자 없음
*************************************************************************/
bool MChattingFilter::PreTranslateStr( const string& strInText, string& strOutText)
{	
	char *pStrLowercase;
//	_strlwr_s( pStrLowercase = _strdup( strInText.c_str()), 512);			// 보안용
//	pStrLowercase = _strlwr( _strdup( strInText.c_str()));					// 비보안용
	pStrLowercase = _strdup( strInText.c_str());
	strOutText = pStrLowercase;

	// 캐릭터 검사
	int nPos = 0;					// 스트링 검사 위치
	bool bHaveSpcChar = false;		// 비허용 특수문자 포함 여부
	char ch;
	unsigned char u_ch;


	bool bAllowDoubleByteChar = true;
	// 미국에서 2바이트 문자를 이용해 캐릭명에 빈칸을 넣는 방법이 유행해서 아예 미국에선 2바이트 문자를 허용하지 않음
	// 이로써 미국에서는 캐릭명과 채팅에 2바이트 문자를 전혀 사용할 수 없게 됨.
#ifdef LOCALE_NHNUSAA
	bAllowDoubleByteChar = false;
#endif

	while ( 1)
	{
		// 사이즈를 벗어나면 종료
		if ( nPos >= (int)strOutText.size())
			break;

		// 현재 위치의 캐릭터를 구함
		ch = strOutText.at( nPos);
		u_ch = ch;


		// 현재 캐릭터가 2바이트 문자의 첫 바이트이면 다음 바이트까지의 검사를 패스
		if ( bAllowDoubleByteChar && IsDBCSLeadByte( ch))
		{
#ifdef LOCALE_JAPAN   // 일본 전각 문자의 경우 특수문자도 전부 2바이트다. 
			if ( ch == -127 && strOutText.at( nPos +1) == 64) // 일단 스페이스만 막음 
			{
				bHaveSpcChar = true;
				m_strLastFilterdWord = strOutText.substr(nPos, 2);;
			}
#endif
			nPos += 2;
		}

		// 일반 ASCII 문자인지 확인
#ifdef LOCALE_JAPAN   
		else if ( ( (ch >= 'a') && (ch <= 'z') ) ||	
				  ( (ch >= 'A') && (ch <= 'Z') ) ||
				  ( (ch >= '0') && (ch <= '9') ) ||
				  ( (u_ch >= 0xA1 ) && (u_ch <= 0xDF ) ) )  // 일본 반각 문자인지 확인
#else 
		else if ( ( (ch >= 'a') && (ch <= 'z') ) ||	
			( (ch >= 'A') && (ch <= 'Z') ) ||
			( (ch >= '0') && (ch <= '9') )  )  
#endif
		{
			if( ( (ch >= 'A') && (ch <= 'Z') ) )  // 소문자 변환
			{
				strOutText[nPos] += 0x20;
			}
			nPos += 1;
		}
		else
		{
			// 삭제하기 전에 해당 캐릭터가 비허용 특수문자인지 검사
			if ( (ch != '_') && (ch != '[') && (ch != ']') )
			{
				bHaveSpcChar = true;

				m_strLastFilterdWord = ch;
			}

			// 캐릭터 삭제
			strOutText.erase( nPos, 1);
		}
	}

	free( pStrLowercase);

	return bHaveSpcChar;
}




/************************************************************************
  IsValidStr

  desc : 입력한 스트링의 유효 여부를 구함
  arg  : szString = 유효 여부를 판단할 스트링
         nFilteringLevel = 필터링 레벨
		 bCheckSpcChar = 비허용 특수문자 검사 여부
  ret  : true(valid) or false(invalid)
*************************************************************************/
bool MChattingFilter::IsValidStr( const char* szString, unsigned int nFilteringLevel, bool bCheckSpcChar)
{
	int i, j;
	int len1, len2;


	if ( szString == 0)
		return false;

	// 특수 문자를 삭제한다.
	string str;
	bool bHaveSpcChar = PreTranslateStr( szString, str);

	// 비허용 특수문자가 있는지 검사
	if ( (bCheckSpcChar == true) && (bHaveSpcChar == true) )
		return false;

	// 금칙어가 있는지 조사한다.
	for ( list<MAbuseWord*>::iterator itr = m_AbuseWordList.begin();  itr != m_AbuseWordList.end();  itr++)
	{
		MAbuseWord* pAbuseWord = (*itr);


		// 레벨 검사
		if ( nFilteringLevel < pAbuseWord->GetFilteringLevel())
			continue;


		// 금칙단어 검사
		len2 = (int)strlen( pAbuseWord->GetAbuseWord());
		len1 = (int)str.length() - len2 + 1;

		for ( i = 0;  i < len1;  i++)
		{
			for ( j = 0;  j < len2;  j++)
			{
				if ( str.at( i + j) != pAbuseWord->GetAbuseWord()[ j])
					break;
			}


			if ( j == len2)
			{
				m_strLastFilterdWord = pAbuseWord->GetAbuseWord();

				return false;
			}


			if ( IsDBCSLeadByte( str.at( i)) == TRUE)
				i++;
		}
	}

	return true;
}


/************************************************************************
  GetLastFilteredStr

  desc : 마지막으로 필터링된 금칙단어를 구한다
  arg  : none
  ret  : filtered string
*************************************************************************/
const char* MChattingFilter::GetLastFilteredStr()
{
	return m_strLastFilterdWord.c_str();
}


/************************************************************************
  GetInstance

  desc : 인스턴스를 구한다
  arg  : none
  ret  : none
*************************************************************************/
MChattingFilter* MChattingFilter::GetInstance()
{
	static MChattingFilter ChattingFilter;
	return &ChattingFilter;
}


/************************************************************************
	GetNumAbuseWords

	desc : 로딩된 금칙단어가 몇개인지 반환한다
	arg  : none
	ret  : none
*************************************************************************/
int MChattingFilter::GetNumAbuseWords()
{
	return (int)m_AbuseWordList.size();
}