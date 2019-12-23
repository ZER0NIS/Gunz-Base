#ifndef CRXHeaderH
#define CRXHeaderH

#include <string>

//#define _USING_CXR

#ifndef _USING_CXR
#define _CXR(x) x
#else
#define _CXR(x) __CXRDecrypt(x).c_str()
extern std::string __CXRDecrypt(const char *pIn);
#endif

#endif
