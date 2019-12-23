#pragma once

class MDuelTournamentFormula
{
public:
	MDuelTournamentFormula(void){}
	~MDuelTournamentFormula(void){}

	int Calc_WinnerTP(int nWinnerTP, int nLoserTP, bool isFinal=false);
	int Calc_LoserTP(int nWinnerGainTP, bool isFinal=false);
};
