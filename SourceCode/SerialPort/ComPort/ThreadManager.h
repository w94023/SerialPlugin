#pragma once
#include "PortManager.h"
#include <thread>
#include <chrono>

class CThreadManager
{
public:
	~CThreadManager()
	{
		StopThread();
	}

	void RegisterCallback(ThreadCallback callback)
	{
		_callback = callback;
	}

	void StartThread()
	{
		_stopThread = false;
		std::thread _rThread(&CThreadManager::ThreadFunc, this);
		_rThread.detach();
	}

	void StopThread()
	{
		_stopThread = true;
	}

private:
	void ThreadFunc()
	{
		while (true) {
			if (_stopThread) break;
			if (_callback) _callback();
			// 루프 속도에 제한을 두는 것은 겁쟁이들이나 하는 짓이다
			//std::this_thread::sleep_for(std::chrono::microseconds(1));
			//Sleep(1);
		}
	}

	ThreadCallback _callback = NULL;

	bool _stopThread = false;
};

//#pragma once 
//#include <thread>
//#include <chrono>
////#include "Udp.h"
//#include "LockBlock.h"
////#include "TcpServer.h"
////#include "TcpClient.h"
//#include "SerialCOM.h"
//
//
//class ThreadManager
//{
//public:
//	ThreadManager (CPortManager *port);
//	virtual ~ThreadManager ();
//
//private:
//	void RecvMessage ();
//	int ThreadFunc ();
//
//	CLock _lock;
//	CPortManager *_port;
//	char *_recvBuff;
//
//	bool _recvCheck;
//	bool _stopRequest;
//	std::thread _thread;
//
//	friend UINT _ThreadFunc (LPVOID pParam);
//};
	
