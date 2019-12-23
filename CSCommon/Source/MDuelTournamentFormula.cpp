#include "stdafx.h"
#include "MDuelTournamentFormula.h"

int MDuelTournamentFormula::Calc_WinnerTP(int nWinnerTP, int nLoserTP, bool isFinal)
{
	float fResult = 5.0f / (1 + pow(5.0f, float(nWinnerTP-nLoserTP) / 1000.0f));
	
	if( fResult < 1 ) fResult = 1;

	if( isFinal )	return (int)(fResult * 2);	// 결승전은 득점이 2배
	else			return (int)fResult;
}

int MDuelTournamentFormula::Calc_LoserTP(int nWinnerGainTP, bool isFinal)
{
	float fWinnerGainTP = (float)nWinnerGainTP;
	if (isFinal)
		fWinnerGainTP /= 2.f;			// 결승인 경우 승자는 2배 보너스를 받으므로 원래 득점으로 되돌린 후

	return (int)(fWinnerGainTP / 2.f);	// 승자 득점의 절반만큼을 패자에게 감점
}