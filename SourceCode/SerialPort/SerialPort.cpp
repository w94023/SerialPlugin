#include "SerialPort.h"

void CheckMemory()
{
	_log.Developer(L"Number of instances currently created : " + std::to_wstring(_manager.size()));
}

int GetLog(char* log)
{
	memcpy(log, _log.log.c_str(), _log.log.size() * 2);
	return _log.level;
}