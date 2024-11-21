#pragma once
#include <iostream>
#include <cstdarg>
#include <string>

#define DEFAULT_BUFF_SIZE 10000

typedef void (*EventCallback)();
typedef void (*EventCallbackWithChar)(char*, int*);

std::wstring CharToWString(const char* utf8Str);
const wchar_t* CharToWChar(const char* utf8Str);
std::wstring StringToWString(const std::string& str);

struct LogManager
{
public:
	void SetCallback(EventCallbackWithChar callback);
	void SetLevel(int level);
	void Delete(char* log);

	void Error(std::wstring log, ...);
	void Normal(std::wstring log, ...);
	void Developer(std::wstring log, ...);

private:
	EventCallbackWithChar _callback;
	bool _isEventRegistered = false;
	int  _logLevel = 0;

	void ExportLog(std::wstring log, va_list args);
};

typedef enum
{
	None,
	USB_COM,
	BTClassic,
	BLE,
	NI_DAQ,
	TCP,
} DeviceType;

struct DeviceInfo
{
    std::wstring deviceName;
	std::wstring deviceID;
	std::wstring deviceClass;
	DeviceType   deviceType;
};

enum class BaudRate
{
    bps1200   = 1200,
    bps2400   = 2400,
    bps4800   = 4800,
    bps9600   = 9600,
    bps19200  = 19200,
    bps38400  = 38400,
    bps57600  = 57600,
    bps115200 = 115200
};

enum class DataBit
{
    bit5 = 5,
    bit6 = 6,
    bit7 = 7,
    bit8 = 8
};

enum class Parity
{
    None = 0,
    Odd	 = 1,
    Even = 2
};

enum class StopBit
{
    bit1   = 0,
    bit1p5 = 1,
    bit2   = 2
};

enum class FlowControl
{
    None    = 0,
    XonXoff = 1,
    RTSCTS  = 2
};

typedef enum
{
	Server = 0,
	Clinet = 1
} SocketType;

struct PacketConfig
{
	int   recvByteSize;
	int   recvBufferSize;
	int   usePacketLength;
	int   packetLength;
	int   useStopByte;
	char* stopByte;
	int   stopByteLength;
	int   usePPSLimit;
	int   PPSLimit;
	float flushRatio;

	PacketConfig() : 
		recvByteSize(1), recvBufferSize(DEFAULT_BUFF_SIZE), usePacketLength(0), 
		packetLength(1), useStopByte(0), stopByte(nullptr), stopByteLength(0),
		usePPSLimit(0), PPSLimit(300), flushRatio(0.9f)
	{}

	void Initialize();
};

struct ResourceConfig
{
	int    useCPULimit;
	double CPULimit;
	int    useMemoryLimit;
	int    memoryLimit;
};

struct ConnectionConfig
{
	DeviceType deviceType;
	wchar_t*   deviceName;

	BaudRate    baudRate;
	DataBit     dataBit;
	Parity      parity;
	StopBit     stopBit;
	FlowControl flowControl;

	int*        AIPorts;
	int         AIPortsCount;
	int*        AOPorts;
	int         AOPortsCount;
	int*        DPorts;
	int         DPortsCount;
	int*        lines;
	int         linesCount;

	SocketType socketType;
	int        localIP;
	int        port;

	int        useTimeout;
	int        connectionTimeout;

	std::wstring GetDeviceName();
};

//struct ConnectionConfig
//{
//	DeviceType deviceType = USB_COM;
//
//	std::wstring deviceName;
//	std::wstring uuidService;
//	std::wstring uuidTx;
//	std::wstring uuidRx;
//	
//	int comBps;
//	int comDataBits;
//	int comParity;
//	int comStopBits;
//	int comFlowControl;
//
//	std::vector<std::string> ports;
//	int portsNum;
//
//	int    recvBuffSize = DEFAULT_BUFF_SIZE;
//	int    PPSLimit     = -1; // Hz
//	double CPULimit     = -1;  // 현재 사용량 %
//	int    MemoryLimit  = -1; // 남은 MB
//
//	int   connectionTimeout = 5000;
//
//	bool isConnected = false;
//};