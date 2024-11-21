#pragma once
#include <windows.h>
#include "PortManager.h"
#include "PortScanner.h"

class BTClassicManager : public PortManager
{
public:
	BTClassicManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc);
	virtual ~BTClassicManager();

	bool Open()			override;
	bool Connect()		override;
	void Close()		override;
	void ForceClose()	override;

	int SendData(const char* msg, int len) override;
	int RecvData(char* data, int len)      override;

private:
	std::mutex			    _mtx;
	std::condition_variable _cv;
	PortScanner             _scanner;

	HANDLE                  _handle        = INVALID_HANDLE_VALUE;
	OVERLAPPED              _wOverlapped   = { 0 };
	OVERLAPPED              _rOverlapped   = { 0 };

	std::chrono::time_point<std::chrono::steady_clock> _dataReceivedTimePoint;

	bool                    _dataReady     = true;
	bool                    _recvReady     = true;
	bool                    _isAlive       = true;
	bool                    _isConnecting  = false;
};