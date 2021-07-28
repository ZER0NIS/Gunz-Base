#include <list>
#include <string>
#include "MWidget.h"
#include "MLookNFeel.h"
#include "MScrollBar.h"

using namespace std;

class MTextAreaChat;
class MScrollBar;

#define DEFAULT_TEXT_COLOR		MCOLOR(224,224,224)

class MTextAreaChatLook {
protected:
	MCOLOR m_bgColor;
public:
	MTextAreaChatLook() : m_bgColor(0, 0, 0, 0) {}
	virtual void OnDraw(MTextAreaChat* pTextArea, MDrawContext* pDC);
	virtual MRECT GetClientRect(MTextAreaChat* pTextArea, MRECT& r);
	virtual void SetBgColor(MCOLOR& c) { m_bgColor = c; }

private:
	virtual void OnFrameDraw(MTextAreaChat* pTextArea, MDrawContext* pDC);
	virtual void OnTextDraw(MTextAreaChat* pTextArea, MDrawContext* pDC);
	virtual void OnTextDraw_WordWrap(MTextAreaChat* pTextArea, MDrawContext* pDC);
};

struct MLineItemChat {
	MLineItemChat(MCOLOR _color, string& _text) {
		color = _color;
		text = _text;
	}
	MLineItemChat(string& _text) {
		color = DEFAULT_TEXT_COLOR;
		text = _text;
	}
	MCOLOR color;
	string text;
};

typedef std::list<MLineItemChat> MLINELISTCHAT;
typedef std::list<MLineItemChat>::iterator MLINELISTITERATORCHAT;

class MTextAreaChat : public MWidget {
	friend MTextAreaChatLook;
protected:
	bool		m_bScrollBarEnable;
	int			m_nIndentation;
	bool		m_bWordWrap;
	bool		m_bColorSupport;
	MPOINT		m_TextOffset;
	bool		m_bMouseOver;
	MCOLOR		m_TextColor;
	int			m_nMaxLen;
	char		m_szIMECompositionString[MIMECOMPOSITIONSTRING_LENGTH];
	bool		m_bEditable;
	int			m_nStartLine;
	int			m_nStartLineSkipLine;

	int			m_nCurrentSize;
	bool		m_bVerticalMoving;
	int			m_nVerticalMoveAxis;

	int			m_nCustomLineHeight;

	MPOINT		m_CaretPos;
	bool		m_bCaretFirst;

	MLINELISTCHAT			m_Lines;
	MLINELISTITERATORCHAT	m_CurrentLine;

	MScrollBar* m_pScrollBar;

	DECLARE_LOOK(MTextAreaChatLook)
	DECLARE_LOOK_CLIENT()

protected:
	virtual void OnSize(int w, int h);
	virtual bool OnCommand(MWidget* pWindow, const char* szMessage);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual void OnSetFocus(void);
	virtual void OnReleaseFocus(void);

	virtual bool InputFilterKey(int nKey, bool bCtrl);
	virtual bool InputFilterChar(int nKey);

	bool OnLButtonDown(MPOINT pos);
	void OnScrollBarChanged(int nPos);

	bool MoveLeft(bool bBackspace = false);
	void MoveRight();
	void MoveDown();
	void MoveUp();
	void DeleteCurrent();
	void ScrollDown();
	void ScrollUp();
	void MoveFirst();
	void MoveLast();
	void OnHome();
	void OnEnd();

	bool GetCaretPosition(MPOINT* pOut, int nSize, const char* szText, int nPos, bool bFirst);

	int GetCharPosition(const char* szText, int nX, int nLine);

	bool IsDoubleCaretPos();

	void UpdateScrollBar(bool bAdjustStart = false);

	MLINELISTITERATORCHAT GetIterator(int nLine);

public:
	MTextAreaChat(int nMaxLen = 120, const char* szName = NULL, MWidget* pParent = NULL, MListener* pListener = NULL);
	virtual ~MTextAreaChat();

#define MINT_TEXTAREA_CHAT	"TextAreaChat"
	virtual const char* GetClassName(void) { return MINT_TEXTAREA_CHAT; }

	MPOINT GetCaretPos() { return m_CaretPos; }
	int	GetStartLine() { return m_nStartLine; }

	bool IsScrollBarVisible() { return m_pScrollBar->IsVisible(); }
	int GetScrollBarWidth() { return m_pScrollBar->GetRect().w; }
	int GetScrollBarMax() { return m_pScrollBar->GetMax(); }

	int GetClientWidth();

	int GetLength() { return (int)(m_nCurrentSize + m_Lines.size()); }
	int GetLineCount() { return (int)m_Lines.size(); }
	int GetTotalLineCount(int& nStartLine, int& nCurrentLine);

	bool GetText(char* pBuffer, int nBufferSize);
	const char* GetTextLine(int nLine);

	void SetMaxLen(int nMaxLen);
	int	GetMaxLen() { return m_nMaxLen; }

	const char* GetCompositionString(void);

	void SetEditable(bool editable) { m_bEditable = editable; }
	bool GetEditable() { return m_bEditable; }

	void SetScrollBarEnable(bool bEnable) { m_bScrollBarEnable = bEnable; }
	bool GetScrollBarEnable() { return m_bScrollBarEnable; }

	void SetTextOffset(MPOINT p);

	void SetIndentation(int nIndentation) { m_nIndentation = nIndentation; }

	void SetTextColor(MCOLOR color);
	MCOLOR GetTextColor(void);

	void Clear();

	void SetText(const char* szText);
	void AddText(const char* szText, MCOLOR color);
	void AddText(const char* szText);

	void DeleteFirstLine();

	int GetLineHeight(void);
	void SetCustomLineHeight(int nHeight);

	virtual void MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight);

	void AdjustHeightByContent();

	int GetTextPosition(const char* text);
};

#define MTextAreaChat_ENTER_VALUE		"entered"
#define MTextAreaChat_ESC_VALUE			"esc"