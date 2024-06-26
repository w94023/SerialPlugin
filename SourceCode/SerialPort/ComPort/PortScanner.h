#pragma once
#include "PortManager.h"
#include <future>
#include <chrono>
#include <thread>
#include <windows.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <tchar.h>
#include <cfgmgr32.h>
#include <bthsdpdef.h>
#include <Bluetoothleapis.h>
#include <bluetoothapis.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")
#pragma comment(lib, "Bthprops.lib")

enum eFlowControl {
	FC_NONE = 0,
	FC_XONXOFF,
	FC_RTSCTS,
	FC_DTRDSR,
	FC_FULLHARDWARE,
};

struct Serial_UUID
{
	// HM-10 UUID
	std::string service = "{0000FFE0-0000-1000-8000-00805F9B34FB}";
	std::string tx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
	std::string rx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
};

class PortScanner
{
public:
	PortScanner(LogManager& log);

	void                        ForceStop();
							    
	void                        EraseList();
	void                        ScanDevices();
	std::vector<std::wstring>   ScanDevices(GUID guid);

	std::map<int, std::wstring> GetDeviceNameList() { return _deviceNameList; }
	std::map<int, DeviceType>   GetDeviceTypeList() { return _deviceTypeList; }
	
	int                         CheckDeviceName(std::wstring deviceName, bool isBTDevice);
	int                         RequestPairing(std::wstring deviceName);
	HANDLE                      GetCOMHandle(std::wstring portName, int baudRate, char dataBits, char parity, char stopBits, eFlowControl flowControl);
	HANDLE                      GetBTHandle(std::wstring deviceName, std::wstring uuidService, bool isBLEDevice);

	size_t requestCountLimit = 5;
	int    requestTimeout = 10; // seconds
	EventCallback               onScanEnded;

private:
	std::vector<std::wstring>  SplitWstring(std::wstring& input, const std::wstring& delimiter);
	HANDLE					   SetTimeout(HANDLE handle, int timeout);

	LogManager&				    _log;
	bool					    _forceStop = false;
	HANDLE	                    _handle    = INVALID_HANDLE_VALUE;
	GUID                        _guid;
	
	DWORD					    _handleTimeout = 100;
	
	int                         _deviceCount = 0;
	std::map<int, DeviceType>   _deviceTypeList;
	std::map<int, std::wstring> _deviceNameList;
	std::map<int, std::wstring> _deviceIDList;
};