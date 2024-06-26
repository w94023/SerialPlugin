#pragma once
#include <map>

#include "Appini.h"
#include "PortManager.h"
#include "PortScanner.h"
#include "ConnectionManager.h"

#define CREATEDLL_EXPORTS

#ifdef CREATEDLL_EXPORTS
	#define SERIALPORT_DECLSPEC __declspec(dllexport)
#else
	#define SERIALPORT_DECLSPEC __declspec(dllimport)
#endif

LogManager  _log;
map<wstring, ConnectionManager*> _manager;

extern "C"
{	
	SERIALPORT_DECLSPEC void ScanDevices(const wchar_t* label)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->ScanDevices();
	}

	SERIALPORT_DECLSPEC void RegisterOnScanEnded(const wchar_t* label, EventCallback callback)
	{ 
		if (_manager.find(label) != _manager.end())
			_manager[label]->RegisterOnScanEnded(callback);
	}

	SERIALPORT_DECLSPEC void GetScannedDevice(const wchar_t* label, char* scannedDevice)
	{
		if (_manager.find(label) == _manager.end()) return;

		size_t offset = 0;
		std::map<int, std::wstring> deviceNameList = _manager[label]->GetDeviceNameList();
		std::map<int, DeviceType>   deviceTypeList = _manager[label]->GetDeviceTypeList();

		std::wstring header;

		for (size_t i = 0; i < deviceNameList.size(); i++) {
			switch (deviceTypeList[i]) {
				case USB_COM:   header = L"[COM]";       break;
				case BTClassic: header = L"[BTClassic]"; break;
				case BLE:       header = L"[BLE]";       break;
			}

			memcpy(scannedDevice + offset, header.c_str(), 2 * header.length());
			offset += 2 * header.length();
			memcpy(scannedDevice + offset, deviceNameList[i].c_str(), 2 * deviceNameList[i].length());
			offset += 2 * deviceNameList[i].length();
			memcpy(scannedDevice + offset, L"\\", 2);
			offset += 2;
		}
	}

	SERIALPORT_DECLSPEC int  GetCountRx(const char* comPort);
	SERIALPORT_DECLSPEC void CheckMemory();
	SERIALPORT_DECLSPEC int  GetLog(char* log);
	SERIALPORT_DECLSPEC void RegisterLogging     (EventCallback callback) { _log.Logging = callback; }

	SERIALPORT_DECLSPEC void CreateConnectionManager(const wchar_t* label, char* result)
	{
		if (_manager.find(label) != _manager.end()) {
			result[0] = 1;
		}
		else {
			result[0] = 2;
			_manager[label] = new ConnectionManager(_log, label);
			_log.Developer(L"ConnectionManager created [label]" + std::wstring(label));
		}
	}

	SERIALPORT_DECLSPEC void DeleteConnectionManager(const wchar_t* label)
	{ 
		if (_manager.find(label) != _manager.end()) { 
			delete _manager[label];
			_manager.erase(label);
		}
		_log.Developer(L"ConnectionManager deleted [label]" + std::wstring(label));
	}

	SERIALPORT_DECLSPEC void SetBTClassicConfig(const wchar_t* label, const wchar_t* deviceName)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->SetConfig(deviceName);
	}

	SERIALPORT_DECLSPEC void SetBLEConfig(const wchar_t* label, const wchar_t* deviceName, const wchar_t* uuidService, const wchar_t* uuidTx, const wchar_t* uuidRx)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->SetConfig(deviceName, uuidService, uuidTx, uuidRx);
	}

	SERIALPORT_DECLSPEC void SetCOMConfig(const wchar_t* label, const wchar_t* comPort, int comBps, int comDataBits, int comParity, int comStopBits, int comFlowControl)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->SetConfig(comPort, comBps, comDataBits, comParity, comStopBits, comFlowControl);
	}

	SERIALPORT_DECLSPEC void SetDAQConfig(const wchar_t* label, const wchar_t* deviceName, char* ports, int portsNum)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->SetConfig(deviceName, ports, portsNum);
	}

	SERIALPORT_DECLSPEC void RegisterEvents(const wchar_t* label, EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected, EventCallback onDataReceived)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->RegisterEvents(onConnected, onConnectionFailed, onDisconnected, onDataReceived);
	}

	SERIALPORT_DECLSPEC void Connect(const wchar_t* label)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->Connect();
	}

	SERIALPORT_DECLSPEC void Disconnect(const wchar_t* label)
	{ 
		if (_manager.find(label) != _manager.end())
			_manager[label]->Disconnect();
	}

	SERIALPORT_DECLSPEC void SetRecvBufferSize(const wchar_t* label, int size)
	{ 
		if (_manager.find(label) != _manager.end())
			_manager[label]->SetRecvBufferSize(size);
	}

	SERIALPORT_DECLSPEC int GetRecvBufferSize(const wchar_t* label)
	{
		if (_manager.find(label) != _manager.end())
			return _manager[label]->GetRecvBufferSize();
		else
			return 0;
	}

	SERIALPORT_DECLSPEC void SendData(const wchar_t* label, const char* data, int length)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->Send(data, length);
	}

	SERIALPORT_DECLSPEC void GetRecvData(const wchar_t* label, char* data)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->GetRecvData(data);
	}

	SERIALPORT_DECLSPEC void GetRecvDone(const wchar_t* label)
	{
		if (_manager.find(label) != _manager.end())
			_manager[label]->GetRecvDone();
	}
}