#pragma once

#include <string>
#include <windows.h> 
#include "global.h"
#include "cspbot.h"
#include "qthread.h"

using namespace std;

enum stopType;

class Server:public QThread
{
	Q_OBJECT
protected:
	void run();
public:
	Server(QObject* parent = NULL){};
	~Server(){};
	bool started = false;
	bool normalStop = false;
	stopType TypeOfStop;
	bool createServer();
	bool forceStopServer();
	bool stopServer();
	bool sendCmd(string cmd);
	bool getPoll();
	BOOL bRet;
	char ReadBuff[8192];
	DWORD ReadNum=0;
	PROCESS_INFORMATION pi;
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	DEBUG_EVENT de;
};

class ServerPoll :public QThread
{
	Q_OBJECT
protected:
	void run();
public:
	ServerPoll(QObject* parent = NULL) {};
	~ServerPoll() {};
signals:
	void stoped();
};
