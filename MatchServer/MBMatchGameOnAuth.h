#ifndef _MBMATCH_GAMEONMODULE
#define _MBMATCH_GAMEONMODULE

#include ".\\GameOn\\GCCertificationHelper.h"





class MBMatchGameOnModule
{
private:
	GCCertificaltionHelper	m_Gcc;
	
	string		m_strUserSerialNum;
	string		m_strNickName;
	int			m_nPCBang;
	int			m_nSex;
	int			m_nAge;
	int			m_nLocation;
	int			m_nBirthYear;


public :
	MBMatchGameOnModule();
	virtual ~MBMatchGameOnModule();


	bool InitModule();

	int CheckCertification( const wchar_t* wszSting, const wchar_t* wszStatIndex);

	const char* GetUserSerialNum()		{ return m_strUserSerialNum.c_str();	}
	const char* GetUserNickName()		{ return m_strNickName.c_str();			}
	int			GetPCBang()				{ return m_nPCBang;						}
	int			GetSex()				{ return m_nSex;						}
	int			GetAge()				{ return m_nAge;						}
	int			GetLocation()			{ return m_nLocation;					}
	int			nBirthYear()			{ return m_nBirthYear;					}



	static MBMatchGameOnModule& GetInstance() 
	{
		static MBMatchGameOnModule module;

		return module;
	}
};


MBMatchGameOnModule& GetGameOnModule();

#endif