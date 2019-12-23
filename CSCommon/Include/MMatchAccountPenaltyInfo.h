#pragma once

// ==============================================================================
// Admin에 의해 내려지는 형벌(?)에 관한 클래스입니다.
// 서버 내의 해킹 검출로 인하여 가해지는 형벌과는 별개입니다.
// 추후에는 합쳐야될텐데-.,- 어떻게 합쳐야될지..ㅎㄷㄷ
//																2010-08-09 홍기주
// ==============================================================================

// DB 상의 AccountPenlatyCode와 일치하여야 한다.
enum MPenaltyCode
{
	MPC_NONE = 0,
	MPC_CONNECT_BLOCK,
	MPC_CHAT_BLOCK,
	MPC_SHOP_BLOCK,
	MPC_MAX,
};

struct MPenaltyInfo
{
	MPenaltyCode	nPenaltyCode;
	SYSTEMTIME		sysPenaltyEndDate;
};

class MMatchAccountPenaltyInfo
{
protected:
	MPenaltyInfo m_PenaltyTable[MPC_MAX];

	SYSTEMTIME GetPenaltyEndDate(int nPenaltyHour);
public:
	MMatchAccountPenaltyInfo();
	~MMatchAccountPenaltyInfo();

	void ClearPenaltyInfo(MPenaltyCode nCode);

	void SetPenaltyInfo(MPenaltyCode nCode, int nPenaltyHour);
	void SetPenaltyInfo(MPenaltyCode nCode, SYSTEMTIME sysPenaltyEndDate);

	bool IsBlock(MPenaltyCode nCode);

	const MPenaltyInfo* GetPenaltyInfo(MPenaltyCode nCode) { return &m_PenaltyTable[nCode]; }
};