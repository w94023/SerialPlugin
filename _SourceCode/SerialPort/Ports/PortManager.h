#pragma once
#include "StructManager.h"
#include "DataManager.h"
#include "ThreadManager.h"
#include <windows.h>
#include <assert.h>

#define DEFAULT_BUFF_SIZE 10000
#define DEVICE_SCAN_RANGE 30

typedef enum
{
	PPS,
	CPU,
	Memory
} ResourceLimitType;

class PortManager
{
public:
	PortManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) : 
		_log(log), _config(config), _dataManager(dataManager), _threadManager(threadManager), _closeFunc(closeFunc) { }

	virtual bool Open()								                                 { return false; }
	virtual bool Connect()							                                 { return false; }
	virtual void Close()							                                 { }
	virtual void ForceClose()						                                 { }
				  			  						                                 
	virtual int  GetRecvData(char* data, int byteSize)                               { return 0; }
	virtual void GetRecvDone()						                                 { }
	virtual int  GetRecvBufferSize()				                                 { return 0; }
	virtual void SetRecvBufferSize(int size)		                                 { }
	virtual void SetConnecitonTimeout(int timeout)                                   { _connectionTimeout = timeout; }

	virtual int  SendData(const char*   msg,    int len)                             { return 0; }
	virtual int  SendData(const double* values, int len)                             { return 0; }
	virtual int  SendData(const int*    values, int len)                             { return 0; }
	virtual int  RecvData(char* data, int len)                                       { return 0; }
	
	LogManager&			  _log;
	ConnectionConfig&     _config;
	DataManager&          _dataManager;
	CThreadManager&       _threadManager;
	std::function<void()> _closeFunc;

	int				      _connectionTimeout = 5000;
	int handle;

private:
};
