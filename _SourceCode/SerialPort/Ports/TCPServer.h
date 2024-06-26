#pragma once
#include <windows.h>
#include "PortManager.h"

class TCPServer : public PortManager
{
public:
	TCPServer(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc);
	//virtual ~TCPServer();

	bool Open()																 override;
	bool Connect()															 override;
	void Close()															 override;

	int  SendData(const char* msg, int len)                                  override;
	int  RecvData(char* data, int len)                                       override;

private:
	void OnSocketError(const wchar_t *prefix);
	void CloseSocket();

	DWORD  _localIp;
	DWORD  _remoteIp;
	DWORD  _timeout = 250; //ms
	SOCKET _sockfd = INVALID_SOCKET;
	bool   _connected = false;
	//void RecvThreadFunc(bool doWork);

	//std::mutex			    _mtx;
	//std::condition_variable _cv;
	//PortScanner             _scanner;

	//HANDLE                  _handle        = INVALID_HANDLE_VALUE;
	//OVERLAPPED              _wOverlapped   = { 0 };
	//OVERLAPPED              _rOverlapped   = { 0 };

	//std::chrono::time_point<std::chrono::steady_clock> _dataReceivedTimePoint;

	//bool                    _dataReady     = true;
	//bool                    _recvReady     = true;
	//bool                    _isAlive       = true;
	//bool                    _isConnecting  = false;
	//	                    
	//int                     _recvBufferSize = DEFAULT_BUFF_SIZE;
	//char					_recvBuffTemp[DEFAULT_BUFF_SIZE] { 0 };
	//char                    _recvBuff	 [DEFAULT_BUFF_SIZE] { 0 };
	//int					    _recvByteForRead = 1;
	//int	                    _recvLen = 0;
};