#include <motd/motd.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "qdebug.h"
#include "qstring.h"

using namespace std;
HMODULE h = LoadLibraryA("Motd.dll");
typedef char*(*funcPtrBE)(char* Host);
typedef char*(*funcPtrJE)(char* Host);
QString stdString2QString(std::string str);
std::string QString2stdString(QString str);


string motdbe(string host) {
	auto func = (funcPtrBE)GetProcAddress(h, "MotdBEJson");
	if (NULL == h || INVALID_HANDLE_VALUE == h)
	{
		return "{\"status\":\"error\"}";
	}
	char* st1 = const_cast<char*>(host.c_str());
	return func(st1);
}

string motdje(string host) {
	auto func = (funcPtrJE)GetProcAddress(h, "MotdJEJson");
	if (NULL == h || INVALID_HANDLE_VALUE == h)
	{
		return "{\"status\":\"error\"}";
	}
	char* st1 = const_cast<char*>(host.c_str());
	return func(st1);
}

bool bind(string qq,string name){
	YAML::Node player = YAML::LoadFile("data/player.yml");
	
	vector<string> player_list;
	vector<string> qq_list;
	for (YAML::Node i : player) {
		player_list.push_back(i["playerName"].as<string>());
		qq_list.push_back(i["qq"].as<string>());
	}

	//检测有没有已绑定的账号
	if (std::find(player_list.begin(), player_list.end(), name) == player_list.end() && \
		std::find(qq_list.begin(), qq_list.end(), qq) == qq_list.end()) {
		ofstream fout("data/player.yml");
		YAML::Node pl;
		pl["playerName"] = name;
		pl["qq"] = qq;
		pl["xuid"] = "";
		player.push_back(pl);
		fout << YAML::Dump(player);
	}
	else {
		return false;
	}
	return true;
}

bool unbind(string qq) {
	YAML::Node player = YAML::LoadFile("data/player.yml");
	vector<string> qq_list;
	for (YAML::Node i : player) {
		qq_list.push_back(i["qq"].as<string>());
	}

	//检测有没有已绑定的账号
	if (std::find(qq_list.begin(), qq_list.end(), qq) != qq_list.end()) {
		ofstream fout("data/player.yml");
		auto it = find(qq_list.begin(), qq_list.end(), qq);
		if (it != qq_list.end())
		{
			int index = it - qq_list.begin();
			player.remove(index);
		}
		fout << YAML::Dump(player);
	}
	else {
		return false;
	}
	return true;
}

bool bindXuid(string name, string xuid) {
	YAML::Node player = YAML::LoadFile("data/player.yml");
	for (YAML::Node i : player) {
		if(i["playerName"].as<string>() == name) {
			ofstream fout("data/player.yml");
			i["xuid"] = xuid;
			fout << YAML::Dump(player);
			return true;
		}
	}
	return false;
}

QString stdString2QString(std::string str) {
	QByteArray byteArray(str.c_str(), str.length());
	QString msg = byteArray;
	return msg;
}

std::string QString2stdString(QString str) {
	QByteArray bytes = str.toUtf8();
	std::string msg = bytes;
	return msg;
}

bool is_str_utf8(const char* str)
{
	unsigned int nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr = *str;
	bool bAllAscii = true;
	for (unsigned int i = 0; str[i] != '\0'; ++i) {
		chr = *(str + i);
		//判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
		if (nBytes == 0 && (chr & 0x80) != 0) {
			bAllAscii = false;
		}
		if (nBytes == 0) {
			//如果不是ASCII码,应该是多字节符,计算字节数
			if (chr >= 0x80) {
				if (chr >= 0xFC && chr <= 0xFD) {
					nBytes = 6;
				}
				else if (chr >= 0xF8) {
					nBytes = 5;
				}
				else if (chr >= 0xF0) {
					nBytes = 4;
				}
				else if (chr >= 0xE0) {
					nBytes = 3;
				}
				else if (chr >= 0xC0) {
					nBytes = 2;
				}
				else {
					return false;
				}
				nBytes--;
			}
		}
		else {
			//多字节符的非首字节,应为 10xxxxxx
			if ((chr & 0xC0) != 0x80) {
				return false;
			}
			//减到为零为止
			nBytes--;
		}
	}
	//违返UTF8编码规则
	if (nBytes != 0) {
		return false;
	}
	if (bAllAscii) { //如果全部都是ASCII, 也是UTF8
		return true;
	}
	return true;
}


string GbkToUtf8(const char* src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

string Utf8ToGbk(const char* src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

