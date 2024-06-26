#pragma once
#include <windows.h>
#include "PortManager.h"
#include "PortScanner.h"
#include "ThreadManager.h"

class BTClassicManager : public PortManager
{
public:
	BTClassicManager(LogManager& log, EventManager eventManager, std::function<void()> closeFunc, std::wstring deviceName);
	virtual ~BTClassicManager();

	bool Open()																 override;
	void Connect()															 override;
	void Close()															 override;
	void ForceClose()														 override;
																			 
	int  GetRecvData(char* data, int byteSize)								 override;
	void GetRecvDone()														 override;
	void SetRecvBufferSize(int size)										 override;
	int  GetRecvBufferSize()												 override;

	void SetResourceLimit(ResourceLimitType resourceLimitType, double limit) override;
	void CheckResources(double* cpuUsage, int* memoryRemained)               override;
	void Test(double cpuUsage, int memoryRemained)                           override;

	int  SendData(const char* msg, int len)                                  override;
	int  RecvData()                                                          override;

private:
	void RecvThreadFunc(bool doWork);

	std::mutex			    _mtx;
	std::condition_variable _cv;

	PortScanner             _scanner;
	CThreadManager          _threadManager;

	HANDLE                  _handle        = INVALID_HANDLE_VALUE;
	OVERLAPPED              _wOverlapped   = { 0 };
	OVERLAPPED              _rOverlapped   = { 0 };

	std::chrono::time_point<std::chrono::steady_clock> _dataReceivedTimePoint;

	std::wstring            _deviceName;

	bool                    _dataReady     = true;
	bool                    _recvReady     = true;
	bool                    _isAlive       = true;
	bool                    _isConnecting  = false;
		                    
	int                     _recvBufferSize = DEFAULT_BUFF_SIZE;
	char					_recvBuffTemp[DEFAULT_BUFF_SIZE] { 0 };
	char                    _recvBuff	 [DEFAULT_BUFF_SIZE] { 0 };
	int					    _recvByteForRead = 1;
	int	                    _recvLen = 0;
};