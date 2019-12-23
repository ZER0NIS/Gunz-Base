#ifndef MCOMMANDBUILDER_H
#define MCOMMANDBUILDER_H

#include <windowsx.h>
#include "MCommandManager.h"
#include "Msg.h"
#include "MDebug.h"
#include "MCRC32.h"
#include "MPacketCrypter.h"

const int MAX_COMMAND_COUNT_FLOODING = 50;

class MCommandBuilder
{	
protected:
	DWORD			m_dwLastCommandMakeTime;
	int				m_CommandCountPerSec;

	MUID					m_uidSender;	// client
	MUID					m_uidReceiver;	// server
	MCommandManager*		m_pCommandManager;

	#define COMMAND_BUFFER_LEN	65535 // Custom: Changed buffer size to fix shop list issues.

	char					m_Buffer[COMMAND_BUFFER_LEN];
	int						m_nBufferNext;

	MCommandList			m_CommandList;
	list<MPacketHeader*>	m_NetCmdList;

	MPacketCrypter*			m_pPacketCrypter;			// ¾ÏÈ£È­
	MCommandSNChecker		m_CommandSNChecker;
	bool					m_bCheckCommandSN;
protected:
	bool CheckBufferEmpty();
	bool EstimateBufferToCmd();
	void AddBuffer(char* pBuffer, int nLen);
	bool MoveBufferToFront(int nStart, int nLen);
	
	int CalcCommandCount(char* pBuffer, int nBufferLen);
	bool CheckFlooding(int nCommandCount);

	int MakeCommand(char* pBuffer, int nBufferLen);

	void Clear();
	int _CalcPacketSize(MPacketHeader* pPacket);
public:
	MCommandBuilder(MUID uidSender, MUID uidReceiver, MCommandManager*	pCmdMgr);
	virtual ~MCommandBuilder();
	void SetUID(MUID uidReceiver, MUID uidSender);
	void InitCrypt(MPacketCrypter* pPacketCrypter, bool bCheckCommandSerialNumber);
	bool Read(char* pBuffer, int nBufferLen, bool bFloodCheck=false, bool *bFloodResult=NULL);
	void SetCheckCommandSN(bool bCheck) { m_bCheckCommandSN = bCheck; }

	MCommand* GetCommand();
	MPacketHeader* GetNetCommand();
};


#endif