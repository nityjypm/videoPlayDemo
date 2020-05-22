#ifndef _CVSOCKETOPT_
#define _CVSOCKETOPT_

#ifdef __cplusplus
extern "C" {
#endif

int cv_getLocalIp(char * ifname, char * ip, int ipLen);
int cv_tcpConnect(const char* szIP, int nPort, int nTimeOut);
int cv_tcpSend(const char* pData, int nLen, int socketFd, int nTimeOut);
int cv_tcpRead(char* pBuf, int nMaxSize, int socketFd, int nTimeOut);
int cv_tcpClose(int socketFd);

#ifdef __cplusplus
}
#endif

#endif



































