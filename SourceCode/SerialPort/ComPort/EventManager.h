#pragma once
#include <string>
#include <functional>

typedef void (*EventCallback)();
typedef void (*EventCallbackTest)(char* data);

typedef std::function<void()> ThreadCallback;

struct LogManager
{
	EventCallback Logging;

	int level = 0;
	std::wstring log;

	void Error(std::wstring _log)     { level = 0; log = _log; if (Logging) Logging(); }
	void Normal(std::wstring _log)    { level = 1; log = _log; if (Logging) Logging(); }
	void Developer(std::wstring _log) { level = 2; log = _log; if (Logging) Logging(); }
};

struct EventManager
{
	EventCallback _onConnected;
	EventCallback _onConnectionFailed;
	EventCallback _onDisconnected;
	EventCallback _onDataReceived;

	void RegisterEventCallbacks(EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected, EventCallback onDataReceived)
	{ 
		_onConnected = onConnected;
		_onConnectionFailed = onConnectionFailed;
		_onDisconnected = onDisconnected;
		_onDataReceived = onDataReceived;
	}

	void Invoke(std::string type)
	{ 
		if      (type == "onConnected")        { if (_onConnected) _onConnected(); }
		else if (type == "onConnectionFailed") { if (_onConnectionFailed) _onConnectionFailed(); }
		else if (type == "onDisconnected")     { if (_onDisconnected) _onDisconnected(); }
		else if (type == "onDataReceived")     { if (_onDataReceived) _onDataReceived(); }
		else printf("Invalid event type was given\n");
	}
};