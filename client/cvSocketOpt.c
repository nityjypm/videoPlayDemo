#include <stdio.h>
#include <stdlib.h>
#include "cvSocketMacro.h"

#ifdef _WIN32
#define MAX_PATH 260 
int cv_getLocalIp(char * ifname, char * ip, int ipLen)  
{  
    char hostname[MAX_PATH] = {0};  
    char *temp = NULL;
    struct hostent FAR* lpHostEnt;
    LPSTR lpAddr;
    struct in_addr inAddr;  

    gethostname(hostname,MAX_PATH);                  
    lpHostEnt = gethostbyname(hostname);  
    if(NULL == lpHostEnt)  
    {  
        return -1;  
    }  

    lpAddr = lpHostEnt->h_addr_list[0];        

    memmove(&inAddr,lpAddr,4);  
    temp = inet_ntoa(inAddr);
    memcpy(ip, temp, strlen(temp));

    return 0;  
}
#else

int cv_getLocalIp(char * ifname, char * ip, int ipLen)
{
    char *temp = NULL;
    int inet_sock;
    struct ifreq ifr;

	if(ipLen<=16)
	{
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
    memset(ip, 0, ipLen);
    inet_sock = socket(AF_INET, SOCK_DGRAM, 0); 
	if(INVALID_SOCKET == inet_sock)  
	{	
		CVSOCKET_ERR("[%s - %s - %d] socket err\n",__FILE__, __FUNCTION__, __LINE__);
		return -1; 	   
	} 

    memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    memcpy(ifr.ifr_name, ifname, strlen(ifname));

    if(0 != ioctl(inet_sock, SIOCGIFADDR, &ifr)) 
    {   
        perror("ioctl error");
        return -1;
    }

    temp = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);     
    memcpy(ip, temp, strlen(temp));

    close(inet_sock);

    return 0;
}
#endif

int cv_tcpConnect(const char* szIP, int nPort, int nTimeOut)
{
    unsigned int cnnIp;
	int	socketFd = -1;	
	int ret, val;
	struct sockaddr_in my_addr;

	if(!szIP || nPort<0 || nTimeOut<0)
	{
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		return INVALID_SOCKET;	
	}
	
    memset(&my_addr, 0, sizeof(my_addr));
    cnnIp = inet_addr(szIP);

	if(cnnIp == INADDR_NONE)
	{
		struct hostent* host_ptr = gethostbyname(szIP);
		if(host_ptr == NULL)
		{
			return INVALID_SOCKET;
		}
		else
		{
			my_addr.sin_addr = *(struct in_addr *)host_ptr->h_addr;
		}
	}
	else
	{
		my_addr.sin_addr.s_addr = cnnIp;
	}
	
	my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(nPort);;
    socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketFd<0)
    {
        CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
        return INVALID_SOCKET;	
    }


		//set to non block	
#ifdef _WIN32
	val = 1;
	if (ioctlsocket(socketFd, FIONBIO, &val))
	{		    
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		close(socketFd);	
		return INVALID_SOCKET;
	}
#else
	int flags = fcntl(socketFd, F_GETFL, 0);
	if(-1==flags)
	{			
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		close(socketFd);	
		return INVALID_SOCKET;
	}

	if(-1==fcntl(socketFd, F_SETFL, flags | O_NONBLOCK))
	{			
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		close(socketFd);	
		return INVALID_SOCKET;
	}
#endif

	
	if(setsockopt(socketFd, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(int)))
	{			
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		close(socketFd);	
		return INVALID_SOCKET;
	}

    ret = connect(socketFd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
	
    if(INVALID_SOCKET == ret)
	{
        struct timeval timeo;
        int iRet;
		fd_set wds;
		FD_ZERO(&wds);
		FD_SET(socketFd,&wds);

		timeo.tv_sec = nTimeOut / 1000;
		timeo.tv_usec = nTimeOut % 1000 * 1000;
		
		iRet = select(socketFd + 1, NULL, &wds, NULL, &timeo);
		
		if(iRet > 0 && FD_ISSET(socketFd, &wds))
		{
			int error = -1;
			int llen = sizeof(int);
#ifdef _WIN32
			int retVal = getsockopt(socketFd, SOL_SOCKET, SO_ERROR, (char*)&error, &llen);
#else
			int retVal=getsockopt(socketFd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t *)&llen);
#endif

			if(0!=retVal)
			{
				ret = INVALID_SOCKET;
				close(socketFd);
			}			
			else if(error == 0)
			{
				ret = socketFd;			
			}
			else
			{
				ret = INVALID_SOCKET;
				close(socketFd);
			}			
			return ret;
		}
		else
		{
			int error = -1;
			int llen = sizeof(int);
#ifdef WIN32
			getsockopt(socketFd, SOL_SOCKET, SO_ERROR, (char*)&error, &llen);
#else
			getsockopt(socketFd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t *)&llen);
#endif
			close(socketFd);
			return INVALID_SOCKET;
		}
	}
    return socketFd;
}

int cv_tcpSend(const char* pData, int nLen, int socketFd, int nTimeOut)
{
	int nSendLen=0;
	struct timeval timeoutSend;
	fd_set writefds;	
    int ret;
	
	FD_ZERO(&writefds);
	FD_SET(socketFd,&writefds);

	timeoutSend.tv_sec = nTimeOut / 1000;
	timeoutSend.tv_usec = nTimeOut % 1000 * 1000;

	ret = select(socketFd + 1, NULL, &writefds, NULL, &timeoutSend);
	if(1==ret)
	{
		nSendLen = send(socketFd, pData, nLen, 0);
		if(nSendLen<=0)
		{
			CVSOCKET_ERR("[%s - %s - %d] send err\n",__FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
	}
	else if(-1==ret)
	{
		CVSOCKET_ERR("[%s - %s - %d] select err\n",__FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	//LOGI("snd 0x%x sndlen %d nfdNums %d\n", _socket, nSendLen, nfdNums);
	return nSendLen;
}

int cv_tcpRead(char* pBuf, int nMaxSize, int socketFd, int nTimeOut)
{

	int nRevLen = 0;
	struct timeval timeoutRev;
	fd_set readfds; 
	int ret;
	
	FD_ZERO(&readfds);
	FD_SET(socketFd,&readfds);
	timeoutRev.tv_sec = nTimeOut / 1000;
	timeoutRev.tv_usec = nTimeOut % 1000 * 1000;

	//Socket is non-blocking, but we still use select.
	//Becasue, select is more effective than calling recv directly.
	//Moreover, most of time client do not recv data from server.
	ret = select(socketFd + 1, &readfds, NULL, NULL, &timeoutRev);

	if(1==ret)
	{
		nRevLen = recv(socketFd, pBuf, nMaxSize, 0);

		if (nRevLen <= 0)
		{
			perror("err: ");
			CVSOCKET_ERR("[%s - %s - %d] recv err\n",__FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
	}
	else if(-1==ret)
	{
		CVSOCKET_ERR("[%s - %s - %d] select err\n",__FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	return nRevLen;
}


int cv_tcpClose(int socketFd)
{
	if(socketFd<0)
	{
		CVSOCKET_ERR("[%s - %s - %d]\n",__FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	close(socketFd);	
	return 0;
}






































