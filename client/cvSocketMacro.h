#ifndef _CVSOCKETMACRO_
#define _CVSOCKETMACRO_

#ifdef __cplusplus
extern "C"{
#endif

#define CVSOCKET_LOG fprintf
#define CVSOCKET_ERR printf//(void)
#define CVSOCKET_OUT_FP stderr//rsc->fpLogOut

#ifdef _WIN32
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS // _CRT_SECURE_NO_WARNINGS for sscanf errors in MSVC2013 Express
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <fcntl.h>
#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <io.h>
#include <time.h>
#include "mstcpip.h"  
	//#include <Windows.h>

#ifndef _SSIZE_T_DEFINED
	typedef int ssize_t;
#define _SSIZE_T_DEFINED
#endif
#ifndef _SOCKET_T_DEFINED
	typedef SOCKET socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef snprintf
#define snprintf _snprintf
#endif
#define close closesocket
#if _MSC_VER >=1600
	// vs2010 or later
#include <stdint.h>
#else
	typedef __int8 int8_t;
	typedef __int16 uint16_t;
	typedef unsigned __int8 uint8_t;
	typedef __int32 int32_t;
	typedef unsigned __int32 uint32_t;
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
#endif
#define socketerrno WSAGetLastError()
#define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#define setsockopt(a,b,c,d,e)	(setsockopt)(a,b,c,(const char *)d,(int)e)
#define strcasecmp _stricmp
#define	MSDK_CLEANUPSOCKETS()	WSACleanup()

#define MSDK_INITSOCKETS()	{\
	WORD version;			\
	WSADATA wsaData;		\
	\
	version = MAKEWORD(1,1);	\
	WSAStartup(version, &wsaData);	}

#else
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/time.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#include <errno.h>
#define socketerrno errno
#define SOCKET_EAGAIN_EINPROGRESS EAGAIN
#define SOCKET_EWOULDBLOCK EWOULDBLOCK
#define MSDK_INITSOCKETS()
#define	MSDK_CLEANUPSOCKETS()
#endif

#define WS_MAX_FRM_LEN (8*1024)
#define WS_MAX_MSG_LEN (8*1024)

#ifdef __cplusplus
}
#endif



#endif