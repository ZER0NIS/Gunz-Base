/////////////////////////////////////////////////////////////
// CXR-generated decoder follows

#include <iostream>
#include <algorithm>
#include <string>
#include <stdexcept>
#define BYTE unsigned char 

class CCXR
{
protected:
   CCXR(const BYTE *key, unsigned int ks)
   {
      int i;BYTE rs;unsigned kp;
      for(i=0;i<256;i++)c[i]=i;kp=0;rs=0;for(i=255;i;i--)std::swap(c[i],c[kr(i,key,ks,&rs,&kp)]);r2=c[1];r1=c[3];av=c[5];lp=c[7];lc=c[rs];rs=0;kp=0;
   }
  inline void SC(){BYTE st=c[lc];r1+=c[r2++];c[lc]=c[r1];c[r1]=c[lp];c[lp]=c[r2];c[r2]=st;av+=c[st];}
  BYTE c[256],r2,r1,av,lp,lc;    

   BYTE kr(unsigned int lm, const BYTE *uk, BYTE ks, BYTE *rs, unsigned *kp)
   {
      unsigned rl=0,mk=1,u;while(mk<lm)mk=(mk<<1)+1;do{*rs=c[*rs]+uk[(*kp)++];if(*kp>=ks){*kp=0;*rs+=ks;}u=mk&*rs;if(++rl>11)u%=lm;}while(u>lm);return u;
   }
};
struct CXRD:CCXR
{
  CXRD(const BYTE *userKey, unsigned int keyLength=16) : CCXR(userKey, keyLength) {}
  inline BYTE pb(BYTE b){SC();lp=b^c[(c[r1]+c[r2])&0xFF]^c[c[(c[lp]+c[lc]+c[av])&0xFF]];lc=b;return lp;}
};
std::string __CXRDecrypt(const char *pIn)
{
   std::string x;char b[3];b[2]=0;
   const unsigned char __pCXRPassword[] = {0x65,0x4a,0x68,0x35,0x69,0x68,0x6a,0x42,0x2a,0x29,0x75,0x51,0x42,0x28,0x23,0x5d,0x28,0x24,0x31,0x3b,0x76,0x78};
   CXRD sap(__pCXRPassword, sizeof(__pCXRPassword));
   int iLen = strlen(pIn);
   if (iLen > 2)
   {
      const int __iCXRDecBase1=128; const int __iCXRDecBase2=128;

      int ibl=strlen(pIn);
      if (ibl&0x01)
      {
         throw new std::runtime_error("");
         return pIn;
      }
      ibl/=2;
      for (int i=0;i<ibl;i++)
      {
         int b1 =pIn[i*2]-__iCXRDecBase1;int b2=pIn[i*2+1]-__iCXRDecBase2;
         int c = (b1 << 4) | b2; char ch =(char)(sap.pb((BYTE)(c)));
         if (i>0) x+=ch;
      }
   }
   return x;
}
