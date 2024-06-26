#include "BTClassicManager.h"

BTClassicManager::BTClassicManager(LogManager& log, EventManager eventManager, std::function<void()> closeFunc, std::wstring deviceName) : 
	PortManager(log, eventManager, closeFunc), _scanner(log), _threadManager(log)
{
	_threadManager.RegisterCallback(std::bind(&BTClassicManager::RecvThreadFunc, this, std::placeholders::_1));

	// 비동기 입출력 이벤트 생성
	_wOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_rOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// BT 연결을 위한 데이터 저장
	_deviceName = deviceName;
}

BTClassicManager::~BTClassicManager()
{
	Close();
}

bool BTClassicManager::Open()
{
	std::wstring uuidService = L"{00001101-0000-1000-8000-00805f9b34fb}";

	if (_deviceName == L"") {
		_log.Error(L"Invalid device name : " + _deviceName);
		goto CLOSE_RET;
	}

	_isConnecting = true;
	_handle = _scanner.GetBTHandle(_deviceName, uuidService, false);

	if (_handle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"BTClassicManager::Open() - Invalid handle value error");
		_log.Error(L"Failed to connect device : " + _deviceName);
		goto CLOSE_RET;
	}

	_log.Developer(L"BTClassicManager::Open() - success");
	return true;

CLOSE_RET:
	_eventManager.Invoke("onConnectionFailed");
	_closeFunc();
	return false;
}

void BTClassicManager::Connect()
{
	_log.Developer(L"BTClassicManager::Connect() - start thread");

	_dataReceivedTimePoint = std::chrono::steady_clock::now();
	_threadManager.StartThread();	

	_eventManager.Invoke("onConnected");
	_isConnecting = false;
	_log.Normal(L"Device connected : " + _deviceName);
}

void BTClassicManager::Close()
{
	_isAlive = false;

	if (_isConnecting) {
		_log.Normal(L"Cancel connection process");
	}
	_threadManager.StopThread();
	if (_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;

		_eventManager.Invoke("onDisconnected");
		_log.Normal(L"Device disconnected : " + _deviceName);
	}
}


void BTClassicManager::ForceClose()
{
	_scanner.ForceStop();
}

int BTClassicManager::GetRecvData(char* data, int recvByteSize)
{
	if (!_isAlive) return 0;

	int bytesForRead = (recvByteSize > _recvLen) ? _recvLen : recvByteSize;
	if (bytesForRead < recvByteSize) {
		_recvBufferSize += recvByteSize * 10;
		if (_recvBufferSize > DEFAULT_BUFF_SIZE) {
			_recvBufferSize = DEFAULT_BUFF_SIZE;
		}
		_log.Developer(L"Buffer size adjusted : " + std::to_wstring(_recvBufferSize));
	}

	memcpy(data, _recvBuff, bytesForRead);
	for (int i = 0; i < _recvLen; i++) {
		if (i < _recvLen - bytesForRead) {
			_recvBuff[i] = _recvBuff[i + bytesForRead];
		}
		else {
			_recvBuff[i] = '\0';
		}
	}
	_recvLen -= bytesForRead;

	// Data read request 및 Flush를 위한 reading byte 정보 저장
	_recvByteForRead = recvByteSize;

	return bytesForRead;
}

void BTClassicManager::GetRecvDone()
{
	if (!_isAlive) return;
	std::thread _thread([this]() {
		std::lock_guard<std::mutex> lock(_mtx);
		_recvReady = true;
		_cv.notify_one();
	});
	_thread.detach();
}

void BTClassicManager::SetRecvBufferSize(int size)
{
	if (size > DEFAULT_BUFF_SIZE) {
		_recvBufferSize = DEFAULT_BUFF_SIZE;
	}
	else {
		_recvBufferSize = size;
	}
}

int BTClassicManager::GetRecvBufferSize()
{
	return _recvBufferSize;
}

void BTClassicManager::SetResourceLimit(ResourceLimitType resourceLimitType, double limit)
{
	_threadManager.SetResourceLimit(resourceLimitType, limit);
}

void BTClassicManager::CheckResources(double* cpuUsage, int* memoryRemained)
{
	_threadManager.CheckResource(cpuUsage, memoryRemained);
}

void BTClassicManager::Test(double cpuUsage, int memoryRemained)
{
	_threadManager.Test(cpuUsage, memoryRemained);
}

int BTClassicManager::SendData(const char* buff, int length)
{
	if (_handle == INVALID_HANDLE_VALUE) return -1;

	DWORD bytesWritten = 0;
	while (true) {
		BOOL writeResult = WriteFile(_handle, buff, length, &bytesWritten, &_wOverlapped);
		if (!writeResult) {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Developer(L"BTClassicManager::Send - write file error");
				_log.Error(L"Failed to send data");
				return -1;
			}
			// 비동기 작업이 시작되었으며, 완료를 기다려야 함
			else {
				//printf("ERROR_IO_PENDING\n");
				if (GetOverlappedResult(_handle, &_wOverlapped, &bytesWritten, FALSE)) {
					// bytesWritten에는 이제 실제로 쓴 바이트 수가 저장됩니다.
					//printf("True : %d\n", bytesWritten);
					break;
				} 
				else {
					// GetOverlappedResult 에러 처리
					//printf("False : %d\n", bytesWritten);
					return -1;
				}
			}
		}
	}

	return bytesWritten;
}

int BTClassicManager::RecvData()
{
	if (!_isAlive) return -1;
	if (_handle == INVALID_HANDLE_VALUE) return -1;

	DWORD bytesRead = 0;

	if (_dataReady) {
		if (ReadFile(_handle, _recvBuffTemp, _recvByteForRead, &bytesRead, &_rOverlapped)) {
			// 데이터 수신 성공
			goto READ_DATA;
		}
		else {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Error(L"Failed to read data");
				return 0;
			}
			else {
				// 데이터 수신 대기
				goto WAIT_FOR_DATA;
			}
		}
	}
	else {
		// 데이터 수신 확인
		if (GetOverlappedResult(_handle, &_rOverlapped, &bytesRead, FALSE)) {
			// 데이터 수신 성공
			goto READ_DATA;
		}
		else {
			// 데이터 수신 대기
			goto WAIT_FOR_DATA;
		}
	}

WAIT_FOR_DATA:
	_dataReady = false;
	return 0;

READ_DATA:
	if (bytesRead > 0) {
		if (_recvLen + bytesRead < _recvBufferSize) {
			memcpy(_recvBuff + _recvLen, _recvBuffTemp, bytesRead);
			_recvLen += bytesRead;
		}
	}
	_dataReady = true;

	return bytesRead;
}

void BTClassicManager::RecvThreadFunc(bool doWork)
{
	std::unique_lock<std::mutex> lock(_mtx);
	_cv.wait(lock, [this]{ return _recvReady; });

	//if (_connectionTimeout > 0) {
	//	auto currentTimePoint = std::chrono::steady_clock::now();
	//	int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint - _dataReceivedTimePoint).count();

	//	if (elapsedTime >= _connectionTimeout) {
	//		_log.Error(L"Device connection timeout : " + _deviceName);
	//		_closeFunc();
	//	}
	//}

	if (!_isAlive) return;
	if (_handle == INVALID_HANDLE_VALUE) return;

	// Flush
	// 지정된 recvBufferSize를 넘을 경우 flush 실행
	// recvBufferSize의 1/10 데이터만 남기고 나머지는 삭제
	if (_recvLen >= _recvBufferSize) {
		for (int i = 0; i < _recvLen; i++) {
			if (i < _recvBufferSize / 10) {
				_recvBuff[i] = _recvBuff[i + _recvLen - _recvBufferSize / 10];
			}
			else {
				_recvBuff[i] = '\0';
			}
		}
		_log.Developer(L"Flush data [deviceName] : " + _deviceName + L" [current data byte] : " + std::to_wstring(_recvLen) + L" [flushed data byte] : " + std::to_wstring(_recvBufferSize / 10));
		_recvLen = _recvBufferSize / 10;
	}

	RecvData();

	if (doWork) {
		if (_recvLen > 0) {
			_recvReady = false;

			_dataReceivedTimePoint = std::chrono::steady_clock::now();
			_eventManager.Invoke("onDataReceived");
		}
	}
}