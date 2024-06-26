#pragma once
#include <windows.h>
#include "PortManager.h"
#include "PortScanner.h"
#include "ThreadManager.h"

class COMManager : public PortManager
{
public:
	COMManager(LogManager& log, EventManager eventManager, std::wstring portName, int baudRate, char dataBits = 8, char parity = 0, char stopBits = 0, eFlowControl flowControl = FC_NONE);
	virtual ~COMManager();

	virtual bool Open();
	virtual void Connect();
	virtual void Close();

	virtual void GetRecvData(char* data);
	virtual void GetRecvDone();
	virtual void SetBufferSize(int size);
	virtual int  GetBufferSize();

	virtual void Send(const char* msg, int len);
	virtual int  Recv(char* msg, int len);
	virtual void Purge() {}

private:
	void RecvThreadFunc();

	std::mutex			    _mtx;
	std::condition_variable _cv;
	PortScanner             _scanner;
	CThreadManager          _threadManager = CThreadManager();

	HANDLE                  _handle        = INVALID_HANDLE_VALUE;
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

	int	                    _buffSize;
	char*                   _recvBuff;
	int	                    _recvLen       = 0;
};