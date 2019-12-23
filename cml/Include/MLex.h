#pragma once

#define MLEX_LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define MLEX_UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))


typedef bool(MLEXPROC)(void* pData, int nLevel, char* pszArg);


struct MLEXNODE {
	char*		szCmd;
	int			nLevel;
	MLEXPROC*	pProc;		
};


class MLex {
private:
	MLEXNODE*	m_pCmdTable;

private:
	void SkipSpaces(char **ppszString);
	int FindCommand(char* szString);

public:
	MLex(MLEXNODE* pNode = 0)		{ m_pCmdTable = pNode; }
	virtual ~MLex()		{}

	char* GetOneArg(char *pszArg, char *pszOutArg);
	char* GetOneArg(char *pszArg, char *pszOutArg, char *pszOutArg2);//jintriple3  귓속말 할때 to: 다음에 소문자로만 나오던 캐릭터 이름을 원래 이름으로 처리.
	char* GetTwoArgs(char* pszArg, char* pszOutArg1, char* pszOutArg2);
	bool SplitValue(char* pszSource, char* pszSeperator, char* pszField, char* pszValue);

	void SetCmdTable(MLEXNODE* pCmdTable) { m_pCmdTable = pCmdTable; }
	MLEXNODE* GetCmdTable() { return m_pCmdTable; }
	void Interprete(void* pData, char* pszString);
};
