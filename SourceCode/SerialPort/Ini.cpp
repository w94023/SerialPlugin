// #include "../PCH/stdafx.h"
#include "Ini.h"
#include <stdio.h>

#pragma warning(disable:4996)

CIni::CIni(const char* fileName)
{
	const char* appName = "Property";
	_appName = new char[strlen(appName) + 1];
	strcpy(_appName, appName);

	//::GetCurrentDirectory(MAX_PATH, _fileName);
	strcat(_fileName, "\\");
	strcat(_fileName, fileName);

	// strncpy (_fileName, fileName, MAX_PATH);
}

CIni::~CIni()
{
}

void CIni::SaveString(const char* keyName, char* value)
{
	int strLen = strlen(value);
	char* strQuot = new char[strLen + 3];

	strQuot[0] = '\"';
	strncpy(strQuot + 1, value, strLen);
	strQuot[strLen + 1] = '\"';
	strQuot[strLen + 2] = '\0';

	//WritePrivateProfileString(_appName, keyName, strQuot, _fileName);

	delete[] strQuot;
}

void CIni::SaveInt(const char* keyName, long value)
{
	char str[32 + 1];

	_snprintf(str, 32, "%d", value);
	str[32] = '\0';

	//WritePrivateProfileString(_appName, keyName, str, _fileName);
}

void CIni::SaveFloat(const char* keyName, double value)
{
	char str[64 + 1];

	_snprintf(str, 64, "%g", value);
	str[64] = '\0';

	//WritePrivateProfileString(_appName, keyName, str, _fileName);
}

void CIni::LoadString(const char* keyName, char* returnedString, DWORD nSize, const char* defaultValue)
{
	//GetPrivateProfileString(_appName, keyName, defaultValue, returnedString, nSize, _fileName);
}

long CIni::LoadInt(const char* keyName, long defaultValue)
{
	//return GetPrivateProfileInt(_appName, keyName, defaultValue, _fileName);
	return 0;
}

double CIni::LoadFloat(const char* keyName, double defaultValue)
{
	char defStr[64 + 1];
	_snprintf(defStr, 64, "%g", defaultValue);
	defStr[64] = '\0';

	char returnedString[128 + 1] = "";
	//GetPrivateProfileString(_appName, keyName, defStr, returnedString, 128, _fileName);
	returnedString[128] = '\0';

	return atof(returnedString);
}

void CIni::Transfer(bool bSave, const char* keyName, char* value, DWORD nSize, const char* defaultValue)
{
	if (bSave) SaveString(keyName, value);
	else       LoadString(keyName, value, nSize, defaultValue);
}

void CIni::Transfer(bool bSave, const char* keyName, bool& value, bool defaultValue)
{
	if (bSave)   SaveInt(keyName, value ? 1 : 0);
	else value = LoadInt(keyName, defaultValue) ? true : false;
}

void CIni::Transfer(bool bSave, const char* keyName, char& value, char defaultValue)
{
	if (bSave)        SaveInt(keyName, (int)value);
	else value = (int)LoadInt(keyName, (int)defaultValue);
}

void CIni::Transfer(bool bSave, const char* keyName, int& value, int defaultValue)
{
	if (bSave)        SaveInt(keyName, (int)value);
	else value = (int)LoadInt(keyName, (int)defaultValue);
}

void CIni::Transfer(bool bSave, const char* keyName, long& value, long defaultValue)
{
	if (bSave)        SaveInt(keyName, (int)value);
	else value = (int)LoadInt(keyName, (int)defaultValue);
}

void CIni::Transfer(bool bSave, const char* keyName, DWORD& value, DWORD defaultValue)
{
	if (bSave)        SaveInt(keyName, (int)value);
	else value = (int)LoadInt(keyName, (int)defaultValue);
}

void CIni::Transfer(bool bSave, const char* keyName, float& value, float defaultValue)
{
	if (bSave)          SaveFloat(keyName, value);
	else value = (float)LoadFloat(keyName, defaultValue);
}

void CIni::Transfer(bool bSave, const char* keyName, double& value, double defaultValue)
{
	if (bSave)   SaveFloat(keyName, value);
	else value = LoadFloat(keyName, defaultValue);
}