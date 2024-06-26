#pragma once
#include <windows.h>
#include "PortManager.h"
#include "PortScanner.h"

class COMManager : public PortManager
{
public:
	COMManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc);
	virtual ~COMManager();

	bool Open()	   override;
	bool Connect() override;
	void Close()   override;

	int  SendData(const char* msg, int len) override;
	int  RecvData(char* data, int len)      override;

private:
	std::mutex			    _mtx;
	std::condition_variable _cv;
	PortScanner             _scanner;

	HANDLE                  _comHandle        = INVALID_HANDLE_VALUE;
	OVERLAPPED              _wOverlapped   = { 0 };
	OVERLAPPED              _rOverlapped   = { 0 };

	std::wstring            _portName;
	int                     _baudRate;
	char                    _dataBits;
	char                    _stopBits;
	char                    _parity;
	eFlowControl            _flowControl;

	bool                    _dataReady     = true;
	bool                    _recvReady     = true;
	bool                    _isAlive       = true;
	bool                    _isConnecting  = false;
};