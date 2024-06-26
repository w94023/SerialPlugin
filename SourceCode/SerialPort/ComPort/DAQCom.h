#pragma once
#include "NIDAQmx.h"
#include "PortManager.h"

#define MAX_PORT_NUM 30

using namespace std;

class CDAQCom : public PortManager
{
public:
	CDAQCom(LogManager& log, EventManager eventManager, std::wstring deviceName, std::vector<std::string> ports, int portsNum);
	~CDAQCom();

	virtual bool Open();
	virtual void Connect() { }
	virtual void Close();

	virtual void GetRecvData(char* data);
	virtual void GetRecvDone() { }
	virtual void SetBufferSize(int size) { }
	virtual int  GetBufferSize() { return -1; }

	virtual void Send(const char* msg, int len) { }
	virtual int  Recv(char* msg, int len) { return -1; }
	virtual void Purge() { }

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