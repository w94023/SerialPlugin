#include "DAQCom.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <stdlib.h>

#define DAQmxErrChk(functionCall) if (DAQmxFailed(_error=(functionCall))) goto Error; else

CDAQCom::CDAQCom(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) :
	PortManager(log, config, dataManager, threadManager, closeFunc)
{
	// double ���� �ϳ� �� 8 byte �ʿ�
	_recvBuff = new char[_config.AIPortsCount * 8];

	// stopByte, packetLength ���� �ʱ�ȭ
	_dataManager.ResetConfig();
}

CDAQCom::~CDAQCom()
{
	Close();
}

bool CDAQCom::Open()
{
	if (_config.GetDeviceName().substr(0, 3) != L"Dev") {
		_log.Error(L"��ȿ���� ���� DAQ ��ġ �̸��Դϴ� : %s", _config.deviceName);
        return false;
    }

	for (size_t i = 3; i < _config.GetDeviceName().size(); ++i) {
        if (!std::isdigit(_config.GetDeviceName()[i])) {
			_log.Error(L"��ȿ���� ���� DAQ ��ġ �̸��Դϴ� : %s", _config.deviceName);
            return false;
        }
    }

	if (_config.AIPortsCount == 0 && _config.AOPortsCount == 0) {
		_log.Error(L"DAQ ��ġ�� ������ ��Ʈ�� ������ 0�� �Դϴ�");
		return false;
	}

	_log.Developer(L"[%s, %d] DAQManager::Open() - ����̽� �˻� ����", _config.deviceName, handle);
	_log.Normal(L"DAQ ��ġ �˻��� �����߽��ϴ� : %s", _config.deviceName);

	// AI ��Ʈ ����
	std::string AIPortsStr = GetPortStr(_config.AIPorts, _config.AIPortsCount, L"ai");
	std::string AOPortsStr = GetPortStr(_config.AOPorts, _config.AOPortsCount, L"ao");
	std::string DPortsStr = GetPortStr(_config.DPorts, _config.DPortsCount, _config.lines, _config.linesCount);

	_log.Developer(L"[%s, %d] DAQManager::Open() - [AIPorts] %s", _config.deviceName, handle, CharToWChar(AIPortsStr.c_str()));
	_log.Developer(L"[%s, %d] DAQManager::Open() - [AOPorts] %s", _config.deviceName, handle, CharToWChar(AOPortsStr.c_str()));
	_log.Developer(L"[%s, %d] DAQManager::Open() - [DPorts] %s",  _config.deviceName, handle, CharToWChar(DPortsStr.c_str()));

	// �½�ũ ����
	DAQmxErrChk(DAQmxCreateTask("", &_taskHandle));
	// AI ��Ʈ ����
	DAQmxErrChk(DAQmxCreateAIVoltageChan(_taskHandle, AIPortsStr.c_str(), "", DAQmx_Val_RSE, -10.0, 10.0, DAQmx_Val_Volts, NULL));
	// Sampling rate ����
	DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandle, "", 1000, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
	// ������ ���� �ڵ鷯 ���
	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(_taskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, this));
	DAQmxErrChk(DAQmxStartTask(_taskHandle));
	
	// AO ��Ʈ ����
	DAQmxErrChk(DAQmxCreateTask("", &_writingTask));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(_writingTask, AOPortsStr.c_str(), "", 0.0, 5.0, DAQmx_Val_Volts, NULL));

	// Digital output ��Ʈ ����
	DAQmxErrChk(DAQmxCreateTask("", &_digitalTask));
	DAQmxErrChk(DAQmxCreateDOChan(_digitalTask, DPortsStr.c_str(), "", DAQmx_Val_ChanForAllLines));
	
	_isConnected = true;
	_log.Normal(L"DAQ ��ġ ���ῡ �����߽��ϴ� : %s", _config.deviceName);
	return true;

Error:
	_isConnected = false;
	_log.Normal(L"DAQ ��ġ ���ῡ �����߽��ϴ� : %s", _config.deviceName);

	ErrorHandler();
	return false;
}

bool CDAQCom::Connect()
{
	return true;
}

std::string CDAQCom::GetPortStr(int* ports, int count, const wchar_t* type)
{
	// DAQ ���̺귯���� ���޵� device name �� channel info ����
	// type�� L"ai", Ȥ�� L"ao"���� ��
	// device name�� Dev2, Ÿ�� ��Ʈ�� ai0, ai2�� ���, "Dev2/ai0,Dev2/ai2"�� ��ȯ
	std::wstring portsStr;
	for (int i = 0; i < count; i++) {
		portsStr += _config.GetDeviceName();
		portsStr += L"/";
		portsStr += type;
		portsStr += std::to_wstring(ports[i]);

		if (i != count-1) {
			portsStr += L",";
		}
	}

	return std::string(portsStr.begin(), portsStr.end());
}

std::string CDAQCom::GetPortStr(int* ports, int count, int* lines, int lineCount)
{
	std::wstring portsStr;
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < lineCount; j++) {
			portsStr += _config.GetDeviceName();
			portsStr += L"/port";
			portsStr += std::to_wstring(ports[i]);
			portsStr += L"/line";
			portsStr += std::to_wstring(lines[j]);

			if (i != count-1 || j != lineCount - 1) {
				portsStr += L",";
			}
		}
	}

	return std::string(portsStr.begin(), portsStr.end());
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
	char* recvData = new char[_config.AIPortsCount * 8];
	for (int i = 0; i < _config.AIPortsCount; i++) {
		memcpy(recvData + 8 * i, &data[i], sizeof(float64));
	}

	_dataManager.SetData(recvData, _config.AIPortsCount * 8);

	delete[] recvData;
}

void CDAQCom::Close()
{
	if (!_isConnected) return;

	delete[] _recvBuff;
	delete[] _errBuff;

	if (_taskHandle != 0) {
		_isConnected = false;

        DAQmxStopTask(_taskHandle);
        DAQmxClearTask(_taskHandle);
    }

	if (_writingTask != 0) {
		double* value = new double[_config.AOPortsCount];
		for (int i = 0; i < _config.AOPortsCount; i++) {
			value[i] = 0;
		}
		SendData(value, _config.AOPortsCount);
		delete[] value;

        DAQmxClearTask(_writingTask);
	}

	if (_digitalTask != 0) {
		int count = _config.DPortsCount * _config.linesCount;
		int* value = new int[count];
		for (int i = 0; i < count; i++) {
			value[i] = 0;
		}
		SendData(value, count);
		delete[] value;

        DAQmxClearTask(_digitalTask);
	}

	_log.Normal(L"DAQ ��ġ ������ �����߽��ϴ� : %s", _config.deviceName);
}

int CDAQCom::SendData(const double* values, int len)
{
	if (len != _config.AOPortsCount) {
		_log.Error(L"�����Ϸ��� AO ��Ʈ�� ���� ������ AO ��Ʈ�� ���� ��ġ���� �ʽ��ϴ� : %s", _config.deviceName);
		return 0;
	}

    int32 error = 0;
    char errBuff[2048] = {'\0'};

    // �Ƴ��α� ��� ä�ο� ���� �� ����
    error = DAQmxWriteAnalogF64(_writingTask, 1, true, 10.0, DAQmx_Val_GroupByChannel, values, NULL, NULL);
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        std::cerr << "DAQmxWriteAnalogF64 Error: " << errBuff << std::endl;
        DAQmxClearTask(_writingTask);
        return 0;
    }

	return 1;
}

int CDAQCom::SendData(const int* values, int len)
{
	if (len != _config.DPortsCount * _config.linesCount) {
		_log.Error(L"�����Ϸ��� ������ ��Ʈ�� ���� ������ ������ ��Ʈ�� ���� ��ġ���� �ʽ��ϴ� : %s", _config.deviceName);
		return 0;
	}

	// int �����͸� uInt8 �����ͷ� ��ȯ
	uInt8* data = new uInt8[len];
	for (int i = 0; i < len; i++) {
		if (values[i] == 1) data[i] = 0x01;
		else                data[i] = 0x00;
	}

	int32 error = 0;
    char errBuff[2048] = {'\0'};

	error = DAQmxWriteDigitalLines(_digitalTask, 1, true, 10.0, DAQmx_Val_GroupByChannel, data, NULL, NULL);
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        std::cerr << "DAQmxWriteDigitalLines Error: " << errBuff << std::endl;
        DAQmxClearTask(_digitalTask);
        return 0;
    }

	delete[] data;
	return 1;
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
		_log.Developer(L"CDAQCom::ErrorHandler - error handler : %s", CharToWString(_errBuff));
		std::cout << _errBuff << std::endl;
	}
}