#pragma once 
#include <windows.h>
#include <iostream>
#include <vector>
#include "StructManager.h"

using namespace std;

//extern void OnSocketError (const wchar_t *prefix, LogManager& log);
extern bool SocketReady (int sockfd, int timeout);
extern DWORD GetLocalIP ();
extern int SocketCountRx (int sockfd);
extern const char *ip_addr_str(DWORD ip);
extern const char *ip_addr_str(BYTE ip[4]);
extern const char *mac_addr_str(BYTE mac[6]);

extern vector<DWORD> GetHostIPAddress();
