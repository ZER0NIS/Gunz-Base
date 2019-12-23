#ifndef _MMATCHDBBASEGAMBLEITEM_H
#define _MMATCHDBBASEGAMBLEITEM_H


#include <string>

using std::string;


// 서버와 클라이언트가 공통으로 사용하는 데이터를 모아놓은 최소 클래스.
class MMatchDBBaseGambleItem
{
protected :
	DWORD	m_dwGambleItemID;
	string	m_strName;
	string	m_strDescription;
	DWORD	m_dwPrice;
	bool	m_bIsCash;

protected :
	MMatchDBBaseGambleItem() {}

public :
	MMatchDBBaseGambleItem( const DWORD dwGIID
		, const string& strName
		, const string& strDesc
		, const DWORD dwPrice
		, const bool bIsCash )
	{
		m_dwGambleItemID	= dwGIID;
		m_strName			= strName;
		m_strDescription	= strDesc;
		m_dwPrice			= dwPrice;
		m_bIsCash			= bIsCash;
	}

	~MMatchDBBaseGambleItem() {}

	const DWORD		GetGambleItemID() const	{ return m_dwGambleItemID; }
	const string&	GetName() const			{ return m_strName; }
	const string&	GetDescription() const	{ return m_strDescription; }
	const DWORD		GetPrice() const		{ return m_dwPrice; }
	const bool		IsCash() const			{ return m_bIsCash; }
};


#endif