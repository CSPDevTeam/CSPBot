
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "global.h"
#include "server.h"
#include <stdio.h>
#include <string>
#include <regex>
#include <qprocess.h>
#include <yaml-cpp/yaml.h>
#include <stdio.h>
#include "logger.h"
#include "mirai.h"
#include "CPython.h"

using namespace std;

enum stopType :int {
	normal = 1,
	force = 2,
	accident = 3,
} stoptype;

void catchInfo(QString line);
string getConfig(string key);
QString stdString2QString(std::string str);
std::string QString2stdString(QString str);
void selfCatchLine(QString qline);
Logger* serverLogger = new Logger("Server");

string m_replace(string strSrc,
	const string& oldStr, const string& newStr, int count = -1)
{
	string strRet = strSrc;
	size_t pos = 0;
	int l_count = 0;
	if (-1 == count) // replace all
		count = strRet.size();
	while ((pos = strRet.find(oldStr, pos)) != string::npos)
	{
		strRet.replace(pos, oldStr.size(), newStr);
		if (++l_count >= count) break;
		pos += newStr.size();
	}
	return strRet;
}

vector<string> split(const string& str, const string& pattern)
{
	vector<string> res;
	if (str == "")
		return res;
	string strs = str + pattern;
	size_t pos = strs.find(pattern);

	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(pattern);
	}

	return res;
}

bool Server::createServer()
{
	STARTUPINFO si = { 0 };
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = 0;
	BOOL bRet = CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0);
	BOOL bWet = CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0);
	if (bRet != TRUE){
		return false;
	}
	HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);
	SetStdHandle(STD_OUTPUT_HANDLE, g_hChildStd_OUT_Wr);
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdInput= g_hChildStd_IN_Rd;
	si.hStdError = g_hChildStd_OUT_Wr;
	si.hStdOutput = g_hChildStd_OUT_Wr;
	std:string s("runner.bat");
	wstring widstr = std::wstring(s.begin(), s.end());
	LPWSTR path = (LPWSTR)widstr.c_str();
	bRet = CreateProcess(NULL, path, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &this->pi);
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp);
	if (bRet != TRUE)
	{
		return false;
	}
	CloseHandle(g_hChildStd_OUT_Wr);
	this->ReadNum = ReadNum;
	this->started = true;
	cin.get();
	serverLogger->info(u8"服务器启动成功,PID为:" + std::to_string(pi.dwProcessId));
	
	
	return true;
}

void Server::run() {
	createServer();
	ServerPoll* sp = new ServerPoll();
	sp->start();
	if (started) {
		win->ThreadCallback(0);
		while (ReadFile(this->g_hChildStd_OUT_Rd, this->ReadBuff, 4096, &this->ReadNum, NULL))
		{
			if (started) {
				ReadBuff[ReadNum] = '\0';
				std::string line = ReadBuff;
				regex pattern("\033\\[(.+?)m");
				line = regex_replace(line, pattern, "");
				vector<string> words = split(line, "\n");
				for (string i : words) {
					string _line = m_replace(i, "\n", "");
					_line = m_replace(_line, "\r", "");
					if (_line != "") {
						
							QString qline = stdString2QString(_line);
							win->ipipelog(_line);
							catchInfo(qline);
							selfCatchLine(qline);
						
					}
				}
			}
			else {
				break;
			}
		}
	}
	
}

bool Server::forceStopServer() {
	try {
		QProcess process(win);
		QStringList argument;
		process.setProgram("taskkill");
		argument << "/F" << "/T" << "/PID" << stdString2QString(std::to_string(this->pi.dwProcessId));
		process.setArguments(argument);
		process.start();
		process.waitForStarted();
		process.waitForFinished();
		TypeOfStop = force;
		return true;
	}
	catch (...) {
		return false;
	}
}

bool Server::sendCmd(string cmd) {
	try {
		//检测停止
		if (cmd == getConfig("stopCmd") + "\n") {
			normalStop = true;
		}

		//Callback
		Callbacker cbe(EventCode::onSendCommand);
		cbe.insert("cmd", py::str(cmd));
		if (!cbe.callback()) {
			return false;
		}

		if (!WriteFile(this->g_hChildStd_IN_Wr, cmd.c_str(),cmd.length(), &this->ReadNum, NULL))
		{
			return false;
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

bool Server::stopServer() {
	try {
		Server::sendCmd(getConfig("stopCmd") + "\n");
		normalStop = true;
		return true;
	}
	catch (...) {
		return false;
	}
	
}


bool Server::getPoll() {
	try {
		if (this != nullptr) {
			return WaitForSingleObject(pi.hProcess, 0);
		}
		else {
			return false;
		}
	}
	catch (...) {
		return false;
	}
	
}

void ServerPoll::run() {
	while (1) {
		bool poll = server->getPoll();
		if(!poll){
			server->started = false;
			win->ipipelog(u8"[CSPBot] 进程已终止.");
			win->ThreadCallback(1);
			win->changeStatus(false);
			win->chenableForce(false);
			win->chLabel("world", "");
			win->chLabel("version","");

			//Callback
			Callbacker cbe(EventCode::onServerStop);
			cbe.callback();
			if (server->normalStop) {
				server->TypeOfStop = normal;
			}
			else {
				server->TypeOfStop = accident;
			}

			break;
		}
		else {
			win->changeStatus(true);
		}
		Sleep(1 * 1000);
	}
}

void catchInfo(QString line) {
	QRegExp world("worlds\\/(.+)\\/db");
	QRegExp version("Version\\s(.+)");
	QRegExp PID("PID\\s(.+)[?]");
	int world_pos = world.indexIn(line);
	int version_pos = version.indexIn(line);
	int pid_pos = PID.indexIn(line);
	if (world_pos > -1) {
		string world_string = "<font color = \"#f89b9b\">" + QString2stdString(world.cap(1)) + "</font>";
		win->chLabel("world",world_string);
	}
	else if (version_pos > -1) {
		string version_raw = QString2stdString(version.cap(1));
		if (version_raw.find('(') != string::npos) {
			QRegExp version_R("(.+)\\(");
			int version_R_pos = version_R.indexIn(stdString2QString(version_raw));
			if (version_R_pos > -1) {
				version_raw = QString2stdString(version_R.cap(1));
			}
		}
		string version_string = "<font color = \"#f89b9b\">" + version_raw + "</font>";
		win->chLabel("version",version_string);
	}
	else if (pid_pos > -1) {
		win->ipipelog(u8"[CSPBot] 提示:已有一个PID为"+ QString2stdString(PID.cap(1))+u8"的相同目录进程，是否结束进程?（确认请输入y,取消请输入n)");
	}
}

YAML::Node getRegular() {
	YAML::Node node = YAML::LoadFile("data/regular.yml"); //读取regular配置文件
	return node;
}

void selfCatchLine(QString line) {
	YAML::Node regular = getRegular();

	//读取正则组
	for (YAML::Node i : regular) {
		string Regular = i["Regular"].as<string>();
		string Action = i["Action"].as<string>();
		string From = i["From"].as<string>();
		bool Permissions = i["Permissions"].as<bool>();

		QRegExp r(stdString2QString(Regular));
		int r_pos = r.indexIn(line);
		//qDebug() << r << r_pos;
		//执行操作
		if (r_pos > -1 && From == "console") {
			string Action_type = Action.substr(0, 2);
			int num = 0;
			for (auto &replace : r.capturedTexts()) {
				Action = m_replace(Action, "$" + std::to_string(num), QString2stdString(replace));
				num++;
			}
			if(Action_type == "<<") {
				string cmd = Action.erase(0, 2);
				server->sendCmd(cmd+"\n");
			}
			else if (Action_type == ">>") {
				string cmd = Action.erase(0, 2);
				qDebug() << stdString2QString(cmd);
				mi->sendAllGroupMsg(cmd);
			}
			else {
				CustomCmd(Action);
			}
		}
	}
}
string fmtMotdBE(string msgJson, string returnMsg);
string fmtMotdJE(string msgJson, string returnMsg);
string motdbe(string host);
string motdje(string host);
bool bind(string qq, string name);
bool unbind(string qq);

void CustomCmd(string cmd,string group) {
	vector<string> sp = split(cmd, " ");
	string Action_Type = sp[0];
	if (Action_Type == "bind") {
		if (sp.size() > 2) {
			bind(sp[1], sp[2]);
		}
		else {
			mi->sendGroupMsg(group, u8"参数错误!");
		}
	}
	else if (Action_Type == "unbind") {
		if (sp.size() > 1) {
			unbind(sp[1]);
		}
		else {
			mi->sendGroupMsg(group, u8"参数错误!");
		}
	}
	else if (Action_Type == "motdbe") {
		if (sp.size() > 2) {
			QRegExp r("(\\w.+):(\\w+)");
			int r_pos = r.indexIn(stdString2QString(sp[1]));
			if (r_pos > -1) {
				string motd_respone = motdbe(sp[1]);
				string fmt_respone;
				fmt_respone = fmtMotdBE(motd_respone, sp[2]);
				mi->sendGroupMsg(group, fmt_respone);
			}
			else {
				mi->sendGroupMsg(group, u8"Motd地址错误!");
			}
		}
		else {
			mi->sendGroupMsg(group, u8"参数错误!");
		}
	}
	else if (Action_Type == "motdje") {
		if (sp.size() > 2) {
			QRegExp r("(\\w.+):(\\w+)");
			int r_pos = r.indexIn(stdString2QString(sp[1]));
			if (r_pos > -1) {
				string motd_respone = motdje(sp[1]);
				string fmt_respone;
				fmt_respone = fmtMotdJE(motd_respone, sp[2]);
				mi->sendGroupMsg(group, fmt_respone);
			}
			else {
				mi->sendGroupMsg(group, u8"Motd地址错误!");
			}
		}
		else {
			mi->sendGroupMsg(group, u8"参数错误!");
		}
	}
	else if (Action_Type == "start") {
		server = new Server();
		server->start();
		win->chenableForce(true);
		win->ipipelog(u8"[CSPBot] 已向进程发出启动命令");
	}
	else if (Action_Type == "stop") {
		if (server->started) {
			server->stopServer();
		}
		else if (server->started != false && group != "0") {
			mi->sendGroupMsg(group, "服务器不在运行中");
		}
	}
}