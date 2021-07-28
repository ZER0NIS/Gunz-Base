#ifndef MCOMMAND_H
#define MCOMMAND_H

#include "MUID.h"

#include <vector>
#include <list>
#include <set>
#include <deque>

using namespace std;

#include "MCommandParameter.h"
#include "mempool.h"

class MCommandManager;

#define MCDT_NOTINITIALIZED		0
#define MCDT_MACHINE2MACHINE	1
#define MCDT_LOCAL				2
#define MCDT_TICKSYNC			4
#define MCDT_TICKASYNC			8
#define MCDT_USER				16
#define MCDT_ADMIN				32
#define MCDT_PEER2PEER			64

#define MCCT_NON_ENCRYPTED		128
#define MCCT_HSHIELD_ENCRYPTED	256

#define MAX_COMMAND_PARAMS		255

class MCommandDesc {
protected:
	int			m_nID;
	char		m_szName[256];
	char		m_szDescription[256];
	int			m_nFlag;

	vector<MCommandParameterDesc*>	m_ParamDescs;
public:
	MCommandDesc(int nID, const char* szName, const char* szDescription, int nFlag);
	virtual ~MCommandDesc(void);

	void AddParamDesc(MCommandParameterDesc* pParamDesc);

	bool IsFlag(int nFlag) const;
	int GetID(void) const { return m_nID; }
	const char* GetName(void) const { return m_szName; }
	const char* GetDescription(void) const { return m_szDescription; }
	MCommandParameterDesc* GetParameterDesc(int i) const {
		if (i < 0 || i >= (int)m_ParamDescs.size()) return NULL;
		return m_ParamDescs[i];
	}
	int GetParameterDescCount(void) const {
		return (int)m_ParamDescs.size();
	}
	MCommandParameterType GetParameterType(int i) const
	{
		if (i < 0 || i >= (int)m_ParamDescs.size()) return MPT_END;
		return m_ParamDescs[i]->GetType();
	}
	MCommandDesc* Clone();
};

class MCommand : public CMemPool<MCommand>
{
public:
	MUID						m_Sender;
	MUID						m_Receiver;
	const MCommandDesc* m_pCommandDesc;
	vector<MCommandParameter*>	m_Params;
	unsigned char				m_nSerialNumber;
	DWORD						m_dwPostTime;			///< Custom: Command send/recv time
	int							m_nCommandType;			///< Custom: Command type (encrypt/decrypt/exchange)
	void ClearParam(int i);

protected:
	void Reset(void);
	void ClearParam(void);

public:
	MCommand(void);
	MCommand(const MCommandDesc* pCommandDesc, MUID Receiver, MUID Sender);
	MCommand::MCommand(int nID, MUID Sender, MUID Receiver, MCommandManager* pCommandManager);
	virtual ~MCommand(void);

	void SetID(const MCommandDesc* pCommandDesc);
	void MCommand::SetID(int nID, MCommandManager* pCommandManager);
	int GetID(void) const { return m_pCommandDesc->GetID(); }
	const char* GetDescription(void) { return m_pCommandDesc->GetDescription(); }

	bool AddParameter(MCommandParameter* pParam);
	int GetParameterCount(void) const;
	MCommandParameter* GetParameter(int i) const;

	bool GetParameter(void* pValue, int i, MCommandParameterType t, int nBufferSize = -1) const;

	MUID GetSenderUID(void) { return m_Sender; }
	void SetSenderUID(const MUID& uid) { m_Sender = uid; }
	MUID GetReceiverUID(void) { return m_Receiver; }

	bool IsLocalCommand(void) { return (m_Sender == m_Receiver); }

	MCommand* Clone(void) const;

	bool CheckRule(void);

	int GetData(char* pData, int nSize);
	bool SetData(char* pData, MCommandManager* pCM, unsigned short nDataLen = USHRT_MAX);

	int GetSize();
	int GetSerial() { return m_nSerialNumber; }
};

class MCommandSNChecker
{
private:
	int				m_nCapacity;
	deque<int>		m_SNQueue;
	set<int>		m_SNSet;
public:
	MCommandSNChecker();
	~MCommandSNChecker();
	void InitCapacity(int nCapacity);
	bool CheckValidate(int nSerialNumber);
};

#define NEWCMD(_ID)		(new MCommand(_ID))
#define AP(_P)			AddParameter(new _P)
#define MKCMD(_C, _ID)									{ _C = NEWCMD(_ID); }
#define MKCMD1(_C, _ID, _P0)							{ _C = NEWCMD(_ID); _C->AP(_P0); }
#define MKCMD2(_C, _ID, _P0, _P1)						{ _C = NEWCMD(_ID); _C->AP(_P0); _C->AP(_P1); }
#define MKCMD3(_C, _ID, _P0, _P1, _P2)					{ _C = NEWCMD(_ID); _C->AP(_P0); _C->AP(_P1); _C->AP(_P2); }
#define MKCMD4(_C, _ID, _P0, _P1, _P2, _P3)				{ _C = NEWCMD(_ID); _C->AP(_P0); _C->AP(_P1); _C->AP(_P2); _C->AP(_P3); }
#define MKCMD5(_C, _ID, _P0, _P1, _P2, _P3, _P4)		{ _C = NEWCMD(_ID); _C->AP(_P0); _C->AP(_P1); _C->AP(_P2); _C->AP(_P3); _C->AP(_P4); }
#define MKCMD6(_C, _ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ _C = NEWCMD(_ID); _C->AP(_P0); _C->AP(_P1); _C->AP(_P2); _C->AP(_P3); _C->AP(_P4); _C->AP(_P5); }

typedef MCommand				MCmd;
typedef MCommandDesc			MCmdDesc;

#endif