#include "logger.h"
#include <Nlohmann/json.hpp>
#include <iostream>
#include <windows.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include <time.h>
#include <yaml-cpp/yaml.h>
#pragma comment(lib,"pdh.lib")

using json = nlohmann::json;

string m_replace(string strSrc, const string& oldStr, const string& newStr, int count = -1);
string getConfig(string key);

string fmtMotdBE(string msgJson, string returnMsg) {
	string tempReturnMsg = returnMsg;
	tempReturnMsg = m_replace(tempReturnMsg, "%s", " ");
	json msg;
	msg = json::parse(msgJson);
	if (msg["status"] == "online") {
		for (json::iterator it = msg.begin(); it != msg.end(); ++it) {
			string key = it.key();
			string value;
			if (key != "online" && key != "max" && key != "online" && key != "delay" && key != "agreement") {
				value = it.value().get<string>();
			}
			else {
				value = std::to_string(it.value().get<int>());
			}
			tempReturnMsg = m_replace(tempReturnMsg, "{" + key + "}", value);
		}
		return tempReturnMsg;
	}
	else {
		return u8"发生错误!原因:服务器状态为" + msg["status"].get<string>();
	}
	
	
}

string fmtMotdJE(string msgJson, string returnMsg) {
	string tempReturnMsg = returnMsg;
	tempReturnMsg = m_replace(tempReturnMsg, "%s", " ");
	json msg;
	msg = json::parse(msgJson);
	if (msg["status"] == "online") {
		//Motd
		string motdjson = msg["motd"];
		//Host
		string host = msg["host"];
		//Player
		string max = std::to_string(msg["max"].get<int>());
		string online = std::to_string(msg["online"].get<int>());
		string player = "";
		for (auto& i : msg["sample"]) {
			string plName = i["name"].get<string>();
			player += plName + "\n";
		}
		//Version
		string name = msg["version"].get<string>();
		string protocal = std::to_string(msg["agreement"].get<int>());
		//delay
		string delay = std::to_string(msg["delay"].get<int>());
		tempReturnMsg = m_replace(tempReturnMsg, "{motd}", motdjson);
		tempReturnMsg = m_replace(tempReturnMsg, "{max}", max);
		tempReturnMsg = m_replace(tempReturnMsg, "{online}", online);
		tempReturnMsg = m_replace(tempReturnMsg, "{player}", player);
		tempReturnMsg = m_replace(tempReturnMsg, "{version}", name);
		tempReturnMsg = m_replace(tempReturnMsg, "{agreement}", protocal);
		tempReturnMsg = m_replace(tempReturnMsg, "{delay}", delay);
		tempReturnMsg = m_replace(tempReturnMsg, "{host}", host);
		return tempReturnMsg;
	}
	else {
		return u8"发生错误!原因:服务器状态为" + msg["status"].get<string>();
	}
}

std::unordered_map<string, string> getRam() {
	MEMORYSTATUSEX statusex;
	statusex.dwLength = sizeof(statusex);
	GlobalMemoryStatusEx(&statusex);
	std::unordered_map<string, string> ram;
	ram.emplace("all", std::to_string(statusex.ullTotalPhys / 1024 / 1024));
	ram.emplace("canuse", std::to_string(statusex.ullAvailPhys / 1024 / 1024));
	ram.emplace("percent", std::to_string(statusex.dwMemoryLoad));
	ram.emplace("used", std::to_string((statusex.ullTotalPhys - statusex.ullAvailPhys) / 1024 / 1024));
	return ram;
}

void getCpuUsePercentage1(int& nMemTotal){
	PDH_HQUERY query;
	PDH_HCOUNTER counter;
	PdhOpenQuery(NULL, NULL, &query);
	PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &counter);
	PdhCollectQueryData(query);
	Sleep(1000);
	PdhCollectQueryData(query);
	PDH_FMT_COUNTERVALUE pdhValue;
	DWORD dwValue;
	PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, &dwValue, &pdhValue);
	nMemTotal = pdhValue.doubleValue;
	qDebug() << "CPU占用:" << nMemTotal;
	PdhCloseQuery(query);
}

string getCPUUsed() {
	int ncpupercent = 0;
	getCpuUsePercentage1(ncpupercent);
	/*MySysInfo msys;
	msys.GetCpuUsage(ncpupercent);*/
	return std::to_string(ncpupercent);
}


string FmtConsoleRegular(string cmd) {
	/*
	//系统
	{cpu} CPU占用率
	{ramAll} 总物理内存
	{ramUse} 已使用的物理内存
	{ramCan} 可使用的物理内存
	{ramPercent} 物理内存使用率

	//时间
	{time} 当前时间
	{year} 当前年份
	{month} 当前月份
	{week} 当前星期(数字)
	{day} 当前天数
	{hour} 当前小时(24小时制)
	{min} 当前分钟
	{second} 当前秒数
	*/

	std::unordered_map<string, string> ram = getRam();
	//系统
	string cpu_ = m_replace(cmd, "{cpu}", getCPUUsed());
	string ramAll_ = m_replace(cpu_, "{ramAll}", ram["all"]);
	string ramUse_ = m_replace(ramAll_, "{ramUse}", ram["used"]);
	string ramPercent_ = m_replace(ramUse_, "{ranPercent}", ram["percent"]);
	string ramCan_ = m_replace(ramPercent_, "{ramCan}", ram["canuse"]);

	//日期
	time_t tt = time(NULL);
	struct tm* t = localtime(&tt);
	char time[1000];
	string str = fmt::format("{}-{}-{} {}:{}:{}", 
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec);
	string time_ = m_replace(ramCan_, "{time}", str);
	string year_ = m_replace(time_, "{year}", std::to_string(t->tm_year + 1900));
	string month_ = m_replace(year_, "{month}", std::to_string(t->tm_mon + 1));
	string week_ = m_replace(month_, "{week}", std::to_string(t->tm_wday + 1));
	string day_ = m_replace(week_, "{day}", std::to_string(t->tm_mday));
	string hour_ = m_replace(day_, "{hour}", std::to_string(t->tm_hour));
	string min_ = m_replace(hour_, "{min}", std::to_string(t->tm_min));
	string second_ = m_replace(min_, "{second}", std::to_string(t->tm_sec));


	return second_;
}


string FmtGroupRegular(
	string qqid,
	string qqnick,
	string cmd
) {
	/*
	//QQ
	{qqid} QQ号
	{qqnick} QQ昵称
	{xboxid} XBOXID(未绑定则不改变)

	//系统
	{cpu} CPU占用率
	{ramAll} 总物理内存
	{ramUse} 已使用的物理内存
	{ramCan} 可使用的物理内存
	{ramPercent} 物理内存使用率

	//时间
	{time} 当前时间
	{year} 当前年份
	{month} 当前月份
	{week} 当前星期(数字)
	{day} 当前天数
	{hour} 当前小时(24小时制)
	{min} 当前分钟
	{second} 当前秒数
	*/
	//QQ
	string qqid_ = m_replace(cmd, "{qqid}", qqid);
	string qqnick_ = m_replace(qqid_, "{qqnick}", qqnick);
	string xboxid_ = qqnick_;
	YAML::Node player = YAML::LoadFile("data/player.yml");
	for (YAML::Node i : player) {
		if (i["qq"].as<string>() == qqid) {
			xboxid_ = m_replace(qqnick_, "{xboxid}", i["playerName"].as<string>());
		}
	}
	
	std::unordered_map<string, string> ram = getRam();
	//系统
	string cpu_ = m_replace(xboxid_, "{cpu}", getCPUUsed());
	string ramAll_ = m_replace(cpu_, "{ramAll}", ram["all"]);
	string ramUse_ = m_replace(ramAll_, "{ramUse}", ram["used"]);
	string ramPercent_ = m_replace(ramUse_, "{ranPercent}", ram["percent"]);
	string ramCan_ = m_replace(ramPercent_, "{ramCan}", ram["canuse"]);

	//日期
	time_t tt = time(NULL);
	struct tm* t = localtime(&tt);
	char time[1000];
	string str = fmt::format("{}-{}-{} {}:{}:{}",
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec);
	string time_ = m_replace(ramCan_, "{time}", str);
	string year_ = m_replace(time_, "{year}", std::to_string(t->tm_year + 1900));
	string month_ = m_replace(year_, "{month}", std::to_string(t->tm_mon + 1));
	string week_ = m_replace(month_, "{week}", std::to_string(t->tm_wday + 1));
	string day_ = m_replace(week_, "{day}", std::to_string(t->tm_mday));
	string hour_ = m_replace(day_, "{hour}", std::to_string(t->tm_hour));
	string min_ = m_replace(hour_, "{min}", std::to_string(t->tm_min));
	string second_ = m_replace(min_, "{second}", std::to_string(t->tm_sec));

	return second_;
}