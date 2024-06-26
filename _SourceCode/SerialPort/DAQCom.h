#pragma once
#include "NIDAQmx.h"
#include "PortManager.h"

#define MAX_PORT_NUM 30

using namespace std;

class CDAQCom : public PortManager
{
public:
	CDAQCom(LogManager& log, EventManager eventManager, std::function<void()> closeFunc, std::wstring deviceName, std::vector<std::string> ports, int portsNum);
	~CDAQCom();

	bool Open()									   override;
	void Close()								   override;

	int  GetRecvData(char* data, int recvByteSize) override;

private:

	static int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
    void NonStaticEveryNCallback(float64* data, int32 read);
    void ErrorHandler();

	std::wstring _deviceName;
	//char*        _ports;
	std::vector<std::string> _ports;
	int			 _portsNum = 0;

	char*        _recvBuff;
	int	         _recvLen;
				 
	char*        _errBuff;
	int32        _error      = 0;
	TaskHandle   _taskHandle = 0;
};