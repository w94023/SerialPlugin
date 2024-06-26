#pragma once
#include <string>
#include <Windows.h>
#include "Ini.h"

enum eConnectionType {
	CONNECTION_UDP,		// UDP, 
	CONNECTION_TCPS,	// TCP Server, 
	CONNECTION_TCPC,	// TCP Client, 
	CONNECTION_COM,		// Serial(COM).
	CONNECTION_NIDAQ,   // NI DAQ
	CONNECTION_BLE	    // Bluetooth low energy (BLE)
};

enum eReplaceChar {
	REPLACE_NOTUSE,		// ������
	REPLACE_ESCAPE,		// Escape Sequence
	REPLACE_ASCII,		// ASCII ����
	REPLACE_SPACE,		// ���� ����( )
	REPLACE_UNDERSCORE,	// ���� ����(_)
	REPLACE_PERIOD,		// ��ħǥ ����(.)
};

struct sBitOptions {
	DWORD MESSAGE_DISPLAY_SEND : 1;	// �� ��ȭ���ڿ��� Send Message�� �α� â�� ��� �� �� ���θ� ����
	DWORD MESSAGE_DISPLAY_RECV : 1;	// �� ��ȭ���ڿ��� Recv Message�� �α� â�� ��� �� �� ���θ� ����
	DWORD MESSAGE_DISPLAY_HEX : 1;	// �� ��ȭ���ڿ� ǥ�õǴ� �޽��� ������ HEXA �ڵ�� ���� ASCII ���ڷ� ���� ����
	DWORD INPUT_SHOW_HEX : 1;	// Send �Է� �ڽ��� ǥ�õǴ� �޽��� ������ HEXA �ڵ�� ���� ASCII ���ڷ� ���� ����, ���⼭ ASCII ���ڿ����� Escape sequence char.�� ��밡��
	DWORD SHOW_SHORTKEY_DLG : 1;	// ����Ű �Է� ��ȭ���ڸ� ǥ���� �� ���θ� ����
	DWORD HIDE_SHORTKEY_PART : 1;	// ����Ű �Է� ��ȭ���ڿ��� ����Ű �κ��� ������ �� ����
	DWORD CONNECTION_TYPE : 2;	// ���� ���� ����: eConnectionType ����
	DWORD PLAY_SOUND : 1;	// ���� ��ȭ����: ���� �ǰų� ������ ����� �� ����� �˸� ���
	DWORD ASCII_7BITS : 1;	// ���� ��ȭ����: Recv Message�� 7-bit ����ũ�� ���� �� ����
	DWORD CONNECT_AT_STARTUP : 1;	// ���� ��ȭ����: ���α׷� ����� ���� �������� ����
	DWORD LINEEND_AUTO_CRLF : 1;	// ���� ��ȭ����: �ۼ��� ���ڿ� ������ �ڵ� �ٹٲ�
	DWORD LINEFEED_WHEN_CR_LF : 1;	// ���� ��ȭ����: �ٹٲ� ���ڸ� ������ �ٹٲ�
	DWORD REPLACE_UNDISPLAY_CHAR : 4;	// ���� ��ȭ����: ǥ���� �� ���� ���� ��ü
	DWORD DISPLAY_SENDRECV_MARK : 1;
	DWORD DISPLAY_SENDRECV_TIME : 3;
	DWORD DISPLAY_WORD_WRAP : 1;
};

class CAppIni : public CIni
{
public:
	sBitOptions options;

	char	comPort[MAX_PATH + 1];
	int		comBps;
	char	comDatabits;
	char	comParity;
	char	comStopbits;
	char	comFlowcontrol;
	DWORD	remoteIpAddr;
	DWORD	localIpAddr;
	DWORD	portNum;

public:
	CAppIni(const char* ini_path);
	virtual ~CAppIni();

	void LoadData();
	void SaveData();

private:
	void TransferAllData(bool bSave);
};

extern CAppIni g_ini;
