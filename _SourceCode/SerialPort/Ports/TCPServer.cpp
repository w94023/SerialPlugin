#include <winsock2.h>
#include <assert.h>
#include <mstcpip.h>

#include "TCPServer.h"
#include "SockCommon.h"

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;
typedef unsigned int   uint;

void TCPServer::OnSocketError(const wchar_t *prefix)
{
	int errCode = WSAGetLastError();
	LPWSTR errString = NULL;

	int size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0, errCode, 0, (LPWSTR)&errString, 0, 0); 

	_log.Developer(L"[%s, %d] %s [Error code] %d : %s", 
		_config.deviceName, handle, prefix, errCode, errString);

	LocalFree(errString); 
}

TCPServer::TCPServer(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) : 
	PortManager(log, config, dataManager, threadManager, closeFunc)
{
	_localIp = 0;
	_remoteIp = 0;
}

bool TCPServer::Open()
{
	WSADATA wsaData;
    int result;

    // Winsock 초기화
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
		_log.Developer(L"[%s, %d] TcpServer::Open - WSAStartup 메서드 에러", _config.deviceName, handle);
        return false;
    }

	bool opt_reuseaddr = true;

	_log.Developer(L"[%s, %d] TcpServer::Open - [remoteIP]%s, [port]%d", _config.deviceName, handle, ip_addr_str(_localIp), _config.port);
	if (_sockfd != INVALID_SOCKET) {
		_log.Developer(L"[%s, %d] TcpServer::Open - 이미 소켓이 열려 있습니다", _config.deviceName, handle);
		Close ();
	}

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd == INVALID_SOCKET) {
		OnSocketError (L"TcpServer::Open - socket 메서드 에러 :");
		goto CLOSE_RET;
	}

	// 소켓에서 사용하던 Port No를 재사용 가능하도록 한다.
	// listen하던 소켓을 닫은 후 새로 연 경우 한동안 bind ()가 안되는 현상을 막기위해 설정한다.
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_reuseaddr, sizeof(opt_reuseaddr)) == SOCKET_ERROR) {
		OnSocketError(L"TcpServer::Open - setsockopt 메서드 에러 :");
		goto CLOSE_RET;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((WORD)_config.port);

	if (::bind(_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		OnSocketError(L"TcpServer::Open - bind 메서드 에러 :");
		goto CLOSE_RET;
	}

	if (listen(_sockfd, 1) == SOCKET_ERROR) {
		OnSocketError(L"TcpServer::Open - listen 메서드 에러 :");
		goto CLOSE_RET;
	}

	_log.Normal(L"TCP 서버가 생성되었고, 연결을 기다리는 중입니다.");
	_log.Developer(L"[%s, %d] TCPServer::Connect - 서버가 생성되었고, 연결을 기다리는 중입니다 [localIP]%s, [port]%d", 
		_config.deviceName, handle, ip_addr_str(_localIp), _config.port);

	_connected = false;
	return true;

CLOSE_RET:
	if (_sockfd != INVALID_SOCKET) {
		closesocket(_sockfd);
	}
	_sockfd = INVALID_SOCKET;
	return false;
}

void TCPServer::Close()
{
	//TRACE ("INFO: CTcpServer::Close (): %s, %d\n", ip_addr_str(_remoteIp), (int)_port);
	if (_sockfd != INVALID_SOCKET) {
		shutdown(_sockfd, SD_BOTH);
		closesocket(_sockfd);
		_sockfd = INVALID_SOCKET;
		_connected = false;
		return;
	}
	_connected = false;

	WSACleanup();

	return;
}

bool TCPServer::Connect()
{
	if (_sockfd == INVALID_SOCKET) Open();

	// Set the socket to non-blocking mode.
	int nonblocking = 1;
	if (ioctlsocket(_sockfd, FIONBIO, (ulong *)&nonblocking) == SOCKET_ERROR) {
		OnSocketError(L"TcpServer::Connect - ioctlsocket 메서드 에러 :");
		CloseSocket();
		return false;
	}

	struct sockaddr_in client_addr;
	int clilen = sizeof (client_addr);

	int connfd = accept(_sockfd, (struct sockaddr *)&client_addr, &clilen);

	if (connfd == INVALID_SOCKET) {
		int error = WSAGetLastError();
		if (error == WSAETIMEDOUT || error == WSAEWOULDBLOCK) {
			// 아직 연결하지 못함
			// socket이 사용가능할 때까지 기다린다.
			//SocketReady(_sockfd, _timeout);
			return false;
		}
		else {
			OnSocketError(L"TcpServer::Connect - accept 메서드 에러 :");
			CloseSocket();
			return false;
		}
	}
	else {
		closesocket(_sockfd);
		_sockfd = connfd;

		DWORD dwRet = 0;
		struct tcp_keepalive ka;
		ka.onoff = 1;
		ka.keepalivetime = 1000;	
		ka.keepaliveinterval = 1000;
		if (WSAIoctl(_sockfd, SIO_KEEPALIVE_VALS, &ka, sizeof(ka),NULL, 0, &dwRet, NULL, NULL) == SOCKET_ERROR) {
			OnSocketError(L"TcpServer::Connect - WSAIoctl 메서드 에러 :");
			CloseSocket();
			return false;
		}

		// Set the socket to blocking mode.
		int nonblocking = 0;
		if (ioctlsocket(_sockfd, FIONBIO, (ulong *)&nonblocking) == SOCKET_ERROR) {
			OnSocketError(L"TcpServer::Connect - ioctlsocket 메서드 에러 :");
			CloseSocket();
			return false;
		}

		// Set the status for the keepalive option 
		int optval = 1;
		if (setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval)) == SOCKET_ERROR) {
			OnSocketError(L"TcpServer::Connect - setsockopt(SO_KEEPALIVE) 메서드 에러 :");
			CloseSocket();
			return false;
		}

		// socket의 opetion (sending/receiving timeout 값) 설정
		if (setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(_timeout)) == SOCKET_ERROR) {
			OnSocketError(L"TcpServer::Connect - setsockopt(SO_RCVTIMEO) 메서드 에러 :");
			CloseSocket();
			return false;
		}
		if (setsockopt(_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&_timeout, sizeof(_timeout)) == SOCKET_ERROR) {
			OnSocketError(L"TcpServer::Connect - setsockopt(SO_SNDTIMEO) 메서드 에러 :");
			CloseSocket();
			return false;
		}

		_connected = true;
		_log.Developer(L"[%s, %d] TcpServer::Connect - 소켓 연결 완료", _config.deviceName, handle);
		return true;
	}
}

void TCPServer::CloseSocket()
{
	if (_sockfd != INVALID_SOCKET) {
			closesocket (_sockfd);
		}
		_sockfd = INVALID_SOCKET;
}

int TCPServer::SendData(const char *msg, int len)
{
	if (_sockfd == INVALID_SOCKET) return -1;
	if (!_connected) return -1;

	int ret = send (_sockfd, msg, len, 0);
	if (ret == 0) {
		Close ();
		return 0;
	}
	else if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAETIMEDOUT) return 0;

		OnSocketError(L"TcpServer::SendData :");
		Close ();
		return -1;
	}
	return ret;
}

int TCPServer::RecvData(char *msg, int len)
{
	if (_sockfd == INVALID_SOCKET) return -1;
	if (!_connected) return -1;

	int ret = recv (_sockfd, msg, len, 0);

	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAETIMEDOUT) return 0;

		OnSocketError(L"TcpServer::RecvData :");
		Close ();
		return -1;
	}

	return ret;
}
//
//DWORD CTcpServer::GetRemoteIP (WORD *port)
//{
//	struct sockaddr_in client_addr;
//	int addrlen = sizeof(client_addr);
//	
//	int ret = getpeername (_sockfd, (struct sockaddr *)&client_addr, &addrlen);
//	if (ret != SOCKET_ERROR) {
//		if (port) *port = client_addr.sin_port;
//		return ntohl(client_addr.sin_addr.s_addr);
//	}
//	return 0;
//}