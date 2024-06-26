// #include "stdafx.h"
#include "AppIni.h"

CAppIni::CAppIni(const char* inipath) : CIni(inipath)
{
	LoadData();
}

CAppIni::~CAppIni()
{
	SaveData();
}

void CAppIni::LoadData()
{
	TransferAllData(false);
}

void CAppIni::SaveData()
{
	TransferAllData(true);
}

void CAppIni::TransferAllData(bool bSave)
{
	Transfer(bSave, "options",			(DWORD&)options,	0x3);

	Transfer(bSave, "comPort",			comPort,			MAX_PATH,	"\\\\.\\COM1");
	Transfer(bSave, "comBps",			comBps,				115200);
	Transfer(bSave, "comDatabits",		comDatabits,		8);
	Transfer(bSave, "comParity",		comParity,			0);
	Transfer(bSave, "comStopbits",		comStopbits,		0);
	Transfer(bSave, "comFlowcontrol",	comFlowcontrol,		0);
}