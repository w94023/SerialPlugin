#pragma once
#include <thread>

#include "StructManager.h"
#include "DAQCom.h"
#include "COMManager.h"
#include "BTClassicManager.h"
#include "BLEManager.h"
#include "TCPServer.h"

class ConnectionManager
{
public:
	ConnectionManager(LogManager& log, int handle);
	~ConnectionManager();

	void SetDeviceConfig(ConnectionConfig config);

	void Connect();
	void Open();
	void Close();
	void Disconnect();

	void RecvThreadFunc(bool doWork);
	void SendData(const char*   data, int length);
	void SendData(const double* data, int length);
	void SendData(const int*    data, int length);

	void RegisterDataEvent(EventCallbackWithChar onDataReceived);
	void RegisterEvents(EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected);

	void SetPacketConfig(PacketConfig packetConfig);
	void SetResourceConfig(ResourceConfig resourceConfig);

	void CheckResources(double* cpuUsage, int* memoryRemained);

	void GetReceivedBufferLength(int* length);

private:
	int          			_handle;

	LogManager&             _log;
	ConnectionConfig		_config;
	PortScanner				_scanner;
	PortManager*            _port = nullptr;
	std::mutex              _mtx;
	std::condition_variable _cv;

	DataManager _dataManager;
	CThreadManager _threadManager;
	CThreadManager _threadManager2;

	bool _isConnecting       = false;
	bool _isConnected        = false;
	bool _threadStarted      = false;
	bool _isOpenFinshed      = false;
	bool _stopConnectionLoop = false;
	bool _stopThreadRequest  = false;
	bool _stopThreadResponse = false;

	EventCallback         _onConnected;
	EventCallback         _onConnectionFailed;
	EventCallback         _onDisconnected;
	EventCallbackWithChar _onDataReceived;

	std::chrono::time_point<std::chrono::steady_clock> _connectionStartPoint;

	int         _recvBufferSize = DEFAULT_BUFF_SIZE;
	char		_recvBuffTemp[DEFAULT_BUFF_SIZE] { 0 };
	char        _recvBuff	 [DEFAULT_BUFF_SIZE] { 0 };
	int			_recvByteForRead = 1;
	int	        _recvLen = 0;
};