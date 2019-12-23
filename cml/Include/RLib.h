#pragma once

#pragma warning( disable : 4251 )

#ifndef RS_API

/////////////////////////////////////////////////////////////////////
// DLL로 export할 때 필요한 것들
#ifdef _USRDLL

#ifdef REALSPACE3_EXPORT
// DLL library project uses this
#define RS_API __declspec(dllexport)
#else
#ifdef REALSPACE3_IMPORT
// client of DLL uses this
#define RS_API __declspec(dllimport)
#else
// static library project uses this
#define RS_API
#endif
#endif // #ifdef REALSPACE3_EXPORT

#else
#define RS_API
#endif // #ifdef _USRDLL

#endif


#ifndef CML2z_API

/////////////////////////////////////////////////////////////////////
// DLL로 export할 때 필요한 것들
#ifdef _USRDLL

#ifdef CML2_EXPORT
// DLL library project uses this
#define CML2_API __declspec(dllexport)
#else
#ifdef CML2_IMPORT
// client of DLL uses this
#define CML2_API __declspec(dllimport)
#else
// static library project uses this
#define CML2_API
#endif
#endif // #ifdef CML2_EXPORT

#else
#define CML2_API
#endif // #ifdef _USRDLL

#endif