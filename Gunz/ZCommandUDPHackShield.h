#pragma once

// NHN 커맨트 해킹 방어를 위한 클래스(2010. 07. 07 홍기주)
// NHN 커맨드 해킹이란, UDP로 MatchServer의 커맨드를 복제한 뒤에 보내는 해킹 수법(MC_NET_DISCONNECT 등..)
class ZCommandUDPHackShield
{
private:
	set<int> m_UDPDeniedCommand;
	void AddDeniedCommand(int nCommandID);
public:
	void Init();
	bool IsDeniedCommand(int nCommandID);
};