
/***********************************************************************************************************/
/* [TAG] Xtrap_S_Interface.h | 2009.07.01 | X-TRAP Interface Library For Server                            */
/*                                                                                                         */
/*  Copyright (C)WiseLogic 2005 - 2009 All Rights Reserved                                                 */
/***********************************************************************************************************/

#ifndef __WISELOGIC_Xtrap_S_Interface_H
#define __WISELOGIC_Xtrap_S_Interface_H

#ifdef WIN32
#define XTRAPCC_CALLCONV __cdecl
#else
#define XTRAPCC_CALLCONV
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reference Macro Definition
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define XTRAP_SETINFO_PACKETBUFF_SIZE				128
#define XTRAP_CS4_BUFSIZE_MAP						13000

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Return Values
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define XTRAP_API_RETURN_OK							0x00000000
#define XTRAP_API_RETURN_ERROR						0x00000001
#define XTRAP_API_RETURN_UNKNOWN					0x00000002	/*unused*/
#define XTRAP_API_RETURN_INVALID_PARAMETER			0x00000003
#define XTRAP_API_RETURN_INVALID_CRC				0x00000004	/*unused*/
#define XTRAP_API_RETURN_TIMEOUT					0x00000005	/*unused*/
#define XTRAP_API_RETURN_DETECTTIMEOUT				0x00000006	/*unused*/
#define XTRAP_API_RETURN_INVALID_FILEVERSION		0x00000007
#define XTRAP_API_RETURN_DETECTHACK					0x0000000F

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (Example) Internal Definition Function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int XTrap_S_LoadDll();
unsigned int XTrap_S_FreeDll();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Export Function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_S_Start) (
	IN		unsigned int	ulTimeOut, 
	IN		unsigned int	ulMapCount, 
	IN		const void *	pBufMap, 
	IN OUT	void *			Reserved 
);

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_S_SessionInit) (
	IN		unsigned int	ulTimeOut, 
	IN		unsigned int	ulMapCount, 
	IN		const void *	pBufMap, 
	IN OUT	void *			pBufSession 
);

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_CS_Step1)	(
	IN OUT	void *			pBufSession, 
	IN		void *			pBufPackData
);

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_CS_Step3)	(
	IN OUT	void *			pBufSession, 
	IN		const void *	pBufPackData
);

#ifndef __Xtrap4Server_a_import_H
extern PFN_XTrap_S_Start			XTrap_S_Start;
extern PFN_XTrap_S_SessionInit		XTrap_S_SessionInit;
extern PFN_XTrap_CS_Step1			XTrap_CS_Step1;
extern PFN_XTrap_CS_Step3			XTrap_CS_Step3;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Export (Reserved) Function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define XTRAP_ACTIVE_CODE_DEFAULT					0x0FFFFFFF
#define XTRAP_ACTIVE_CODE_THEMIDA					0x0FFFFFC3
#define XTRAP_ACTIVE_CODE_NOTUSE_MAPFILE			0x0FFFFF0C

#define XTRAP_ACTIVE_CODE_LEVEL1					XTRAP_ACTIVE_CODE_DEFAULT
#define XTRAP_ACTIVE_CODE_LEVEL2					XTRAP_ACTIVE_CODE_THEMIDA
#define XTRAP_ACTIVE_CODE_LEVEL3					XTRAP_ACTIVE_CODE_NOTUSE_MAPFILE

#define XTRAP_CS_OPTION_NULL						0x00000000
#define XTRAP_CS_OPTION_USETIME						0x00000001

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_S_SetActiveCode) ( 
	IN		unsigned int	ulActiveCode 
);

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_S_SetOption) ( 
	IN		unsigned int	ulMethod, 
	IN OUT	void *			pArray 
);

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_S_SendGamePacket)	( 
	IN		void *			pBufSession 
);

typedef unsigned int (XTRAPCC_CALLCONV *PFN_XTrap_S_RecvGamePacket)	( 
	IN		void *			pBufSession 
);

#ifndef __Xtrap4Server_a_import_H
extern PFN_XTrap_S_SetActiveCode	XTrap_S_SetActiveCode;
extern PFN_XTrap_S_SetOption		XTrap_S_SetOption;
extern PFN_XTrap_S_SendGamePacket	XTrap_S_SendGamePacket;
extern PFN_XTrap_S_RecvGamePacket	XTrap_S_RecvGamePacket;
#endif

#ifdef __cplusplus
}
#endif

#endif
