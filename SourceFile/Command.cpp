#include "command.h"
#include <string>
#include <vector>
#include <ctime>
#include <optional>
#include <sstream>
#include "global.h"
#include "server.h"
#include "cspbot.h"
#include "mirai.h"
#include "CPython.h"

using namespace std;

string fmtMotdBE(string msgJson, string returnMsg);
string fmtMotdJE(string msgJson, string returnMsg);
string motdbe(string host);
string m_replace(string strSrc, const string& oldStr, const string& newStr, int count = -1);
string motdje(string host);
bool bind(string qq, string name);
bool unbind(string qq);

vector<string> Command::SplitCommand(const std::string& paras)
{
    if (paras.empty())
        return vector<string>();

    vector<string> res;
    string now, strInQuote = "";
    istringstream strIn(paras);
    while (strIn >> now)
    {
        if (!strInQuote.empty())
        {
            strInQuote = strInQuote + " " + now;
            if (now.back() == '\"')
            {
                strInQuote.pop_back();
                res.push_back(strInQuote.erase(0, 1));
                strInQuote = "";
            }
        }
        else
        {
            if (now.front() == '\"')
            {
                if (now.back() == '\"')
                {
                    now = now.substr(1, now.size() - 2);
                    res.push_back(now);
                }
                else
                    strInQuote = now;
            }
            else
                res.push_back(now);
        }
    }
    if (!strInQuote.empty())
    {
        istringstream leftIn(strInQuote);
        while (leftIn >> now)
            res.push_back(now);
    }
	for (int i = 0; i < res.size(); i++) {
		res[i] = m_replace(res[i], "\"", "");
	}
    return res;
}

void Command::CustomCmd(string cmd, string group) {
	vector<string> sp = SplitCommand(cmd);
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
	else if (command.find(Action_Type) != command.end()) {
		qDebug() << stdString2QString(Action_Type);
		vector<string> args;
		int num=0;
		for (auto& i : sp) {
			if(num != 0){
				args.push_back(i);
			}
			num++;
		}
		win->CommandCallback(Action_Type, args);
	}
}