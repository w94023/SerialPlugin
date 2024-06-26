#include "StructManager.h"
#include <functional>
#include <cstdio>
#include <cwchar>
#include <sstream>
#include <vector>
#include <Windows.h>
//#include <locale>
//#include <codecvt>

std::wstring CharToWString(const char* utf8Str) {
    // UTF-8 ���ڿ��� ���� ��� (NULL ���� ���� ����)
    int strLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

    // wstring ũ�⿡ �´� ���� �Ҵ�
    std::wstring wstr(strLength, L'\0');

    // MultiByteToWideChar�� ����Ͽ� UTF-8���� UTF-16���� ��ȯ
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wstr[0], strLength);

    return wstr;
}

const wchar_t* CharToWChar(const char* utf8Str)
{
	return CharToWString(utf8Str).c_str();
}

std::wstring StringToWString(const std::string& input)
{
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
 //   
 //   // ��ȯ ����
 //   std::wstring wstr = converter.from_bytes(input);
 //   
 //   return wstr;
	std::wstring result;
    result.reserve(input.size());
    for (auto c : input) {
        // ASCII ���� ���� ���ڸ� ó��
        if (c >= 0 && c <= 127) {
            result.push_back(c);
        } else {
            // ASCII ������ ��� ���ڿ� ���� ������ ��ȯ �ʿ�
            // ���� ���, ���� ó���� Ȯ��� ���� ó�� ���� �߰�
        }
    }
    return result;
}

void LogManager::SetCallback(EventCallbackWithChar callback)
{
	_callback = callback;
	_isEventRegistered = true;
}

void LogManager::SetLevel(int level)
{
	_logLevel = level;
}

void LogManager::Delete(char* log)
{
	delete[] log;
}

void LogManager::Error(std::wstring log, ...)     { va_list args; va_start(args, log); if (_logLevel >= 0) { ExportLog(log, args); va_end(args); } }
void LogManager::Normal(std::wstring log, ...)    { va_list args; va_start(args, log); if (_logLevel >= 1) { ExportLog(log, args); va_end(args); } }
void LogManager::Developer(std::wstring log, ...) { va_list args; va_start(args, log); if (_logLevel >= 2) { ExportLog(log, args); va_end(args); } }


void LogManager::ExportLog(std::wstring log, va_list args)
{
	std::wstringstream wss;
	const wchar_t* logStr = log.c_str();

	while (*logStr != L'\0') {
		if (*logStr == L'%') {
			++logStr;
			switch (*logStr) {
				case L'd': {
					int d = va_arg(args, int);
					wss << d;
					break;
				}
				case L's': {
					//wchar_t ����� ���ڿ��� ����մϴ�.
					const wchar_t* s = va_arg(args, const wchar_t*);
					wss << s;
					break;
				}
				case L'f': {
					// ���� ���ڿ��� float�� ������ ���� double�� �����;� �մϴ�
					double d = va_arg(args, double);
					wss << d;
					break;
				}
			}
		}
		else {
			wss << *logStr;
		}
		++logStr;
	}

	std::wstring formattedString = wss.str();

	int* strLength = new int(2 * formattedString.length());
	char* str = new char[*strLength];
	memcpy(str, formattedString.c_str(), *strLength);

	if (_isEventRegistered) _callback(str, strLength);
}


void PacketConfig::Initialize()
{
	if (recvByteSize < 1) recvByteSize = 1;
	if (recvBufferSize < 1) recvBufferSize = 1;
	if (recvByteSize > recvBufferSize) recvByteSize = recvBufferSize;
	if (packetLength < 0) packetLength = 0;
	if (stopByteLength < 0) stopByteLength = 0;
	if (PPSLimit < 0) PPSLimit = 0;
	if (flushRatio > 1.0f) flushRatio = 1.0f;
	if (flushRatio < 0.01f) flushRatio = 0.01f;
}


std::wstring ConnectionConfig::GetDeviceName()
{
	return std::wstring(deviceName);
}
