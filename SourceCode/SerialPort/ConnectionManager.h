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

	int recvBuffSize = DEFAULT_BUFF_SIZE;

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
		printf("Set COM device config\n");
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
			_log.Developer(L"Thread started [label]" + _label + L" [deviceName]" + _config.deviceName);

			_log.Developer(L"Open - [deviceName]" + _config.deviceName);
			switch (_config.deviceType) {
				case BTClassic: _port = new BTClassicManager(std::ref(_log), _eventManager, _config.deviceName); break;
				case BLE:	    _port = new BLEManager(std::ref(_log), _eventManager, _config.deviceName, _config.uuidService, _config.uuidTx, _config.uuidRx); break;
				case USB_COM:	_port = new COMManager(std::ref(_log), _eventManager, _config.deviceName, _config.comBps, 
					(char)_config.comDataBits, (char)_config.comParity, (char)_config.comStopBits, (eFlowControl)_config.comFlowControl); break;
				case NI_DAQ:	_port = new CDAQCom(std::ref(_log), _eventManager, _config.deviceName, _config.ports, _config.portsNum); break;
			}
			Open();

			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]{ return _stopThreadRequest; });

			_log.Developer(L"Close - [deviceName]" + _config.deviceName);
			Close();

			_stopThreadResponse = true;
			_cv.notify_one();

			_log.Developer(L"Thread stopped [label]" + _label + L" [deviceName]" + _config.deviceName);
		});
		_thread.detach();
	}

	bool Open()
	{
		_isConnecting = true;
		if (_port->Open()) {
			_log.Developer(L"OpenConnection - connect [deviceName]" + _config.deviceName);
			_port->Connect();
			_isConnected = true;

			if (_config.recvBuffSize != 0) {
				_log.Developer(L"Set buffer size [deviceName]" + _config.deviceName + L" [size]" + std::to_wstring(_config.recvBuffSize));
				_port->SetBufferSize(_config.recvBuffSize);
			}

			_isConnecting = false;
			return true;
		}
		else {
			_log.Developer(L"OpenConnection - failed to open [deviceName]" + _config.deviceName);
			_isConnected = false;

			_isConnecting = false;
			return false;
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
			_cv.notify_one();

			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]{ return _stopThreadResponse; });
		});
		_thread.join();
	}

	void Send(const char* data, int length)
	{
		if (!_isConnected) return;
		_port->Send(data, length);
	}

	void SetRecvBufferSize(int size)
	{
		_config.recvBuffSize = size;

		if (!_isConnected) return;

		_log.Developer(L"Set buffer size [deviceName]" + _config.deviceName + L" [size]" + std::to_wstring(_config.recvBuffSize));
		_port->SetBufferSize(_config.recvBuffSize);
	}

	int GetRecvBufferSize()
	{
		return _config.recvBuffSize;
	}

	void GetRecvData(char* data)
	{
		if (!_isConnected) return;
		_port->GetRecvData(data);
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
	PortManager*            _port;
	std::mutex              _mtx;
	std::condition_variable _cv;

	bool _isConnecting       = false;
	bool _isConnected        = false;
	bool _threadStarted      = false;
	bool _stopThreadRequest  = false;
	bool _stopThreadResponse = false;
};