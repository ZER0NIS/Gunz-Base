#include "stdafx.h"
#include "MEvent.h"
#include <zmouse.h>
#include "Mint.h"

MPOINT MEvent::LatestPos = MPOINT(0, 0);
bool MEvent::bIMESupport = false;
bool MEvent::bLButton = false;
bool MEvent::bMButton = false;
bool MEvent::bRButton = false;

bool MEvent::GetShiftState(void)
{
	if ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000) return true;
	return false;
}

bool MEvent::GetCtrlState(void)
{
	if ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) return true;
	return false;
}

bool MEvent::GetAltState(void)
{
	if ((GetKeyState(VK_MENU) & 0x8000) == 0x8000) return true;
	return false;
}

bool MEvent::IsKeyDown(int key)
{
	return ((GetAsyncKeyState(key) & 0x8000) != 0);
}

bool MEvent::GetLButtonState()
{
	return bLButton;
}

bool MEvent::GetRButtonState()
{
	return bRButton;
}

bool MEvent::GetMButtonState()
{
	return bMButton;
}

MPOINT MEvent::GetMousePos(void)
{
	POINT p;
	GetCursorPos(&p);
	return MPOINT(p.x, p.y);
}

#include "MDebug.h"
void MEvent::ForceSetIME(DWORD fdwConversion, DWORD fdwSentence)
{
	HWND hWnd = Mint::GetInstance()->GetHWND();
	HIMC hImc = ImmGetContext(hWnd);
	if (hImc)
	{
		ImmSetConversionStatus(hImc, fdwConversion, fdwSentence);
		ImmReleaseContext(hWnd, hImc);
	}
}

int MEvent::TranslateEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	bCtrl = GetCtrlState();
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		Pos.x = LOWORD(lparam);
		Pos.y = HIWORD(lparam);

		LatestPos = Pos;
		break;
	case WM_MOUSEWHEEL:
	{
		POINT pos;
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		ScreenToClient(Mint::GetInstance()->GetHWND(), &pos);

		Pos.x = pos.x;
		Pos.y = pos.y;

		LatestPos = Pos;
	}
	break;
	}

	switch (message) {
	case WM_LBUTTONDOWN:
		nMessage = MWM_LBUTTONDOWN;
		bLButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_LBUTTONUP:
		bLButton = false;
		nMessage = MWM_LBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONDOWN:
		bRButton = true;
		nMessage = MWM_RBUTTONDOWN;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONUP:
		bRButton = false;
		nMessage = MWM_RBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONDOWN:
		bMButton = true;
		nMessage = MWM_MBUTTONDOWN;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONUP:
		bMButton = false;
		nMessage = MWM_MBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_LBUTTONDBLCLK:
		nMessage = MWM_LBUTTONDBLCLK;
		bLButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONDBLCLK:
		nMessage = MWM_RBUTTONDBLCLK;
		bRButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONDBLCLK:
		nMessage = MWM_MBUTTONDBLCLK;
		bMButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_MOUSEMOVE:
		nMessage = MWM_MOUSEMOVE;
		return EVENT_MINT_TRANSLATED;
	case WM_MOUSEWHEEL:
		nMessage = MWM_MOUSEWHEEL;
		nDelta = (short)HIWORD(wparam);
		return EVENT_MINT_TRANSLATED;
	case WM_KEYDOWN:
		nMessage = MWM_KEYDOWN;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;

		return EVENT_MINT_TRANSLATED;
	case WM_KEYUP:
		nMessage = MWM_KEYUP;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSCHAR:
		nMessage = MWM_SYSCHAR;
		nKey = wparam;
		bAlt = true;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSKEYDOWN:
		nMessage = MWM_SYSKEYDOWN;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSKEYUP:
		nMessage = MWM_SYSKEYUP;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_CHAR:
		nMessage = MWM_CHAR;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_HOTKEY:
		nMessage = MWM_HOTKEY;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;

	case WM_INPUTLANGCHANGE:
		return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
	case WM_IME_STARTCOMPOSITION:
		if (bIMESupport == true) {
			szIMECompositionString[0] = NULL;
			szIMECompositionResultString[0] = NULL;
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
		return EVENT_PROCESSED;
	case WM_IME_COMPOSITION:
		if (bIMESupport == true) {
			nMessage = MWM_IMECOMPOSE;

			Mint* pMint = Mint::GetInstance();

			HIMC hIMC = ImmGetContext(hwnd);
			if (hIMC) {
				if (lparam & GCS_RESULTSTR) {
					LONG i = ImmGetCompositionString(hIMC, GCS_RESULTSTR, szIMECompositionResultString, sizeof(szIMECompositionResultString));
					szIMECompositionResultString[i] = NULL;
					pMint->m_nCompositionAttributeSize = 0;
					memset(pMint->m_nCompositionAttributes, 0, sizeof(BYTE) * (MIMECOMPOSITIONSTRING_LENGTH));
				}
				else {
					szIMECompositionResultString[0] = NULL;
				}
				if (lparam & GCS_COMPSTR) {
					LONG i = ImmGetCompositionString(hIMC, GCS_COMPSTR, szIMECompositionString, sizeof(szIMECompositionString));
					szIMECompositionString[i] = NULL;
				}
				else {
					szIMECompositionString[0] = NULL;
				}

				if (lparam & GCS_COMPATTR)
					pMint->m_nCompositionAttributeSize = ImmGetCompositionString(hIMC, GCS_COMPATTR, pMint->m_nCompositionAttributes, sizeof(pMint->m_nCompositionAttributes));

				if (lparam & GCS_CURSORPOS)
					pMint->m_nCompositionCaretPosition = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
			}

			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
		return EVENT_PROCESSED;
	case WM_IME_ENDCOMPOSITION:
		if (bIMESupport == true) {
			szIMECompositionString[0] = NULL;
			szIMECompositionResultString[0] = NULL;
			Mint* pMint = Mint::GetInstance();
			pMint->m_nCompositionCaretPosition = 0;
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
		return EVENT_PROCESSED;
	case WM_IME_NOTIFY:
	{
		Mint* pMint = Mint::GetInstance();
		if (bIMESupport == true && pMint->IsEnableIME() == true) {
			if (wparam == IMN_OPENCANDIDATE || wparam == IMN_CHANGECANDIDATE) {
				pMint->OpenCandidateList();
			}
			else if (wparam == IMN_CLOSECANDIDATE) {
				pMint->CloseCandidateList();
			}
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
	}
	return EVENT_PROCESSED;
	case WM_IME_SETCONTEXT:
		return EVENT_PROCESSED;
	default:
		return EVENT_NOT_PROCESSED;
	}

	return EVENT_NOT_PROCESSED;
}

int MEvent::TranslateEvent(SDL_Event& event)
{
	bCtrl = SDL_GetModState() & KMOD_CTRL;
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEWHEEL:
		Pos.x = event.motion.x;
		Pos.y = event.motion.y;

		LatestPos = Pos;
		break;
	}

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		switch (event.button.button) {
		case SDL_BUTTON_LEFT:
			nMessage = MWM_LBUTTONDOWN;
			bLButton = true;
			return EVENT_MINT_TRANSLATED;
		case SDL_BUTTON_RIGHT:
			nMessage = MWM_RBUTTONDOWN;
			bRButton = true;
			return EVENT_MINT_TRANSLATED;
		case SDL_BUTTON_MIDDLE:
			nMessage = MWM_MBUTTONDOWN;
			bMButton = true;
			return EVENT_MINT_TRANSLATED;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		switch (event.button.button) {
		case SDL_BUTTON_LEFT:
			bLButton = false;
			nMessage = MWM_LBUTTONUP;
			return EVENT_MINT_TRANSLATED;
		case SDL_BUTTON_RIGHT:
			bRButton = false;
			nMessage = MWM_RBUTTONUP;
			return EVENT_MINT_TRANSLATED;
		case SDL_BUTTON_MIDDLE:
			bMButton = false;
			nMessage = MWM_MBUTTONUP;
			return EVENT_MINT_TRANSLATED;
		}
		break;
	case SDL_MOUSEMOTION:
		nMessage = MWM_MOUSEMOVE;
		return EVENT_MINT_TRANSLATED;
	case SDL_MOUSEWHEEL:
		nMessage = MWM_MOUSEWHEEL;
		nDelta = event.wheel.y;
		return EVENT_MINT_TRANSLATED;
	case SDL_KEYDOWN:
		nMessage = MWM_KEYDOWN;
		nKey = event.key.keysym.sym;
		return EVENT_MINT_TRANSLATED;
	case SDL_KEYUP:
		nMessage = MWM_KEYUP;
		nKey = event.key.keysym.sym;
		return EVENT_MINT_TRANSLATED;
	case SDL_TEXTINPUT:
		nMessage = MWM_CHAR;
		nKey = event.text.text[0];
		return EVENT_MINT_TRANSLATED;
	}

	return EVENT_PROCESSED;
}