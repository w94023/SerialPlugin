#pragma once
#include <map>
#include "StructManager.h"
#include "DataManager.h"
#include "PortManager.h"
#include "PortScanner.h"
#include "ConnectionManager.h"

#ifdef __has_include
  #if __has_include("NIDAQmx.h")
    #include "NIDAQmx.h"
    #define HEADER_INCLUDED 1
  #else
    #define HEADER_INCLUDED 0
  #endif
#else
  // __has_include�� �������� �ʴ� ���
  #define HEADER_INCLUDED 0
#endif

#define USE_DAQ false

LogManager _log = LogManager();
std::map<int, ConnectionManager*> _manager;

extern "C"
{
	// Create console for debuggin in Unity editor
	__declspec(dllexport) void OpenConsoleAndPrint() 
    {
        // Create console window
        AllocConsole();

		// Redirecting C++ output to console
        FILE* stream;
        errno_t err = freopen_s(&stream, "CONOUT$", "w", stdout);

        // Ready for user input (prevent to close console)
        std::cin.get();

//#if HEADER_INCLUDED
//		std::cout << "DAQ included" << std::endl;
//#else
//		std::cout << "DAQ not included" << std::endl;
//#endif
    }

	// Register log event callback
	__declspec(dllexport) void SetLogCallback(EventCallbackWithChar callback)
	{ 
		_log.SetCallback(callback);
	}

	// Set log level
	__declspec(dllexport) void SetLogLevel(int logLevel)
	{ 
		_log.SetLevel(logLevel);
	}

	// Delete memory in int pointer
	__declspec(dllexport) void DeleteLogMemory(char* log)
	{
		_log.Delete(log);
	}

	// Search for available ports
	//__declspec(dllexport) void ScanDevices(char* scannedDevices, int* charCount)
	__declspec(dllexport) void ScanDevices()
	{
		PortScanner scanner = PortScanner(_log);
		scanner.ScanDevices();

		size_t offset = 0;
		std::map<int, std::wstring> deviceNameList = scanner.GetDeviceNameList();
		std::map<int, DeviceType>   deviceTypeList = scanner.GetDeviceTypeList();

		std::wstring header;
		for (size_t i = 0; i < deviceNameList.size(); i++) {
			switch (deviceTypeList[i]) {
				case USB_COM:   header = L"[USB_COM]";       break;
				case BTClassic: header = L"[BTClassic]"; break;
				case BLE:       header = L"[BLE]";       break;
			}

			_log.Normal(L"%s %s", header.c_str(), deviceNameList[i].c_str());

			//memcpy(scannedDevices + offset, header.c_str(), 2 * header.length());
			//offset += 2 * header.length();
			//memcpy(scannedDevices + offset, deviceNameList[i].c_str(), 2 * deviceNameList[i].length());
			//offset += 2 * deviceNameList[i].length();
			//memcpy(scannedDevices + offset, L"\\", 2);
			//offset += 2;
		}

		//*charCount = offset;
	}

	__declspec(dllexport) int CreateConnection(int handle)
	{
		if (_manager.find(handle) != _manager.end()) {
			return 0;
		}
		else {
			_manager[handle] = new ConnectionManager(_log, handle);
			_log.Developer(L"[%d] ConnectionManager�� ���� �Ǿ����ϴ�.", handle);
			return 1;
		}
	}

	__declspec(dllexport) void DeleteConnection(int handle)
	{ 
		if (_manager.find(handle) != _manager.end()) { 
			delete _manager[handle];
			_manager.erase(handle);
		}
		_log.Developer(L"[%d] ConnectionManager�� ���� �Ǿ����ϴ�.", handle);
	}

	__declspec(dllexport) void Connect(int handle, ConnectionConfig connectionConfig)
	{
		if (_manager.find(handle) != _manager.end()) {
			_manager[handle]->SetDeviceConfig(connectionConfig);
			_manager[handle]->Connect();
		}
	}

	__declspec(dllexport) void Disconnect(int handle)
	{ 
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->Disconnect();
	}

	__declspec(dllexport) void SetPacketConfig(int handle, PacketConfig packetConfig)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->SetPacketConfig(packetConfig);
	}

	__declspec(dllexport) void SetResourceConfig(int handle, ResourceConfig resourceConfig)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->SetResourceConfig(resourceConfig);
	}

	__declspec(dllexport) void CheckResources(int handle, double* cpuUsage, int* memoryRemained)
	{
		if (_manager.find(handle) != _manager.end()) {
			_manager[handle]->CheckResources(cpuUsage, memoryRemained);
		}
	}

	__declspec(dllexport) void RegisterDataEvent(int handle, EventCallbackWithChar onDataReceived)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->RegisterDataEvent(onDataReceived);
	}

	__declspec(dllexport) void RegisterEvents(int handle, EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->RegisterEvents(onConnected, onConnectionFailed, onDisconnected);
	}

	__declspec(dllexport) void GetReceivedBufferLength(int handle, int* length)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->GetReceivedBufferLength(length);
	}

	__declspec(dllexport) void SendPacketData(int handle, const char* data, int length)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->SendData(data, length);
	}

	__declspec(dllexport) void SendFloatData(int handle, const double* data, int length)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->SendData(data, length);
	}

	__declspec(dllexport) void SendIntData(int handle, const int* data, int length)
	{
		if (_manager.find(handle) != _manager.end())
			_manager[handle]->SendData(data, length);
	}
}