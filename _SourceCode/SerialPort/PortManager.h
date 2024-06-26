#pragma once
#include "EventManager.h"
#include <windows.h>
#include <thread>
#include <assert.h>

#define DEFAULT_BUFF_SIZE 10000
#define DEVICE_SCAN_RANGE 30

typedef enum
{
	NI_DAQ,
	USB_COM,
	BTClassic,
	BLE,
} DeviceType;

typedef enum
{
	PPS,
	CPU,
	Memory
} ResourceLimitType;

class PortManager
{
public:
	PortManager(LogManager& log, EventManager eventManager, std::function<void()> closeFunc) : 
		_log(log), _eventManager(eventManager), _closeFunc(closeFunc) { }

	virtual bool Open()								                                 { return false; }
	virtual void Connect()							                                 { }
	virtual void Close()							                                 { }
	virtual void ForceClose()						                                 { }
				  			  						                                 
	virtual int  GetRecvData(char* data, int byteSize)                               { return 0; }
	virtual void GetRecvDone()						                                 { }
	virtual int  GetRecvBufferSize()				                                 { return 0; }
	virtual void SetRecvBufferSize(int size)		                                 { }
	virtual void SetConnecitonTimeout(int timeout)                                   { _connectionTimeout = timeout; }

	virtual void SetResourceLimit(ResourceLimitType resourceLimitType, double limit) { }
	virtual void CheckResources(double* cpuUsage, int* memoryRemained)               { }
	virtual void Test(double cpuUsage, int memoryRemained)                           { }

	virtual int  SendData(const char* msg, int len)                                  { return 0; }
	virtual int  RecvData()                                                          { return 0; }

	LogManager&			  _log;
	EventManager		  _eventManager;
	std::function<void()> _closeFunc;

	int				      _connectionTimeout = 5000;
};
