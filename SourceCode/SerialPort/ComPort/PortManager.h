#pragma once
#include "EventManager.h"
#include <windows.h>
#include <thread>
#include <assert.h>

#define DEFAULT_BUFF_SIZE 10000
#define DEVICE_SCAN_RANGE 30

enum DeviceType
{
	NI_DAQ,
	USB_COM,
	BTClassic,
	BLE,
};

class PortManager
{
public:
	PortManager(LogManager& log, EventManager eventManager) : _log(log), _eventManager(eventManager) { }

	virtual bool Open()       { return false; }
	virtual void Connect()    { }
	virtual void Close()      { }
	virtual void ForceClose() { }
				  
	virtual void Send(const char* msg, int len) { }
	virtual int  Recv(char* msg, int len) = 0;
	virtual void Purge() { }
				  
	virtual void GetRecvData(char* data) { }
	virtual void GetRecvDone() { }
	virtual int  GetBufferSize() = 0;
	virtual void SetBufferSize(int size) { }

	LogManager&   _log;
	EventManager _eventManager;
};
