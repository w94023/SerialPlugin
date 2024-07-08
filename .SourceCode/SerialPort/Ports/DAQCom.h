#pragma once
#include "NIDAQmx.h"
#include "PortManager.h"

#define MAX_PORT_NUM 30

using namespace std;

class CDAQCom : public PortManager
{
public:
	CDAQCom(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc);
	~CDAQCom();

	bool Open()	   override;
	bool Connect() override;
	void Close()   override;

	int  SendData(const double* values, int len) override;
	int  SendData(const int*    values, int len) override;

private:
	std::string GetPortStr(int* ports, int count, const wchar_t* type);
	std::string GetPortStr(int* ports, int count, int* lines, int lineCount);
	static int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
    void NonStaticEveryNCallback(float64* data, int32 read);
    void ErrorHandler();

	TaskHandle _taskHandle = 0;
	TaskHandle _writingTask = 0;
	TaskHandle _digitalTask = 0;

	std::wstring _deviceName;
	//char*        _ports;
	std::vector<std::string> _ports;
	int			 _portsNum = 0;

	bool _isConnected = false;

	char*        _recvBuff;
	int	         _recvLen;
				 
	char*        _errBuff;
	int32        _error      = 0;};