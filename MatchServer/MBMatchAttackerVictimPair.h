#ifndef _MBMATCHATTACKERVICTIMPAIR_H
#define _MBMATCHATTACKERVICTIMPAIR_H


class MBMatchAttVicPair
{
private :
	DWORD m_dwAttackerCID;
	DWORD m_dwVictimCID;

public :
	MBMatchAttVicPair( const DWORD dwAttackerCID, const DWORD dwVictimCID )
	{
		m_dwAttackerCID = dwAttackerCID;
		m_dwVictimCID	= dwVictimCID;
	}

	const DWORD GetAttackerCID() const	{ return m_dwAttackerCID; }
	const DWORD GetVictimCID() const	{ return m_dwVictimCID; }

	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator > (const MBMatchAttVicPair& a, const MBMatchAttVicPair& b){
		if(a.GetAttackerCID() > b.GetAttackerCID()) return true;
		if(a.GetAttackerCID() == b.GetAttackerCID()){
			if(a.GetVictimCID() > b.GetVictimCID()) return true;
		}
		return false;
	}
	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator >= (const MBMatchAttVicPair& a, const MBMatchAttVicPair& b){
		if(a.GetAttackerCID() > b.GetAttackerCID()) return true;
		if(a.GetAttackerCID() == b.GetAttackerCID()){
			if(a.GetVictimCID() >= b.GetVictimCID()) return true;
		}
		return false;
	}
	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator < (const MBMatchAttVicPair& a, const MBMatchAttVicPair& b){
		if(a.GetAttackerCID() < b.GetAttackerCID()) return true;
		if(a.GetAttackerCID() == b.GetAttackerCID()){
			if(a.GetVictimCID() < b.GetVictimCID()) return true;
		}
		return false;
	}
	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator <= (const MBMatchAttVicPair& a, const MBMatchAttVicPair& b){
		if(a.GetAttackerCID() < b.GetAttackerCID()) return true;
		if(a.GetAttackerCID() == b.GetAttackerCID()){
			if(a.GetVictimCID() <= b.GetVictimCID()) return true;
		}
		return false;
	}
};

#endif