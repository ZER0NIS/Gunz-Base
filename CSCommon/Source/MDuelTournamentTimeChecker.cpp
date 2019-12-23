#include "stdafx.h"
#include "MDuelTournamentTimeChecker.h"

MDuelTournamentTimeChecker::MDuelTournamentTimeChecker(void)
{
	memset(m_szTimeStamp, 0, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);

	m_CurrTime = 0;
	m_nCurrMonth = 0;
	m_nCurrDay = 0;

	m_bTimeStampChanged = false;
}

void MDuelTournamentTimeChecker::Tick(unsigned int nTick)
{
	CTime nCurrTime = CTime::GetCurrentTime();

	if( nCurrTime > m_CurrTime){
		int nMonth  = nCurrTime.GetMonth();
		int nDay    = nCurrTime.GetDay();
		int nHour   = nCurrTime.GetHour();
		int nMinute = nCurrTime.GetMinute();

		if( nMonth > m_nCurrMonth ){	///< ÇÏ·ç°¡ Èê·¶À»²¨´Ù..
			if( nMinute == 10 ) {
				MMatchServer::GetInstance()->LOG(MMatchServer::LOG_PROG, 
					"MDuelTournamentTimeChecker::Current Month - %d, Day - %d", nMonth, nDay);

				MMatchServer::GetInstance()->OnAsyncRequest_GetDuelTournamentTimeStamp();

				m_CurrTime = nCurrTime;
				m_nCurrMonth = nMonth;
				m_nCurrDay = nDay;
			}
		} else if( nDay > m_nCurrDay ){	///< ÇÏ·ç°¡ Èê·¶´Ù..
			if( nMinute == 10 ) {
				MMatchServer::GetInstance()->LOG(MMatchServer::LOG_PROG, 
					"MDuelTournamentTimeChecker::Current Month - %d, Day - %d", nMonth, nDay);

				MMatchServer::GetInstance()->OnAsyncRequest_GetDuelTournamentTimeStamp();

				m_CurrTime = nCurrTime;
				m_nCurrMonth = nMonth;
				m_nCurrDay = nDay;
			}
		}
	}
}

void MDuelTournamentTimeChecker::SetTimeStamp(const char* szTimeStamp)
{
	memcpy(m_szTimeStamp, szTimeStamp, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);
}

bool MDuelTournamentTimeChecker::IsSameTimeStamp(const char* szTimeStamp)
{
	if( strcmp(m_szTimeStamp, szTimeStamp) == 0 ) return true;
	return false;
}