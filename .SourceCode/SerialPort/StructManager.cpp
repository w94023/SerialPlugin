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
    // UTF-8 문자열의 길이 계산 (NULL 종료 문자 제외)
    int strLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

    // wstring 크기에 맞는 버퍼 할당
    std::wstring wstr(strLength, L'\0');

    // MultiByteToWideChar를 사용하여 UTF-8에서 UTF-16으로 변환
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
 //   // 변환 수행
 //   std::wstring wstr = converter.from_bytes(input);
 //   
 //   return wstr;
	std::wstring result;
    result.reserve(input.size());
    for (auto c : input) {
        // ASCII 범위 내의 문자만 처리
        if (c >= 0 && c <= 127) {
            result.push_back(c);
        } else {
            // ASCII 범위를 벗어난 문자에 대해 적절한 변환 필요
            // 예를 들어, 에러 처리나 확장된 문자 처리 로직 추가
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
					//wchar_t 기반의 문자열을 기대합니다.
					const wchar_t* s = va_arg(args, const wchar_t*);
					wss << s;
					break;
				}
				case L'f': {
					// 가변 인자에서 float을 가져올 때는 double로 가져와야 합니다
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
