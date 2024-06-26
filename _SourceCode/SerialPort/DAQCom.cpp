#include "DAQCom.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <stdlib.h>

#define DAQmxErrChk(functionCall) if (DAQmxFailed(_error=(functionCall))) goto Error; else

CDAQCom::CDAQCom(LogManager& log, EventManager eventManager, std::function<void()> closeFunc, std::wstring deviceName, std::vector<std::string> ports, int portsNum) :
	PortManager(log, eventManager, closeFunc)
{
	_deviceName = deviceName;
	_portsNum = portsNum;
	_ports = ports;
	
	// double 형은 하나 당 8 byte 필요
	_recvBuff = new char[_portsNum * 8];
}

CDAQCom::~CDAQCom()
{
	Close();
}

bool CDAQCom::Open()
{
	if (_deviceName.substr(0, 3) != L"Dev") {
		_log.Error(L"Invalid device name : " + _deviceName);
        return false;
    }

	for (size_t i = 3; i < _deviceName.size(); ++i) {
        if (!std::isdigit(_deviceName[i])) {
			_log.Error(L"Invalid port name : " + _deviceName);
            return false;
        }
    }

	_log.Developer(L"DAQManager::Open() : [deviceName]" + _deviceName);
	_log.Normal(L"Device found : " + _deviceName);

	// Set name of physical channel
	// If target device name = "Dev2", target ports = ai0, ai2
	// channel name = "Dev2/ai0,Dev2/ai2"
	std::string channelName;
	std::string deviceName(_deviceName.begin(), _deviceName.end());
	for (int i = 0; i < _portsNum; i++) {
		channelName += deviceName;
		channelName += "/ai";
		channelName += _ports[i];

		if (i != _portsNum-1) {
			channelName += ",";
		}
	}

	_log.Developer(L"DAQManager::Open() : [channelName]" + std::wstring(channelName.begin(), channelName.end()));
	
	// Open DAQ port using NIDAQmx.h, NIDAQmx.lib
	DAQmxErrChk(DAQmxCreateTask("", &_taskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(_taskHandle, channelName.c_str(), "", DAQmx_Val_RSE, -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandle, "", 1000, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(_taskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, this));
	DAQmxErrChk(DAQmxStartTask(_taskHandle));

	_eventManager.Invoke("onConnected");
	return true;

Error:
	_eventManager.Invoke("onConnectionFailed");
	ErrorHandler();
	return false;
}

int32 CDAQCom::EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	float64 localData[MAX_PORT_NUM];
    int32   localRead;

	// Read the data inside the callback
    DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByChannel, localData, MAX_PORT_NUM, &localRead, nullptr);

	CDAQCom* instance = static_cast<CDAQCom*>(callbackData);
    instance->NonStaticEveryNCallback(localData, localRead);

    return 0;
}

void CDAQCom::NonStaticEveryNCallback(float64* data, int32 read)
{
	for (int i = 0; i < _portsNum; i++) {
		memcpy(_recvBuff + 8 * i, &data[i], sizeof(float64));
	}
	_recvLen = 8 * _portsNum;
	_eventManager.Invoke("onDataReceived");
}

void CDAQCom::Close()
{
	delete[] _recvBuff;
	delete[] _errBuff;

	if (_taskHandle != 0) {
		_eventManager.Invoke("onDisconnected");
        DAQmxStopTask(_taskHandle);
        DAQmxClearTask(_taskHandle);
    }
}

int CDAQCom::GetRecvData(char* data, int recvByteSize)
{
	memcpy(data, _recvBuff, _recvLen);
	return _recvLen;
}

void CDAQCom::ErrorHandler()
{
	if (DAQmxFailed(_error))
		DAQmxGetExtendedErrorInfo(_errBuff, 2048);

	if (_taskHandle != 0) {
		DAQmxStopTask(_taskHandle);
		DAQmxClearTask(_taskHandle);
	}
	if (DAQmxFailed(_error)) {
		_log.Error(L"Failed to connect DAQ device : " + _deviceName);
		std::string errBuff(_errBuff);
		_log.Developer(L"CDAQCom::ErrorHandler - error handler [_errBuff]" + std::wstring(errBuff.begin(), errBuff.end()));
	}
}