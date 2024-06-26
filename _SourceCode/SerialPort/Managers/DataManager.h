#pragma once
#include <chrono>
#include "StructManager.h"

class DataManager
{
public:
	DataManager(LogManager& log) : _log(log), _config(PacketConfig())
	{
		_invokePoint = std::chrono::steady_clock::now();
		_invokeRequestedPoint = std::chrono::steady_clock::now();
	}

	void SetDeviceName(std::wstring deviceName)
	{
		_deviceName = deviceName;
	}

	void SetConfig(PacketConfig config)
	{
		_config = config;
		_config.Initialize();
	}

	void ResetConfig()
	{
		_config = PacketConfig();
	}

	void SetCallback(EventCallbackWithChar callback)
	{
		_callback = callback;
		_isCallbackRegistered = true;
	}

	void GetReceivedBufferLength(int* length)
	{
		*length = _recvLen;
	}

	void SetData(char* data, int bytesRead)
	{
		if (_recvLen + bytesRead >= _config.recvBufferSize) {
			Flush();
		}

		memcpy(_recvBuff + _recvLen, data, bytesRead);
		_recvLen += bytesRead;

		auto currentTimePoint = std::chrono::steady_clock::now();
		int elapsedTime = (int)std::chrono::duration_cast<std::chrono::microseconds>(currentTimePoint - _invokePoint).count();
		if (_config.usePPSLimit == 1 && _config.PPSLimit > 0 && _config.PPSLimit < 1000) {
			if (elapsedTime < 1000 * 1000 / _config.PPSLimit - _timeOffset - _delayedTime) return;
			else {
				if (!_isPPSLimitTriggered) {
					_timeOffset = elapsedTime - (1000 * 1000 / _config.PPSLimit - _timeOffset - _delayedTime);
					_invokeRequestedPoint = std::chrono::steady_clock::now();
					_isPPSLimitTriggered = true;
				}
			}
		}

		if (_config.usePacketLength == 0 || _config.packetLength <= 0) {
			Invoke(_recvLen);
		}
		else {
			if (_recvLen >= _config.packetLength) {
				Invoke(_config.packetLength);
			}
		}
	}

	void TrimData(int startIndex, int length)
	{
		for (int i = startIndex; i < _recvLen; i++) {
			if (i < _recvLen - length) {
				_recvBuff[i] = _recvBuff[i + length];
			}
			else {
				_recvBuff[i] = '\0';
			}
		}
		_recvLen -= length;
	}

	void Invoke(int length)
	{
		if (!_isCallbackRegistered) return;

		int startIndex = 0;
		int packetLength = length;

		if (_config.useStopByte == 1 && _config.stopByteLength > 0) {
			int stopByteStartIndex = -1;
			for (int i = 0; i < _recvLen; i++) {

				int stopByteCount = 0;
				for (int j = 0; j < _config.stopByteLength; j++) {
					if (i + j >= _recvLen) {
						break;
					}

					if (_recvBuff[i + j] == _config.stopByte[j]) {
						stopByteCount++;
					}
					else {
						break;
					}
				}

				if (stopByteCount == _config.stopByteLength) {
					stopByteStartIndex = i;
					break;
				}
			}

			if (stopByteStartIndex == -1) return;
			if (_recvLen < length) return;

			startIndex = stopByteStartIndex - (length - _config.stopByteLength);
			if (startIndex < 0) {
				startIndex = 0;
				packetLength = stopByteStartIndex + _config.stopByteLength;
			}
		}
		
		int* bytesRead = new int(packetLength);
		char* data = new char[packetLength];

		memcpy(data, _recvBuff + startIndex, sizeof(char) * packetLength);
		TrimData(startIndex, packetLength);

		auto currentTimePoint = std::chrono::steady_clock::now();
		int elapsedTime = (int)std::chrono::duration_cast<std::chrono::microseconds>(currentTimePoint - _invokeRequestedPoint).count();
		_delayedTime = elapsedTime;
		_isPPSLimitTriggered = false;

		_invokePoint = std::chrono::steady_clock::now();
		_timeOffsetSaved = _timeOffset;
		_callback(data, bytesRead);
	}

private:
	void Flush()
	{
		int targetLength = (int)(_config.recvBufferSize * (1.0f - _config.flushRatio));

		for (int i = 0; i < _recvLen; i++) {
			if (i < targetLength) {
				_recvBuff[i] = _recvBuff[i + _recvLen - targetLength];
			}
			else {
				_recvBuff[i] = '\0';
			}
		}

		_log.Developer(L"Flush data [deviceName] : " + _deviceName + 
						L" [current data byte] : "    + std::to_wstring(_recvLen) + 
				        L" [flushed data byte] : "    + std::to_wstring(targetLength));
		_recvLen = targetLength;
	}

	LogManager& _log;
	std::wstring _deviceName;
	PacketConfig _config;

	bool _isPPSLimitTriggered = false;

	std::chrono::time_point<std::chrono::steady_clock> _invokePoint;
	int _timeOffset = 0;
	std::chrono::time_point<std::chrono::steady_clock> _invokeRequestedPoint;
	int _delayedTime = 0;

	int _timeOffsetSaved = 0;

	EventCallbackWithChar _callback;
	bool _isCallbackRegistered = false;

	char _recvBuff[DEFAULT_BUFF_SIZE] { 0 };
	int	 _recvLen = 0;
};