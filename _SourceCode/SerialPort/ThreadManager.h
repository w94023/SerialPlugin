#pragma once
#include "PortManager.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <iomanip> // setw, setprecision
#include <sstream> // wstringstream
// CPU �� �޸� ��뷮 Ȯ��
#include <windows.h>
#include <pdh.h>
#include <iostream>
#pragma comment(lib, "pdh.lib")

typedef std::function<void(bool)> ThreadCallback;

class CThreadManager
{
public:
	CThreadManager(LogManager& log) : _log(log)
	{
		_isAlive = true;
	}

	~CThreadManager()
	{
		_isAlive = false;
		StopThread();
	}

	void SetResourceLimit(ResourceLimitType resourceLimitType, double limit)
	{
		switch (resourceLimitType) {
			case PPS:     _ppsLimit    = (int)limit; break;
			case CPU:     _cpuLimit    = limit;      break;
			case Memory : _memoryLimit = (int)limit; break;
		}

		std::cout << _ppsLimit << ", " << _cpuLimit << ", " << _memoryLimit << std::endl;

		if (_ppsLimit > 0 && _ppsLimit < 1000) {
			_threadSleepMilliSeconds = 1000 / _ppsLimit;
		}
		else {
			_threadSleepMilliSeconds = 1;
		}
	}

	void CheckResource(double* cpuUsage, int* memoryRemained)
	{
		std::wstringstream wss;
		wss << std::fixed << std::setprecision(2) << _cpuUsage;
		std::wstring cpuUsageString = wss.str();

		_log.Developer(L"Current CPU Usage (%) : " + cpuUsageString + L" / Memory remained (MB) : " + std::to_wstring(_memoryRemained));
		*cpuUsage	    = _cpuUsage;
		*memoryRemained = _memoryRemained;
	}

	void Test(double cpuUsage, int memoryRemained)
	{
		_cpuUsage = cpuUsage;
		_memoryRemained = memoryRemained;
	}

	void RegisterCallback(ThreadCallback callback)
	{
		_callback = callback;
	}

	void StartThread()
	{
		_stopDataThread			 = false;
		_stopResourceThread      = false;
		isThreadStopped			 = false;

		PdhOpenQuery(NULL, NULL, &cpuQuery);
		PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal); // CPU ��뷮 ī���� �߰�

		std::thread dataThread(&CThreadManager::ThreadFunc, this);
		dataThread.detach();
		std::thread resourceThread(&CThreadManager::ResourceCheckThread, this);
		resourceThread.detach();
	}

	void StopThread()
	{
		_stopDataThread     = true;
		_stopResourceThread = true;
		PdhCloseQuery(cpuQuery);
	}

	bool isThreadStopped = false;

private:
	int countDigits(int number) {
		if (number == 0) {
			return 1; // 0�� �� �ڸ���
		}

		int count = 0;
		while (number != 0) {
			number /= 10; // ���ڸ� 10���� ������
			count++; // �ڸ��� ����
		}
		return count;
	}

	void ThreadFunc()
	{
		_isDataThreadStopped = false;

		while (true) {
			if (_stopDataThread || !_isAlive) break;
			if (!_stopDataThread && _isAlive) {
				auto currentTimePoint = std::chrono::steady_clock::now();
				int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint - _threadWorkPoint).count();

				if (elapsedTime >= _threadSleepMilliSeconds * _multiplier) {
					if (_callback) _callback(true);
					_threadWorkPoint = std::chrono::steady_clock::now();
				}
				else {
					if (_callback) _callback(false);
				}
			}
		}

		_isDataThreadStopped = true;
		if (_isDataThreadStopped && _isDataThreadStopped) {
			isThreadStopped = true;
		}
	}

	void ResourceCheckThread()
	{
		_isResourceThreadStopped = false;

		Sleep(1000);

		while (true) {
			if (_stopResourceThread || !_isAlive) break;
			if (!_stopResourceThread && _isAlive) {
				PdhCollectQueryData(cpuQuery);
				// CPU ��뷮�� ������
				PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
				// CPU ��뷮 ���
				_cpuUsage = counterVal.doubleValue;

				// RAM ��뷮 Ȯ��
				MEMORYSTATUSEX memInfo;
				memInfo.dwLength = sizeof(MEMORYSTATUSEX);
				GlobalMemoryStatusEx(&memInfo);

				DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
				DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

				//std::cout << "Total RAM: " << totalPhysMem / (1024 * 1024) << " MB" << std::endl;
				//std::cout << "Used RAM: " << physMemUsed / (1024 * 1024) << " MB" << std::endl;
				_memoryRemained = (int)totalPhysMem / (1024 * 1024) - (int)physMemUsed / (1024 * 1024);

				//std::cout << "CPU Limit : " << _cpuLimit << " Memory Limit : " << _memoryLimit << std::endl;

				if (_cpuLimit > 0 && _cpuUsage > _cpuLimit) {
					AdjustIntervalMultiplier();
				} 

				if (_memoryLimit > 0 && _memoryLimit < _memoryRemained) {
					AdjustIntervalMultiplier();
				} 
			}

			Sleep(1000);
		}

		_isResourceThreadStopped = true;
		if (_isDataThreadStopped && _isDataThreadStopped) {
			isThreadStopped = true;
		}
	}

	void AdjustIntervalMultiplier()
	{
		if (!_isIntervalAdjustingDone) return;
		std::thread thread(&CThreadManager::AdjustIntervalMultiplierThreadFunc, this);
		thread.detach();
	}

	void AdjustIntervalMultiplierThreadFunc()
	{
		_isIntervalAdjustingDone = false;

		_multiplierModifier++;
		_multiplier = (int)std::pow(10, _multiplierModifier);
		_log.Developer(L"The receiving thread's speed has been adjusted [CPU usage] : " + std::to_wstring(_cpuUsage) + L"% [RAM remained] : " + std::to_wstring(_memoryRemained) + 
					   L"% [Interval] : " + std::to_wstring(_threadSleepMilliSeconds * _multiplier) + L"ms");
		
		int count = 0;
		for (int i = 0; i < 10; i++) {
			// 10�ʰ� ���
			if (!_isAlive) return;
			Sleep(1000);
		}

		if (_isAlive) {
			if (_cpuLimit > 0 && _cpuUsage > _cpuLimit) {
				AdjustIntervalMultiplierThreadFunc();
			}
			else if (_memoryRemained > 0 && _memoryRemained < _memoryLimit) {
				AdjustIntervalMultiplierThreadFunc();
			}
			else {
				_multiplierModifier = 0;
				_multiplier = 1;
				_log.Developer(L"The receiving thread's speed has been adjusted [CPU usage] : " + std::to_wstring(_cpuUsage) + L"% [RAM remained] : " + std::to_wstring(_memoryRemained) + 
							L"% [Interval] : " + std::to_wstring(_threadSleepMilliSeconds * _multiplier) + L"ms");
				_isIntervalAdjustingDone = true;
			}
		}
	}

	// �����忡 ����� �ݹ�
	ThreadCallback _callback = NULL;

	// ������ ���͹� ������
	std::chrono::time_point<std::chrono::steady_clock> _threadWorkPoint;
	int _threadSleepMilliSeconds = 1;
	int _multiplier = 1;
	int _ppsLimit = -1;
	int _multiplierModifier = 0;

	// CPU �� �޸� ��뷮 Ȯ��
	PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;
	PDH_FMT_COUNTERVALUE counterVal;

	double _cpuLimit = -1;
	double _cpuUsage;
	int    _memoryLimit = -1;
	int    _memoryRemained;

	bool _isIntervalAdjustingDone = true;

	// �α�
	LogManager&	_log;

	bool _isDataThreadStopped = false;
	bool _isResourceThreadStopped = false;
	bool _stopDataThread = false;
	bool _stopResourceThread = false;

	bool _isAlive = true;
};
