#include <motd/motd.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <string>
#include "qdebug.h"
#include "qstring.h"

using namespace std;
HMODULE h = LoadLibraryA("Motd.dll");
typedef char*(*funcPtrBE)(char* Host);
typedef char*(*funcPtrJE)(char* Host);


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
