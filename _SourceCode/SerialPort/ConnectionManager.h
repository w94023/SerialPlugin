#pragma once
#include <thread>

#include "EventManager.h"
#include "DAQCom.h"
#include "COMManager.h"
#include "BTClassicManager.h"
#include "BLEManager.h"

struct ConnectionConfig
{
	DeviceType deviceType = USB_COM;

	std::wstring deviceName;
	std::wstring uuidService;
	std::wstring uuidTx;
	std::wstring uuidRx;
	
	int comBps;
	int comDataBits;
	int comParity;
	int comStopBits;
	int comFlowControl;

	std::vector<std::string> ports;
	int portsNum;

	int    recvBuffSize = DEFAULT_BUFF_SIZE;
	int    PPSLimit     = -1; // Hz
	double CPULimit     = -1;  // 현재 사용량 %
	int    MemoryLimit  = -1; // 남은 MB

	int   connectionTimeout = 5000;

	bool isConnected = false;
};

class ConnectionManager
{
public:
	ConnectionManager(LogManager& log, const wchar_t* label) : _log(log), _scanner(log)
	{
		_label = label;
	}

	~ConnectionManager()
	{
		Disconnect();
	}

	void SetConfig(const wchar_t* deviceName)
	{
		_config.deviceName = deviceName;
		_config.deviceType = BTClassic;
	}

	void SetConfig(const wchar_t* deviceName, const wchar_t* uuidService, const wchar_t* uuidTx, const wchar_t* uuidRx)
	{
		_config.deviceName  = deviceName;
		_config.uuidService = uuidService;
		_config.uuidTx      = uuidTx;
		_config.uuidRx      = uuidRx;
		_config.deviceType  = BLE;
	}

	void SetConfig(const wchar_t* comPort, int comBps, int comDataBits, int comParity, int comStopBits, int comFlowControl)
	{
		_config.deviceName     = comPort;
		_config.comBps         = comBps;
		_config.comDataBits    = comDataBits;
		_config.comParity      = comParity;
		_config.comStopBits    = comStopBits;
		_config.comFlowControl = comFlowControl;
		_config.deviceType     = USB_COM;
	}

	void SetConfig(const wchar_t* deviceName, char* ports, int portsNum)
	{
		_config.deviceName = deviceName;
		for (size_t i = 0; i < portsNum; i++) {
			_config.ports.push_back(std::to_string(ports[i]));
		}
		_config.portsNum   = portsNum;
		_config.deviceType = NI_DAQ;
	}

	void RegisterEvents(EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected, EventCallback onDataReceived)
	{
		_eventManager.RegisterEventCallbacks(onConnected, onConnectionFailed, onDisconnected, onDataReceived);
	}

	void Connect()
	{
		std::thread _thread([this]() {
			_threadStarted = true;
			_log.Developer(L"Thread started [label] : " + _label + L" [deviceName] : " + _config.deviceName);

			_log.Developer(L"Open - [deviceName] : " + _config.deviceName);
			switch (_config.deviceType) {
				case BTClassic: _port = new BTClassicManager(std::ref(_log), _eventManager, [this]() { this->Disconnect(); }, _config.deviceName); break;
				case BLE:	    _port = new BLEManager(std::ref(_log), _eventManager, [this]() { this->Disconnect(); }, _config.deviceName, _config.uuidService, _config.uuidTx, _config.uuidRx); break;
				case USB_COM:	_port = new COMManager(std::ref(_log), _eventManager, [this]() { this->Disconnect(); }, _config.deviceName, _config.comBps, 
					(char)_config.comDataBits, (char)_config.comParity, (char)_config.comStopBits, (eFlowControl)_config.comFlowControl); break;
				case NI_DAQ:	_port = new CDAQCom(std::ref(_log), _eventManager, [this]() { this->Disconnect(); }, _config.deviceName, _config.ports, _config.portsNum); break;
			}
			Open();

			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]{ return _stopThreadRequest; });
			_log.Developer(L"Thread stop request accepted [deviceName] : " + _config.deviceName + L" [label] : " + _label);
			Close();

			_stopThreadResponse = true;
			_cv.notify_all();
			_log.Developer(L"Thread stop responsed [deviceName] : " + _config.deviceName + L" [label] : " + _label);

			_log.Developer(L"Thread stopped [label] : " + _label + L" [deviceName] : " + _config.deviceName);
		});
		_thread.detach();
	}

	void Open()
	{
		_isConnecting = true;
		if (_port->Open()) {
			_log.Developer(L"OpenConnection - connect [deviceName] : " + _config.deviceName);
			_port->Connect();
			_isConnected = true;

			if (_config.recvBuffSize > 0) {
				_log.Developer(L"Set buffer size [deviceName] : " + _config.deviceName + L" [size] : " + std::to_wstring(_config.recvBuffSize));
				_port->SetRecvBufferSize(_config.recvBuffSize);
			}

			SetResourceLimit(PPS,    _config.PPSLimit);
			SetResourceLimit(CPU,    _config.CPULimit);
			SetResourceLimit(Memory, _config.MemoryLimit);

			_log.Developer(L"Set connection timeout [deviceName] : " + _config.deviceName + L" [timeout] : " + std::to_wstring(_config.connectionTimeout));
			_port->SetConnecitonTimeout(_config.connectionTimeout);

			_isConnecting = false;
		}
		else {
			_log.Developer(L"OpenConnection - failed to open [deviceName] : " + _config.deviceName);
			_isConnected = false;
			_isConnecting = false;
		}
	}

	void Close()
	{
		_log.Developer(L"CloseConnection - [deviceName]" + _config.deviceName);
		_port->Close();
		delete _port;

		_isConnected = false;
	}

	void Disconnect()
	{
		if (!_threadStarted) return;
		if (_stopThreadRequest) return;
		if (_stopThreadResponse) return;

		std::thread _thread([this]() {
			if (_isConnecting) {
				_port->ForceClose();
			}

			_stopThreadRequest = true;
			_cv.notify_all();
			_log.Developer(L"Thread stop requested [deviceName] : " + _config.deviceName + L" [label] : " + _label);

			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]{ return _stopThreadResponse; });
			_log.Developer(L"Thread stop response accepted [deviceName] : " + _config.deviceName + L" [label] : " + _label);
		});
		_thread.join();
	}

	void Send(const char* data, int length)
	{
		if (!_isConnected) return;
		_port->SendData(data, length);
	}

	void SetConnectionTimeout(int timeout)
	{
		_config.connectionTimeout = timeout;

		if (!_isConnected) return;

		//_log.Developer(L"Set connection timeout [deviceName] : " + _config.deviceName + L" [timeout] : " + std::to_wstring(_config.connectionTimeout));
		//_port->SetConnecitonTimeout(_config.connectionTimeout);
	}

	void SetRecvBufferSize(int size)
	{
		_config.recvBuffSize = size;

		if (!_isConnected) return;

		_log.Developer(L"Set buffer size [deviceName] : " + _config.deviceName + L" [size] : " + std::to_wstring(_config.recvBuffSize));
		_port->SetRecvBufferSize(_config.recvBuffSize);
	}

	int GetRecvBufferSize()
	{
		return _config.recvBuffSize;
	}

	void SetResourceLimit(ResourceLimitType resourceLimitType, double limit)
	{
		std::wstring typeHeader = L"";
		switch (resourceLimitType) {
			case PPS:    _config.PPSLimit    = (int)limit; typeHeader = L"PPS";    break;
			case CPU:    _config.CPULimit    =      limit; typeHeader = L"CPU";    break;
			case Memory: _config.MemoryLimit = (int)limit; typeHeader = L"Memory"; break;
		}

		if (!_isConnected) return;

		if (limit > 0) {
			_log.Developer(L"Set " + typeHeader + L" limit [deviceName] : " + _config.deviceName + L" [" + typeHeader + L" limit] : " + std::to_wstring(limit));
			_port->SetResourceLimit(resourceLimitType, limit);
		}
	}

	void CheckResources(double* cpuUsage, int* memoryRemained)
	{
		if (!_isConnected) {
			*cpuUsage       = -1;
			*memoryRemained = -1;
			return;
		}
		_port->CheckResources(cpuUsage, memoryRemained);
	}

	void Test(double cpuUsage, int memoryRemained)
	{
		if (!_isConnected) return;
		_port->Test(cpuUsage, memoryRemained);
	}

	int GetRecvData(char* data, int byteSize)
	{
		if (!_isConnected) return 0;
		else			   return _port->GetRecvData(data, byteSize);
	}

	void GetRecvDone()
	{
		if (!_isConnected) return;
		_port->GetRecvDone();
	}

	void ScanDevices()
	{
		_log.Developer(L"ScanDevice - start scan");
		_scanner.ScanDevices();
	}

	void RegisterOnScanEnded(EventCallback callback)
	{
		_scanner.onScanEnded = callback;
	}

	std::map<int, std::wstring> GetDeviceNameList()
	{
		return _scanner.GetDeviceNameList();
	}

	std::map<int, DeviceType> GetDeviceTypeList()
	{
		return _scanner.GetDeviceTypeList();
	}

private:
	std::wstring			_label;

	LogManager&             _log;
	ConnectionConfig		_config;
	EventManager			_eventManager;
	PortScanner				_scanner;
	PortManager*            _port = nullptr;
	std::mutex              _mtx;
	std::condition_variable _cv;

	bool _isConnecting       = false;
	bool _isConnected        = false;
	bool _threadStarted      = false;
	bool _stopThreadRequest  = false;
	bool _stopThreadResponse = false;
};