///////////////////////////////////////////////.h

//#pragma once
//#include "./ComPort/ComThread.h"
//#include <vector>
//
//#define CREATEDLL_EXPORTS
//
//#ifdef CREATEDLL_EXPORTS
//	#define SERIALPORT_DECLSPEC __declspec(dllexport)
//#else
//	#define SERIALPORT_DECLSPEC __declspec(dllimport)
//#endif
////void VoidFunc() {}
//
//class CSerialPort
//{
//public:
//	CSerialPort();
//
//	void GetValidPort(uint8_t * output, int num);
//	bool SetComPort(const char* comPort, int comBps);
//	void SetRecvBufferSize(int size);
//	int  GetRecvBufferSize();
//	int  GetCountRx();
//
//	bool OpenConnection();
//	void CloseConnection();
//	void SendData(const char* data);
//	void GetRecvData(char* data);
//
//	void RegisterConnectionHandlers			(EventCallback callback);
//	void RegisterConnectionFailedHandlers	(EventCallback callback);
//	void RegisterDisconnectionHandlers		(EventCallback callback);
//	void RegisterDataReceivedHandlers		(EventCallback callback);
//	void ClearHandlers();
//
//private:
//	vector<EventCallback> _onConnected;
//	vector<EventCallback> _onConnectionFailed;
//	vector<EventCallback> _onDisconnected;
//	vector<EventCallback> _onDataReceived;
//
//	CComPort* g_com;
//	CComThread* _comThread;
//
//	bool recvBuffSizeResetRequest;
//	bool recvEventRegisterRequest;
//	int recvBufferSize;
//};
//
//extern "C"
//{
//	SERIALPORT_DECLSPEC CSerialPort* CreateInstance()			{ return new CSerialPort(); }			
//	SERIALPORT_DECLSPEC void GetValidPort						(CSerialPort* instance, uint8_t * output, int num);
//	SERIALPORT_DECLSPEC bool SetComPort							(CSerialPort* instance, const char* comPort, int comBps);
//	SERIALPORT_DECLSPEC void SetRecvBufferSize					(CSerialPort* instance, int size);
//	SERIALPORT_DECLSPEC int  GetRecvBufferSize					(CSerialPort* instance);
//	SERIALPORT_DECLSPEC int  GetCountRx							(CSerialPort* instance);
//
//	SERIALPORT_DECLSPEC bool OpenConnection						(CSerialPort* instance);
//	SERIALPORT_DECLSPEC void CloseConnection					(CSerialPort* instance);
//	SERIALPORT_DECLSPEC void SendData							(CSerialPort* instance, const char* data);
//	SERIALPORT_DECLSPEC void GetRecvData						(CSerialPort* instance, char* data);
//
//	SERIALPORT_DECLSPEC void RegisterConnectionHandlers			(CSerialPort* instance, EventCallback callback);
//	SERIALPORT_DECLSPEC void RegisterConnectionFailedHandlers	(CSerialPort* instance, EventCallback callback);
//	SERIALPORT_DECLSPEC void RegisterDisconnectionHandlers		(CSerialPort* instance, EventCallback callback);
//	SERIALPORT_DECLSPEC void RegisterDataReceivedHandlers		(CSerialPort* instance, EventCallback callback);
//	SERIALPORT_DECLSPEC void ClearHandlers						(CSerialPort* instance);
//}




////////////////////////////cpp

//#include "SerialPort.h"
//#include "Appini.h"
//#include "stdio.h"
//#include <assert.h>
//#include <vector>
//
//using namespace std;
//
//void GetValidPort						(CSerialPort* instance, uint8_t * output, int num)			{ return instance->GetValidPort(output, num); }
//bool SetComPort							(CSerialPort* instance, const char* comPort, int comBps)	{ return instance->SetComPort(comPort, comBps); }
//void SetRecvBufferSize					(CSerialPort* instance, int size)							{ return instance->SetRecvBufferSize(size); }
//int  GetRecvBufferSize					(CSerialPort* instance)										{ return instance->GetRecvBufferSize(); }
//int  GetCountRx							(CSerialPort* instance)										{ return instance->GetCountRx(); }
//
//bool OpenConnection						(CSerialPort* instance)										{ return instance->OpenConnection(); }
//void CloseConnection					(CSerialPort* instance)										{ return instance->CloseConnection(); }
//void SendData							(CSerialPort* instance, const char* data)					{ return instance->SendData(data); }
//void GetRecvData						(CSerialPort* instance, char* data)							{ return instance->GetRecvData(data); }
//
//void RegisterConnectionHandlers			(CSerialPort* instance, EventCallback callback)				{ return instance->RegisterConnectionHandlers(callback); }
//void RegisterConnectionFailedHandlers	(CSerialPort* instance, EventCallback callback)				{ return instance->RegisterConnectionFailedHandlers(callback); }
//void RegisterDisconnectionHandlers		(CSerialPort* instance, EventCallback callback)				{ return instance->RegisterDisconnectionHandlers(callback); }
//void RegisterDataReceivedHandlers		(CSerialPort* instance, EventCallback callback)				{ return instance->RegisterDataReceivedHandlers(callback); }
//void ClearHandlers						(CSerialPort* instance)										{ return instance->ClearHandlers(); }
//
//CAppIni g_ini("SerialPort.ini");
//
//CSerialPort::CSerialPort() :
//	g_com(NULL), _comThread(NULL),
//	recvBuffSizeResetRequest(false), recvEventRegisterRequest(false), recvBufferSize(DEFAULT_BUFF_SIZE)
//{
//}
//
//void CSerialPort::GetValidPort(uint8_t* output, int num)
//{
//	// COM0 ~ COM(num)까지 검색
//	vector<bool> validPort(num, false);
//	for (int i = 0; i < num; i++) {
//		char name[MAX_PATH];
//		sprintf_s(name, "\\\\.\\COM%d", i);
//
//		// COM 포트를 열어본다. 열리면 COM 포트가 있는 것이다.
//		HANDLE h = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
//		if (h != INVALID_HANDLE_VALUE) {
//			CloseHandle(h);
//
//			validPort[i] = true;
//		}
//		output[i] = validPort[i];
//	}
//}
//
//bool CSerialPort::SetComPort(const char* comPort, int comBps)
//{
//	strcpy_s(g_ini.comPort, comPort);
//	g_ini.comBps = comBps;
//	return true;
//}
//
//void CSerialPort::SetRecvBufferSize(int size)
//{
//	recvBufferSize = size;
//	// reading thread가 생성되어 있지 않다면, request를 등록하여 thread가 생성될 때 바로 수정되도록 함
//	if (!_comThread)	recvBuffSizeResetRequest = true;
//	else				_comThread->SetBufferSize(recvBufferSize);
//}
//
//int CSerialPort::GetRecvBufferSize()
//{
//	return recvBufferSize;
//}
//
//int CSerialPort::GetCountRx()
//{
//	if (!g_com) return -1;
//	else		return g_com->CountRx();
//}
//
//void CSerialPort::RegisterConnectionHandlers(EventCallback callback)
//{
//	_onConnected.push_back(callback);
//}
//
//void CSerialPort::RegisterConnectionFailedHandlers(EventCallback callback)
//{ 
//	_onConnectionFailed.push_back(callback);
//}
//
//void CSerialPort::RegisterDisconnectionHandlers(EventCallback callback)
//{
//	_onDisconnected.push_back(callback);
//}
//
//void CSerialPort::RegisterDataReceivedHandlers(EventCallback callback)
//{
//	_onDataReceived.push_back(callback);
//}
//
//void CSerialPort::ClearHandlers()
//{
//	_onConnected.clear();
//	_onConnectionFailed.clear();
//	_onDisconnected.clear();
//	_onDataReceived.clear();
//}
//
//bool CSerialPort::OpenConnection()
//{
//	g_ini.options.CONNECTION_TYPE = CONNECTION_COM;
//	g_ini.comDatabits = 8;
//	g_ini.comParity = 0;
//	g_ini.comStopbits = 0;
//	g_ini.comFlowcontrol = 0;
//
//	int timeout = 250; // ms
//
//	switch (g_ini.options.CONNECTION_TYPE) {
//	//case CONNECTION_UDP: g_comm = new CUdp(g_ini.localIpAddr, g_ini.remoteIpAddr, (WORD)g_ini.portNum, timeout); break;
//	//case CONNECTION_TCPS: g_comm = new CTcpServer(g_ini.localIpAddr, (WORD)g_ini.portNum, timeout); break;
//	//case CONNECTION_TCPC: g_comm = new CTcpClient(g_ini.remoteIpAddr, (WORD)g_ini.portNum, timeout); break;
//	case CONNECTION_COM: g_com = new CSerialCOM(g_ini.comPort, g_ini.comBps, (char)g_ini.comDatabits,
//		(char)g_ini.comParity, (char)g_ini.comStopbits, (eFlowControl)g_ini.comFlowcontrol, timeout); break;
//	default: assert(0); break;
//	}
//	//g_com->onConnected			= _onConnected;
//	//g_com->onConnectionFailed	= _onConnectionFailed;
//	//g_com->onDisconnected		= _onDisconnected;
//
//	if (g_com->Open()) {
//		g_com->Connect();
//		for (EventCallback handler : _onConnected) handler();
//		// 연결할 포트를 오픈 하였다면 계속 연결을 시도하거나 
//		// 연결을 대기하거나 메시지 수신을 쓰레드로 수행한다.
//		_comThread = new CComThread(g_com);
//		if (recvBuffSizeResetRequest) {
//			_comThread->SetBufferSize(recvBufferSize);
//			recvBuffSizeResetRequest = false;
//		}
//		_comThread->onDataReceived = _onDataReceived;
//		return true;
//	}
//	else {
//		delete g_com;
//		g_com = NULL;
//		for (EventCallback handler : _onConnectionFailed) handler();
//		return false;
//	}
//}
//
//void CSerialPort::CloseConnection()
//{
//	if (_comThread) {
//		delete _comThread;
//		_comThread = NULL;
//	}
//	if (g_com) {
//		g_com->Close();
//		delete g_com;
//		g_com = NULL;
//	}
//	for (EventCallback handler : _onDisconnected) handler();
//}
//
//void CSerialPort::SendData(const char* data)
//{
//	if (!g_com) return;
//	else		g_com->Send(data, 10);
//}
//
//void CSerialPort::GetRecvData(char* data)
//{
//	if (g_com) {
//		for (int i = 0; i < recvBufferSize; i++) {
//			data[i] = (_comThread->recvBuff[i]);
//		}
//	}
//}