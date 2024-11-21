#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(LogManager& log, int handle) : _log(log), _scanner(log), _dataManager(log), _threadManager(log), _threadManager2(_log)
{
	_handle = handle;
}

ConnectionManager::~ConnectionManager()
{
	Disconnect();
}

void ConnectionManager::SetDeviceConfig(ConnectionConfig config)
{
	_config = config;
	_log.Developer(L"[%s, %d] ConnectionManager::SetDeviceConfig - 설정 완료", _config.deviceName, _handle);
}

void ConnectionManager::Connect()
{
	std::thread _thread([this]() {
		_threadStarted = true;
		_log.Developer(L"[%s, %d] ConnectionManager::Connect - 연결 스레드 시작", _config.deviceName, _handle);
		_log.Developer(L"[%s, %d] ConnectionManager::Connect - 디바이스 타입 : %d", _config.deviceName, _handle, (int)_config.deviceType);
		_dataManager.SetDeviceName(_config.deviceName);

		switch (_config.deviceType) {
			case None:      _port = NULL; break;
			case BTClassic: _port = new BTClassicManager(std::ref(_log), std::ref(_config), std::ref(_dataManager), std::ref(_threadManager), [this]() { this->Disconnect(); }); break;
			case BLE:	    _port = new BLEManager      (std::ref(_log), std::ref(_config), std::ref(_dataManager), std::ref(_threadManager), [this]() { this->Disconnect(); }); break;
			case USB_COM:	_port = new COMManager      (std::ref(_log), std::ref(_config), std::ref(_dataManager), std::ref(_threadManager), [this]() { this->Disconnect(); }); break;
#ifdef USE_DAQ
			case NI_DAQ:	_port = new CDAQCom         (std::ref(_log), std::ref(_config), std::ref(_dataManager), std::ref(_threadManager), [this]() { this->Disconnect(); }); break;
#else
			case NI_DAQ:    _port = NULL; break;
#endif
			case TCP:
				if (_config.socketType == Server) _port = new TCPServer(std::ref(_log), std::ref(_config), std::ref(_dataManager), std::ref(_threadManager), [this]() { this->Disconnect(); });
				break;
		}

		if (_port != NULL) {
			_port->handle = _handle;

			_log.Developer(L"[%s, %d] ConnectionManager::Connect - port open 요청", _config.deviceName, _handle);
			Open();

			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]{ return _stopThreadRequest; });
			_log.Developer(L"[%s, %d] ConnectionManager::Connect - thread stop request 수신", _config.deviceName, _handle);
			Close();
		}
		
		_stopThreadResponse = true;
		_cv.notify_all();
		_log.Developer(L"[%s, %d] ConnectionManager::Connect - 연결 스레드 종료", _config.deviceName, _handle);
	});
	_thread.detach();
}

void ConnectionManager::Open()
{
	_isConnecting = true;
	_connectionStartPoint = std::chrono::steady_clock::now();

	_log.Developer(L"[%s, %d] ConnectionManager::Open - port connect 요청", _config.deviceName, _handle);

	if (!_port->Open()) {
		_log.Developer(L"[%s, %d] ConnectionManager::Open - port open 실패", _config.deviceName, _handle);
		_onConnectionFailed();

		_isConnected = false;
		_isConnecting = false;

		std::thread _thread([this]() { Disconnect(); });
		_thread.detach();

		_isOpenFinshed = true;
		_cv.notify_all();

		return;
	}

	while(!_isConnected && !_stopConnectionLoop) {
		auto currentTimePoint = std::chrono::steady_clock::now();
		int elapsedTime = (int)std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint - _connectionStartPoint).count();

		if (_config.useTimeout == 1) {
			if (elapsedTime > _config.connectionTimeout) {
				_log.Developer(L"[%s, %d] ConnectionManager::Open - Connection timeout", _config.deviceName, _handle);
				_onConnectionFailed();

				_isConnected = false;
				_isConnecting = false;

				std::thread _thread([this]() { Disconnect(); });
				_thread.detach();

				break;
			}
		}
			
		if (_port->Connect()) {
			_onConnected();

			if (_config.deviceType != NI_DAQ) {
				_log.Developer(L"[%s, %d] ConnectionManager::Open - 데이터 수신 스레드 시작", _config.deviceName, _handle);
				_threadManager2.RegisterCallback(std::bind(&ConnectionManager::RecvThreadFunc, this, std::placeholders::_1));
				_threadManager2.StartThread();
			}
				
			_isConnected = true;
			_isConnecting = false;

			break;
		}

		Sleep(10);
	}

	_isOpenFinshed = true;
	_cv.notify_all();
} 

void ConnectionManager::Close()
{
	_log.Developer(L"[%s, %d] ConnectionManager::Close - port close 요청", _config.deviceName, _handle);
	_port->Close();
	delete _port;

	_isConnected = false;
}

void ConnectionManager::Disconnect()
{
	if (!_threadStarted) return;
	if (_stopThreadRequest) return;
	if (_stopThreadResponse) return;

	_log.Developer(L"[%s, %d] ConnectionManager::Disconnect - 연결 종료 시작", _config.deviceName, _handle);

	std::thread _thread([this]() {
		_stopConnectionLoop = true;

		if (_isConnecting) {
			_port->ForceClose();
		}

		std::unique_lock<std::mutex> lock(_mtx);
		_cv.wait(lock, [this]{ return _isOpenFinshed; });

		_threadManager2.StopThread();

		_stopThreadRequest = true;
		_cv.notify_all();
		_log.Developer(L"[%s, %d] ConnectionManager::Disconnect - thread stop request 요청", _config.deviceName, _handle);

		_cv.wait(lock, [this]{ return _stopThreadResponse; });
		_log.Developer(L"[%s, %d] ConnectionManager::Disconnect - thread stop request 수신 확인", _config.deviceName, _handle);

		_onDisconnected();
	});
	_thread.join();
}

void ConnectionManager::RecvThreadFunc(bool doWork)
{
	if (!_isConnected) return;
	int bytesRead = _port->RecvData(_recvBuff, _recvByteForRead);
	if (bytesRead > 0) {
		_dataManager.SetData(_recvBuff, bytesRead);
	}
}

void ConnectionManager::SendData(const char* data, int length)
{
	if (!_isConnected) return;
	_port->SendData(data, length);
}

void ConnectionManager::SendData(const double* data, int length)
{
	if (!_isConnected) return;
	_port->SendData(data, length);
}

void ConnectionManager::SendData(const int* data, int length)
{
	if (!_isConnected) return;
	_port->SendData(data, length);
}

void ConnectionManager::RegisterDataEvent(EventCallbackWithChar onDataReceived)
{
	_dataManager.SetCallback(onDataReceived);
}

void ConnectionManager::RegisterEvents(EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected)
{
	_onConnected = onConnected;
	_onConnectionFailed = onConnectionFailed;
	_onDisconnected = onDisconnected;
}

void ConnectionManager::SetPacketConfig(PacketConfig packetConfig)
{
	_dataManager.SetConfig(packetConfig);
}

void ConnectionManager::SetResourceConfig(ResourceConfig resourceConfig)
{
	_threadManager.SetResourceLimit(resourceConfig);
}

void ConnectionManager::CheckResources(double* cpuUsage, int* memoryRemained)
{
	_threadManager.CheckResource(cpuUsage, memoryRemained);
}

void ConnectionManager::GetReceivedBufferLength(int* length)
{
	_dataManager.GetReceivedBufferLength(length);
}