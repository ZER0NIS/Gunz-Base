#pragma once

#include <map>

#include "MTypes.h"
#include "MEvent.h"

class MDrawContext;
class MWidget;
class MFont;
class MBitmap;
class MResourceMap;
class MIDLResource;
class MListener;
class MEvent;

#define MINT_VERSION	2
#define	MVersion()	MINT_VERSION

typedef bool(MGLOBALEVENTCALLBACK)(MEvent* pEvent);

class Mint {
protected:
	static Mint* m_pInstance;
	MWidget* m_pMainFrame;
	MDrawContext* m_pDC;
	MGLOBALEVENTCALLBACK* m_fnGlobalEventCallBack;

	char		m_szDragObjectString[256];
	char		m_szDragObjectItemString[256];
	MBitmap* m_pDragObjectBitmap;
	MPOINT		m_GrabPoint;
	bool		m_bVisibleDragObject;
	MWidget* m_pDropableObject;
	MWidget* m_pDragSourceObject;

	int			m_nWorkspaceWidth;
	int			m_nWorkspaceHeight;
	bool		Stretch = true;

	void* m_pCandidateList;
	int		m_nCandidateListSize;
	MPOINT	m_CandidateListPos;

	bool	m_bEnableIME;

public:
	DWORD	m_nCompositionAttributeSize;
	BYTE	m_nCompositionAttributes[MIMECOMPOSITIONSTRING_LENGTH];
	int		m_nCompositionCaretPosition;

protected:
	virtual void DrawCandidateList(MDrawContext* pDC, MPOINT& p);

public:
	Mint(void);
	virtual ~Mint(void);

	bool Initialize(int nWorkspaceWidth, int nWorkspaceHeight, MDrawContext* pDC, MFont* pDefaultFont);
	void Finalize(void);

#ifdef WIN32
	bool ProcessEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	bool ProcessEvent(SDL_Event& event);
#endif

	virtual void Run(void);
	virtual void Draw(void);

	virtual void Update(void) {}

	MWidget* GetMainFrame(void);

	MDrawContext* GetDrawContext(void);

	static Mint* GetInstance(void);

#ifdef WIN32
	void SetHWND(HWND hWnd);
	HWND GetHWND(void);

	HIMC m_hImc;
#endif

	void EnableIME(bool bEnable);
	bool IsEnableIME(void);

	int RegisterHotKey(unsigned long int nModifier, unsigned long int nVirtKey);
	void UnregisterHotKey(int nID);

	MWidget* SetDragObject(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	MWidget* GetDragObject(void);

	virtual MWidget* NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener);

	MWidget* FindWidgetDropAble(MPOINT& p);
	MWidget* FindWidget(MPOINT& p);
	MWidget* FindWidget(int x, int y);

	int GetWorkspaceWidth();
	int GetWorkspaceHeight();
	void SetWorkspaceSize(int w, int h);

	auto GetStretch() const { return Stretch; }
	void SetStretch(bool b) { Stretch = b; }

	virtual MBitmap* OpenBitmap(const char* szName) = 0;
	virtual MFont* OpenFont(const char* szName, int nHeight) = 0;

	void SetGlobalEvent(MGLOBALEVENTCALLBACK pGlobalEventCallback);

	const char* GetDefaultFontName(void) const;

	int GetPrimaryLanguageIdentifier(void) const;

	int GetSubLanguageIdentifier(void) const;

	const char* GetLanguageIndicatorString(void) const;

	bool IsNativeIME(void) const;

	void OpenCandidateList(void);
	void CloseCandidateList(void);

	const char* GetCandidate(int nIndex) const;
	int GetCandidateCount(void) const;
	int GetCandidateSelection(void) const;
	int GetCandidatePageStart(void) const;
	int GetCandidatePageSize(void) const;

	void SetCandidateListPosition(MPOINT& p, int nWidgetHeight);
	int GetCandidateListWidth(void);
	int GetCandidateListHeight(void);

	DWORD GetCompositionAttributeSize(void) const { return m_nCompositionAttributeSize; }
	const BYTE* GetCompositionAttributes(void) const { return m_nCompositionAttributes; }
	int DrawCompositionAttribute(MDrawContext* pDC, MPOINT& p, const char* szComposition, int i);
	void DrawCompositionAttributes(MDrawContext* pDC, MPOINT& p, const char* szComposition);
	void DrawIndicator(MDrawContext* pDC, MRECT& r);
};

inline int MGetWorkspaceWidth()
{
	return Mint::GetInstance()->GetWorkspaceWidth();
}
inline int MGetWorkspaceHeight()
{
	return Mint::GetInstance()->GetWorkspaceHeight();
}

inline int MGetCorrectedWorkspaceWidth()
{
	if (Mint::GetInstance()->GetStretch())
		return MGetWorkspaceWidth();

	auto Aspect = static_cast<float>(MGetWorkspaceWidth()) / MGetWorkspaceHeight();
	return static_cast<int>(MGetWorkspaceWidth() / Aspect * (4.f / 3.f));
}

#define CONVERT800(x)	(int((x)/800.f * MGetWorkspaceWidth()))
#define CONVERT600(y)	(int((y)/600.f * MGetWorkspaceHeight()))

void MCreateSample(void);
void MDestroySample(void);

#define MMODIFIER_ALT	1
#define MMODIFIER_CTRL	2
#define MMODIFIER_SHIFT	4

#define MIsActionKeyPressed(_ActionID)	(Mint::GetInstance()->IsActionKeyPressed(_ActionID))

#define IsHangul(x) ((unsigned char)(x)>127)
