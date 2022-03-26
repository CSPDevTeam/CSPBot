
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
#include <Command.h>
#include "CPython.h"

using namespace std;

enum stopType :int {
	normal,
	force,
	accident,
} stoptype;

void catchInfo(QString line);
string getConfig(string key);
string FmtConsoleRegular(string cmd);
QString stdString2QString(std::string str);
std::string QString2stdString(QString str);
bool bindXuid(string name, string xuid);
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

QString getColoredLine(string line);
bool is_str_utf8(const char* str);
string GbkToUtf8(const char* src_str);
string Utf8ToGbk(const char* src_str);

void Server::run() {
	createServer();
	ServerPoll* sp = new ServerPoll();
	sp->start();
	if (started) {
		win->OtherCallback("onServerStart");
		while (ReadFile(this->g_hChildStd_OUT_Rd, this->ReadBuff, 8192, &this->ReadNum, NULL))
		{
			if (started) {
				ReadBuff[ReadNum] = '\0';
				std::string line = ReadBuff;
				if (!is_str_utf8(line.c_str())) {
					line = GbkToUtf8(line.c_str());
				}
				qDebug() << stdString2QString(line);
				regex pattern("\033\\[(.+?)m");
				string nocolor_line = regex_replace(line, pattern, "");
				vector<string> words = split(line, "\n");
				vector<string> nocolor_words = split(nocolor_line, "\n");
				for (string i : words) {
					string _line = m_replace(i, "\n", "");
					_line = m_replace(_line, "\r", "");
					if (_line != "") {
						QString qline = stdString2QString(_line);
						QString coloredLine = getColoredLine(_line);
						if (coloredLine != "") {
							win->ipipelog(QString2stdString(coloredLine));
						}
					}
				}

				for (string i : nocolor_words) {
					string _line = m_replace(i, "\n", "");
					_line = m_replace(_line, "\r", "");
					if (_line != "") {
						QString qline = stdString2QString(_line);
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
		std::unordered_map<string, string> p;
		p.emplace("cmd", cmd);
		win->OtherCallback("onSendCommand", p);

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
			win->OtherCallback("onServerStop");
			win->changeStatus(false);
			win->chenableForce(false);
			win->chLabel("world", "");
			win->chLabel("version","");

			//Callback
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
	QRegExp Join("Player\\sconnected:\\s(.+),\\sxuid:\\s(.+)");
	int world_pos = world.indexIn(line);
	int version_pos = version.indexIn(line);
	int pid_pos = PID.indexIn(line);
	int join_pos = Join.indexIn(line);
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
	else if (join_pos > -1) {
		bindXuid(QString2stdString(Join.cap(1)), QString2stdString(Join.cap(2)));
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
		//qDebug() << line << r << r_pos;
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
				cmd = FmtConsoleRegular(cmd);
				server->sendCmd(cmd+"\n");
			}
			else if (Action_type == ">>") {
				string cmd = Action.erase(0, 2);
				cmd = FmtConsoleRegular(cmd);
				mi->sendAllGroupMsg(cmd);
			}
			else {
				string ac = FmtConsoleRegular(Action);
				Command::CustomCmd(ac,"");
			}
		}
	}
}
