#pragma once

// MMatchActiveTrapMgr는 유저가 던져서 맵 상에 발동중이거나 혹은 아직 발동되기 전인 트랩들의 목록을 관리한다.
// (게임 도중 난입한 유저에게 현재 월드에 존재하는 트랩들을 알려주기 위해서 기억해두는 것이다)


class MMatchActiveTrap
{
public:
	unsigned long m_nTimeThrowed;
	int m_nLifeTime;

	// 트랩을 던졌다는 커맨드로 얻는 정보
	MUID m_uidOwner;
	int m_nTrapItemId;

	// 이후 트랩이 발동되었다는 커맨드로 정보를 보충
	MVector3 m_vPosActivated;
	unsigned long m_nTimeActivated;

	// 이 트랩이 던져졌으나 발동되기 전의 시간대에 난입한 유저의 uid를 여기 기억해 둠
	vector<MUID> m_vecUidForcedEntered;

public:
	MMatchActiveTrap();
	bool IsActivated() { return m_nTimeActivated!=0; }
	void AddForcedEnteredPlayer(const MUID& uid);
};

class MMatchActiveTrapMgr
{
	typedef list<MMatchActiveTrap*>		ListTrap;
	typedef ListTrap::iterator			ItorTrap;
	list<MMatchActiveTrap*> m_listTrap;

	MMatchStage* m_pStage;

public:
	MMatchActiveTrapMgr();
	~MMatchActiveTrapMgr();

	void Create(MMatchStage* pStage);
	void Destroy();
	void Clear();

	void AddThrowedTrap(const MUID& uidOwner, int nItemId);
	void OnActivated(const MUID& uidOwner, int nItemId, const MVector3& vPos);

	void Update(unsigned long nClock);

	void RouteAllTraps(MMatchObject* pObj);
	void RouteTrapActivationForForcedEnterd(MMatchActiveTrap* pTrap);
};