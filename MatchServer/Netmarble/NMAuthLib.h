// NMAuthLib.h: interface for the CNMAuthLib class.
/**
@file	NMAuthLib.h
@brief	넷마블 인증 정적 라이브러리 헤더 파일.
@author	Heo Jaemin<judge@cj.net>
@date 	2009.10.28
*/
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NMAUTHLIB_H__493CC8A5_8FFB_4CA8_86DB_C2318A62FC52__INCLUDED_)
#define AFX_NMAUTHLIB_H__493CC8A5_8FFB_4CA8_86DB_C2318A62FC52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

// 넷마블 서버 인증 라이브러리(NMAuth) 자동추가.(Library 경로는 프로젝트 설정에 추가)
#if !defined(_LIB) && !defined(_NMAUTHDLL)
#ifndef _NMAUTHLIBNAME
#define _NMAUTHLIBNAME "./netmarble/NMAuthLib"
#endif //#ifndef _NMAUTHLIBNAME
#ifdef _DLL
#ifdef _DEBUG	// Debug multithread DLL
#pragma comment(lib, _NMAUTHLIBNAME "_MDd")
#else			// multithread DLL
#pragma comment(lib, _NMAUTHLIBNAME "_MD")
#endif // #ifdef _DEBUG
#else
#ifdef _DEBUG	// Debug multithread
#pragma comment(lib, _NMAUTHLIBNAME "_MTd")
#else			// multithread
#pragma comment(lib, _NMAUTHLIBNAME "_MT")
#endif // #ifdef _DEBUG
#endif // #ifdef _DLL
#endif //#if !defined(_LIB) && !defined(_NMAUTHDLL)

#define TIMEOUT_NMAUTH_DEFAULT 10000

/**
@defgroup GROUPERRCODE Error Code
@{
*/
/// @brief - 넷마블 인증 라이브러리 오류코드
enum eERROR_NMAUTH
{
	ERROR_NMAUTH_SUCCESS = 0,								///< 성공(오류 없음).
	ERROR_NMAUTH_FAIL,										///< 실패.
	ERROR_NMAUTH_INVALIDPARAM,								///< 잘못된 파라미터.
	ERROR_NMAUTH_OUTOFMEMEORY,								///< 메모리부족.
	ERROR_NMAUTH_FATALERROR,								///< 치명적 오류.
///	@details <b> ERROR_NMAUTH_LIB
	ERROR_NMAUTH_LIB_ALREADY_INITIALIZE = 11,				///< 이미 초기화되었음.
	ERROR_NMAUTH_LIB_DLL_CANT_LOAD,							///< DLL을 로드할 수 없음.
	ERROR_NMAUTH_LIB_DLL_NOT_LOADED,						///< DLL이 로드되지 않음.
	ERROR_NMAUTH_LIB_DLLPROC_NOTFOUND,						///< DLL 함수를 찾을 수 없음.
///	@details <b> ERROR_NMAUTH_DLL
	ERROR_NMAUTH_DLL_ALREADY_INITED = 101,					///< DLL이 이미 초기화되었음.
	ERROR_NMAUTH_DLL_AUTHWINDOW_CREATE_FAIL,				///< 인증 라이브러리 내부 윈도우 생성 실패.
///	@details <b> ERROR_NMAUTH_DLL_AUTHDATA
	ERROR_NMAUTH_DLL_AUTHDATA_NOT_INITED = 201,				///< 인증 데이터 Class가 초기화되지 않았음.
	ERROR_NMAUTH_DLL_AUTHDATA_DUPKEY,						///< 인증 데이터 Class의 Key가 중복됨.
	ERROR_NMAUTH_DLL_AUTHDATA_NOTFOUND,						///< 인증 데이터 Class를 찾을수 없음.
	ERROR_NMAUTH_DLL_AUTHDATA_ALREADY_INITED,				///< 인증 데이터 Class가 이미 초기화되었음.
	ERROR_NMAUTH_DLL_AUTHDATA_ALREADY_LOADING,				///< 인증 데이터 Class가 이미 데이터를 가져오고 있음.
	ERROR_NMAUTH_DLL_AUTHDATA_INVALID_XMLURL,				///< 잘못된 XML URL.
	ERROR_NMAUTH_DLL_AUTHDATA_XMLURL_ISNONE,				///< XML Url이 설정되지 않음.
	ERROR_NMAUTH_DLL_AUTHDATA_FAIL_INIT_EVTHANDLER,			///< 이벤트 핸들러를 초기화 할수 없음.
	ERROR_NMAUTH_DLL_AUTHDATA_FAIL_MAKE_REQHEADER,			///< XML Request 헤더를 생성하는데 실패함.
	ERROR_NMAUTH_DLL_AUTHDATA_FAIL_GET_SOCKET,				///< XML Socket을 획득하는데 실패함.
	ERROR_NMAUTH_DLL_AUTHDATA_FAIL_OPEN_SOCKET,				///< XML Socket을 여는데 실패함.
	ERROR_NMAUTH_DLL_AUTHDATA_FAIL_WAITFORSYNC,				///< 이벤트를 대기하는데 실패함.
	ERROR_NMAUTH_DLL_AUTHDATA_TIMEOUT_WAITFORSYNC,			///< 이벤트를 대기하는데 실패함.(타임아웃)
	ERROR_NMAUTH_DLL_AUTHDATA_MISSING_COOKIEDATA,			///< 쿠키데이터가 누락됨.
	ERROR_NMAUTH_DLL_AUTHDATA_MISSING_AUTHDATA,				///< 인증데이터가 누락됨.
///	@details <b> ERROR_NMAUTH_DLL_AUTHDATA_DECRYPT
	ERROR_NMAUTH_DLL_AUTHDATA_DECRYPT_INIT_FAIL = 301,		///< 복호화 모듈 초기화 실패.
	ERROR_NMAUTH_DLL_AUTHDATA_DECRYPT_FAIL,					///< 복호화 실패.
///	@details <b> ERROR_NMAUTH_DLL_AUTHDATA_XML
	ERROR_NMAUTH_DLL_AUTHDATA_XML_NOT_LOADED_DATA = 401,	///< 데이터가 로드되지 않음.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_FAIL_SENDREQUEST,			///< XML Request Header를 전송하는데 실패함.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_FAIL_ONSOCKDATA,			///< 소켓 데이터 처리시 오류가 발생함.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_FAIL_GETXMLCONTENTS,		///< XML Reponse 데이터를 가져오기 실패.
///	@details <b> ERROR_NMAUTH_DLL_AUTHDATA_XML_STATUSCODE
	ERROR_NMAUTH_DLL_AUTHDATA_XML_STATUSCODE_FAIL_GET = 411,///< XML Status Code를 가져올수 없음.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_STATUSCODE_INVALID,		///< 잘못된 XML Status Code.
///	@details <b> ERROR_NMAUTH_DLL_AUTHDATA_XML_COM
	ERROR_NMAUTH_DLL_AUTHDATA_XML_COM_FAIL_CREATE = 421,	///< XML Com 생성 실패.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_COM_FAIL_LOAD,			///< XML Com 데이터 로드 실패.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_COM_FAIL_GETROOTELEMENT,	///< XML Com Root Element 가져오기 실패.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_COM_INVALID_ROOTNAME,		///< XML Com Root Element의 이름이 유효하지 않음.
	ERROR_NMAUTH_DLL_AUTHDATA_XML_COM_CHILDNODE_IS_NONE,	///< XML Com Chile element가 존재하지 않음.
///	@details <b> ERROR_NMAUTH_DLL_AUTHDATA_XML_DATA
	ERROR_NMAUTH_DLL_AUTHDATA_XML_DATA_NOTFOUND = 431,		///< XML 데이터를 찾을 수 없음.
};

/// @brief -  넷마블 인증 라이브러리 오류코드 타입 정의
typedef unsigned int ERROR_NMAUTH;

/**
@}
*/

/**
@defgroup GROUPREADYSTATE ReadyState Code
@{
*/
/// @brief - 넷마블 인증 모듈 상태코드
enum eNMAUTHDATA_READYSTATE {
	eNMAUTHDATA_READYSTATE_UNKNOWN = 0,	///< 알수 없는 ReadyState 
	eNMAUTHDATA_READYSTATE_INITED,				///< 초기화됨.
	eNMAUTHDATA_READYSTATE_LOADING,         ///< 데이터 로딩중.
	eNMAUTHDATA_READYSTATE_ERROR,			///< 오류 발생.
	eNMAUTHDATA_READYSTATE_COMPLETE,     ///< 데이터 로드 완료.
};

/// @brief - 넷마블 인증 라이브러리 ReadyState 코드 타입 정리
typedef unsigned int NMAUTHDATA_READYSTATE;

/**
@brief - ReadyState 변화 알림용 함수 타입 정의
@param[in] eReadyState - 변화된 ReadyState
@param[in] pNMAuthLib - 변화가 발생한 인증 데이터 클래스.
@param[in] pUserData - 함수 설정시 함께 전달한 유저데이터.
*/
typedef void (_stdcall *PFN_NMAUTHDATA_ONREADYSTATECHANGE)(NMAUTHDATA_READYSTATE eReadyState, const void* pNMAuthClass, const void* pUserData);

/**
@}
*/


/// @brief - 넷마블 인증 라이브러리 API 함수 네임스페이스.
namespace NMAuthLib 
{
	/**
	@brif - 수동으로 넷마블 인증 동적 라이브러리(DLL) 파일을 설정한다.
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] szFileName - 설정할 인증 동적 라이브러리(DLL) 파일을 설정한다.
	@note 이 함수는 특정 경로나 파일을 로드해야할 필요가 있을 경우에만 호출한다.<br>
	이 함수가 호출되지 않을 경우 기본적으로 현재 디렉토리의 DLL을 로드하도록 설계 되어있다.	
	*/
	ERROR_NMAUTH _cdecl		SetCustomDLLFileName(const char* szFileName);

	/**
	@brif - 넷마블 인증 라이브러리 모듈을 초기화한다.
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] szGameCode - 현재 모듈을 사용하고 있는 게임의 넷마블 게임코드를 입력. <br>
				넷마블 게임코드란 넷마블 사이트에서 게임을 구분하기 위해서 발급하는 문자열로 구성된 코드.
	@param[in] bInteralGame - 내부/CP 게임 구분 파라미터. 내부게임일경우 true값을, CP게임 일 경우 false값을 입력한다.
	@note 특별한 이유가 없는 한 프로그램이 시작될때 한번만 호출되도록 하며, 해당 함수 호출시 윈도우가 생성되므로, <br>
	메시지 루프가 정상적으로 돌수 있도록 프로세스가 시작된 쓰레드에서 호출 하도록한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEINIT "모듈 초기화 및 파괴"
	*/
	ERROR_NMAUTH _cdecl		Init(const char* szGameCode, bool bInteralGame = true);
	
	/**
	@brif - 넷마블 인증 라이브러리 모듈을 초기화한다.
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] szGameCode - 현재 모듈을 사용하고 있는 게임의 넷마블 게임코드를 입력. <br>
				넷마블 게임코드란 넷마블 사이트에서 게임을 구분하기 위해서 발급하는 문자열로 구성된 코드.
	@param[in] szXMLBaseURL - 데이터를 가져올 XML의 기본 URL. 생략되거나 NULL 값을 입력할 경우 기본 값(CP 게임용)으로 처리됨.
	@note 특별한 이유가 없는 한 프로그램이 시작될때 한번만 호출되도록 하며, 해당 함수 호출시 윈도우가 생성되므로, <br>
	메시지 루프가 정상적으로 돌수 있도록 프로세스가 시작된 쓰레드에서 호출 하도록한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEINIT "모듈 초기화 및 파괴"
	*/
	ERROR_NMAUTH _cdecl		Initialize(const char* szGameCode, const char* szXMLBaseURL = NULL);

	/**
	@brif - 넷마블 인증 라이브러리 모듈을 파괴한다.
	@note 특별한 이유가 없는 한 프로그램이 종료될때 한번만 호출되도록 한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEINIT "모듈 초기화 및 파괴"
	*/
	void _cdecl				Destroy();

	/**
	@brif - 주어진 에러코드 값을 바탕으로 문자열 에러 코드명을 가져온다.
	@returns 주어진 에러 코드 값을 지칭하는 문자열 에러 코드명이 존재할 경우 해당 문자열 에러코드 명을 반환하고, 존재하지 않을 경우 NULL 값을 반환한다.
	@param[in] code - 문자열 에러 코드명을 가져올 에러코드 값.
	*/
	const char*	_cdecl		ErrorCode2String(ERROR_NMAUTH code);
	/**
	@brif - ReadyState 코드 값을 바탕으로 문자열 ReadyState 코드명을 가져온다.
	@returns 주어진 ReadyState 코드 값을 지칭하는 문자열 ReadyState 코드명이 존재할 경우 해당 문자열 ReadyState 코드 명을 반환하고, 존재하지 않을 경우 NULL 값을 반환한다.
	@param[in] code - 문자열 ReadyState 코드명을 가져올ReadyState 코드 값.
	*/
	const char*	_cdecl		ReadyState2String(NMAUTHDATA_READYSTATE code);
};

/**
@brief - 넷마블 XML 인증 데이터 클래스.
*/
class CNMXMLUserData {
public:
	/** @brif - CNMXMLUserData 생성자 */
	CNMXMLUserData();
	/** @brif - CNMXMLUserData 소멸자 */
	virtual ~CNMXMLUserData();

	/**
	@brif - 넷마블 XML 인증 데이터 클래스를 파괴한다.
	*/
	void			Destroy();

	/**
	@brif - 인증할 사용자의 SiteCode를 설정한다. (내부게임 채널링 전용)
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] iSiteCode - 인증할 사용자의 SiteCode
	*/
	ERROR_NMAUTH	SetSiteCode(int iSiteCode);

	/**
	@brif - XML로 부터 인증 데이터를 로드한다. (동기 방식)
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] dwTimeOutMillisecond - XML 로드 타임아웃값을 설정한다. 주어진 타임 아웃 이상으로 대기가 일어날 경우, 데이터 로드는 중단되고 오류를 반환한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEXMLDATALOAD "사용자 인증 - XML 데이터 로드"
	*/
	ERROR_NMAUTH	LoadDataFromXML(DWORD dwTimeOutMillisecond = TIMEOUT_NMAUTH_DEFAULT);
	/**
	@brif - XML로 부터 인증 데이터를 로드한다. (비동기 방식)
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] pfnOnReadyStateChange - XML 로드 ReadyState가 변경될때 호출될 함수 포인터를 입력한다.
	@param[in] pUserData - XML 로드 ReadyState가 변경될때 호출될때 전달 받을 UserData 를 입력한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEXMLDATALOAD "사용자 인증 - XML 데이터 로드"
	*/
	ERROR_NMAUTH	LoadDataFromXML(PFN_NMAUTHDATA_ONREADYSTATECHANGE pfnOnReadyStateChange, const void* pUserData);

	/**
	@brif - XML 호출시 웹서버에서 반환한 HTTP Status Code를 가져온다.
	@returns XML 호출시 웹서버에서 반환한 HTTP Status Code를 반환한다.
	*/
	DWORD			GetHTTPStatusCode();
	/**
	@brif - XML 호출시 웹서버에서 반환한 HTTP Contents를 가져온다.
	@returns XML 호출시 웹서버에서 반환한 HTTP Contents를 반환한다.
	*/
	const char*		GetHTTPContents();

	/**
	@brif - XML 인증 결과를 가져온다.
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[out] bAllow_Out - 로그인 허용 여부가 저장될 변수를 입력한다.
	@param[out] pdwErrorCode_Out - XML 결과 코드를 저장할 변수의 포인터 입력한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEXMLDATACHECK "사용자 인증 - XML 결과 코드 체크"
	*/
	ERROR_NMAUTH	GetXMLResult(BOOL& bAllow_Out, DWORD* pdwErrorCode_Out = NULL);

	/**
	@brif - XML 인증 결과 메시지를 반환한다. 
	@returns 문자열로 된 인증 결과 메시지를 반환한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEXMLDATACHECK "사용자 인증 - XML 결과 코드 체크"
	*/
	const char*		GetXMLResultMessage();

	/**
	@brif - 현재 Class에 저장된 데이터의 갯수를 반환한다.
	@returns 현재 Class에 저장된 데이터의 갯수를 반환한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEGETDATA "사용자 인증 - 사용자 정보 획득"
	*/
	DWORD			GetDataCount();
	/**
	@brif - 주어진 인덱스의 데이터의 이름을 반환한다.
	@returns 주어진 인덱스의 데이터의 이름을 반환한다.
	@param[in] dwIdx - 가져올 데이터의 인덱스를 입력한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEGETDATA "사용자 인증 - 사용자 정보 획득"
	*/
	const char* 	GetDataName(DWORD dwIdx);
	/**
	@brif - 주어진 이름의 데이터를 가져온다. 
	@returns 주어진 이름의 문자열 데이터를 반환한다.
	@param[in] szName - 가져올 데이터의 이름을 입력한다.
	@remark 주어진 이름의 데이터를 가져올때, 데이터 이름의 대소문자 비교는 무시된다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEGETDATA "사용자 인증 - 사용자 정보 획득"
	*/
	const char* 	GetData(const char* szName);
	/**
	@brif - 주어진 이름의 데이터를 버퍼에 복사한다. 
	@returns 성공할 경우 TRUE를 그렇지 않을 경우 FALSE를 반환한다.
	@param[in] szName - 복사할 데이터의 이름을 입력한다.
	@param[in] szBuffer - 복사할 데이터가 저장될 스트링 버퍼의 포인터를 입력한다.
	@param[in] cbBufferLen - 복사할 데이터가 저장될 버퍼의 길이를 입력한다.
	*/
	bool			CopyDataToString(const char* szName, char* pszBuffer, size_t cbBufferLen);
	/**
	@brif - 주어진 이름의 문자열 데이터를 숫자로 변환하여 long타입의 버퍼에 복사한다. 
	@returns 성공할 경우 TRUE를 그렇지 않을 경우 FALSE를 반환한다.
	@param[in] szName - 복사할 데이터의 이름을 입력한다.
	@param[in] szBuffer - 복사할 데이터가 저장될 버퍼의 포인터를 입력한다.
	*/
	bool			CopyDataToLong(const char* szName, long* pLongBuffer_Out);
	/**
	@brif - 현재 XML 로드 ReadyState를 반환한다.
	@returns 현재 XML 로드 ReadyState를 반환한다.
	*/
	NMAUTHDATA_READYSTATE	GetReadyState();
	/**
	@brif - XML 로드중에 가장 최근에 설정된 에러코드를 반환한다.
	@returns XML 로드중에 가장 최근에 설정된 에러코드를 반환한다.
	*/
	ERROR_NMAUTH			GetLastError();
};

//////////////////////////////////////////////////////////////////////////
// 넷마블 내부 게임 전용 클래스. (CP사 사용 불가)
/**
@brief - 넷마블 인증 클래스.
*/
class CNMAuth : public CNMXMLUserData {
public:
	/** @brif - CNMAuth 생성자 */
	CNMAuth();
	/** @brif - CNMAuth 소멸자 */
	virtual ~CNMAuth();

	/**
	@brif - 넷마블 인증 데이터 클래스를 초기화한다.(쿠키기반)
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@param[in] szAuthCookie - 인증을 확인할 사용자의 넷마블 AuthCookie 를 입력한다.
	@param[in] szDataCookie - 인증을 확인할 사용자의 넷마블 DataCookie 를 입력한다.
	@param[in] szCPCookie - 인증을 확인할 사용자의 넷마블 CPCookie 를 입력한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLEXMLDATALOAD "사용자 인증 - XML 데이터 로드"
	- @ref PAGEEXAMPLEGETDATA "사용자 인증 - 사용자 정보 획득"
	- @ref PAGEEXAMPLECKDATA "사용자 인증 - 쿠키값 직접 데이터 추출"
	*/
	ERROR_NMAUTH	Init(const char* szAuthCookie, const char* szDataCookie, const char* szCPCookie);
	/**
	@brif - 쿠키 값을 직접 복호화하여 인증 데이터를 로드한다.
	@returns 성공하면 @ref ERROR_NMAUTH_SUCCESS 값을 반환하고, 실패하였을 경우 ERROR_NMAUTH_SUCCESS 를 제외한 열거자 @ref ERROR_NMAUTH 중의 하나의 값을 반환한다.
	@section SECEXAMPLE 관련 예제 
	- @ref PAGEEXAMPLECKDATA "사용자 인증 - 쿠키값 직접 데이터 추출"
	*/
	ERROR_NMAUTH	LoadDataFromCookie();
};

#endif // !defined(AFX_NMAUTHLIB_H__493CC8A5_8FFB_4CA8_86DB_C2318A62FC52__INCLUDED_)
