#include "logger.h"
#include <Nlohmann/json.hpp>
#include <iostream>
#include <windows.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include <time.h>
#include <yaml-cpp/yaml.h>
#include <string>
#include <sstream>
#include <regex>
#include <cctype>
#pragma comment(lib,"pdh.lib")

using json = nlohmann::json;

string m_replace(string strSrc, const string& oldStr, const string& newStr, int count = -1);
string getConfig(string key);
QString stdString2QString(std::string str);
std::string QString2stdString(QString str);
vector<string> split(const string& str, const string& pattern);

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
	qDebug() << u8"CPU占用:" << nMemTotal;
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

std::unordered_map<QString, QString> G_colorParts = {
	{"\033[0m","#fff"},
	{"\033[1m","#fff"},
	{"\033[2m","#fff"},
	{"\033[30m","#fff"},
	{"\033[31m","#ff0000"},
	{"\033[32m","#00ff00"},
	{"\033[33m","#ffff00"},
	{"\033[34m","#0000ff"},
	{"\033[35m","#ff00ff"},
	{"\033[36m","#00ffff"},
	{"\033[37m","#fff"},
	{"\033[40m","#fff"},
	{"\033[41m","#fff"},
	{"\033[42m","#fff"},
	{"\033[43m","#fff"},
	{"\033[44m","#fff"},
	{"\033[45m","#fff"},
	{"\033[46m","#fff"},
	{"\033[47m","#fff"},
	{"\033[91m","#e74856"}
};

std::unordered_map<QString, QString> fromatColor = {
	{"§0","#fff"},
	{"§1","#fff"},
	{"§2","#fff"},
	{"§3","#fff"},
	{"§4","#ff0000"},
	{"§5","#00ff00"},
	{"§6","#ffff00"},
	{"§7","#0000ff"},
	{"§8","#ff00ff"},
	{"§9","#00ffff"},
	{"§a","#fff"},
	{"§b","#fff"},
	{"§c","#fff"},
	{"§d","#fff"},
	{"§e","#fff"},
	{"§f","#fff"},
	{"§g","#fff"},
};

std::unordered_map<QString, QString> Xtermcolor = {
	{"0","#000000"},{"1","#800000"},{"2","#008000"},{"3","#808000"},{"4","#000080"},
	{"5","#800080"},{"6","#008080"},{"7","#c0c0c0"},{"8","#808080"},{"9","#ff0000"},
	{"10","#00ff00"},{"11","#ffff00"},{"12","#0000ff"},{"13","#ff00ff"},{"14","#00ffff"},
	{"15","#ffffff"},{"16","#000000"},{"17","#00005f"},{"18","#000087"},{"19","#0000af"},
	{"20","#0000d7"},{"21","#0000ff"},{"22","#005f00"},{"23","#005f5f"},{"24","#005f87"},
	{"25","#005faf"},{"26","#005fd7"},{"27","#005fff"},{"28","#008700"},{"29","#00875f"},
	{"30","#008787"},{"31","#0087af"},{"32","#0087d7"},{"33","#0087ff"},{"34","#00af00"},
	{"35","#00af5f"},{"36","#00af87"},{"37","#00afaf"},{"38","#00afd7"},{"39","#00afff"},
	{"40","#00d700"},{"41","#00d75f"},{"42","#00d787"},{"43","#00d7af"},{"44","#00d7d7"},
	{"45","#00d7ff"},{"46","#00ff00"},{"47","#00ff5f"},{"48","#00ff87"},{"49","#00ffaf"},
	{"50","#00ffd7"},{"51","#00ffff"},{"52","#5f0000"},{"53","#5f005f"},{"54","#5f0087"},
	{"55","#5f00af"},{"56","#5f00d7"},{"57","#5f00ff"},{"58","#5f5f00"},{"59","#5f5f5f"},
	{"60","#5f5f87"},{"61","#5f5faf"},{"62","#5f5fd7"},{"63","#5f5fff"},{"64","#5f8700"},
	{"65","#5f875f"},{"66","#5f8787"},{"67","#5f87af"},{"68","#5f87d7"},{"69","#5f87ff"},
	{"70","#5faf00"},{"71","#5faf5f"},{"72","#5faf87"},{"73","#5fafaf"},{"74","#5fafd7"},
	{"75","#5fafff"},{"76","#5fd700"},{"77","#5fd75f"},{"78","#5fd787"},{"79","#5fd7af"},
	{"80","#5fd7d7"},{"81","#5fd7ff"},{"82","#5fff00"},{"83","#5fff5f"},{"84","#5fff87"},
	{"85","#5fffaf"},{"86","#5fffd7"},{"87","#5fffff"},{"88","#870000"},{"89","#87005f"},
	{"90","#870087"},{"91","#8700af"},{"92","#8700d7"},{"93","#8700ff"},{"94","#875f00"},
	{"95","#875f5f"},{"96","#875f87"},{"97","#875faf"},{"98","#875fd7"},{"99","#875fff"},
	{"100","#878700"},{"101","#87875f"},{"102","#878787"},{"103","#8787af"},{"104","#8787d7"},
	{"105","#8787ff"},{"106","#87af00"},{"107","#87af5f"},{"108","#87af87"},{"109","#87afaf"},
	{"110","#87afd7"},{"111","#87afff"},{"112","#87d700"},{"113","#87d75f"},{"114","#87d787"},
	{"115","#87d7af"},{"116","#87d7d7"},{"117","#87d7ff"},{"118","#87ff00"},{"119","#87ff5f"},
	{"120","#87ff87"},{"121","#87ffaf"},{"122","#87ffd7"},{"123","#87ffff"},{"124","#af0000"},
	{"125","#af005f"},{"126","#af0087"},{"127","#af00af"},{"128","#af00d7"},{"129","#af00ff"},
	{"130","#af5f00"},{"131","#af5f5f"},{"132","#af5f87"},{"133","#af5faf"},{"134","#af5fd7"},
	{"135","#af5fff"},{"136","#af8700"},{"137","#af875f"},{"138","#af8787"},{"139","#af87af"},
	{"140","#af87d7"},{"141","#af87ff"},{"142","#afaf00"},{"143","#afaf5f"},{"144","#afaf87"},
	{"145","#afafaf"},{"146","#afafd7"},{"147","#afafff"},{"148","#afd700"},{"149","#afd75f"},
	{"150","#afd787"},{"151","#afd7af"},{"152","#afd7d7"},{"153","#afd7ff"},{"154","#afff00"},
	{"155","#afff5f"},{"156","#afff87"},{"157","#afffaf"},{"158","#afffd7"},{"159","#afffff"},
	{"160","#d70000"},{"161","#d7005f"},{"162","#d70087"},{"163","#d700af"},{"164","#d700d7"},
	{"165","#d700ff"},{"166","#d75f00"},{"167","#d75f5f"},{"168","#d75f87"},{"169","#d75faf"},
	{"170","#d75fd7"},{"171","#d75fff"},{"172","#d78700"},{"173","#d7875f"},{"174","#d78787"},
	{"175","#d787af"},{"176","#d787d7"},{"177","#d787ff"},{"178","#d7af00"},{"179","#d7af5f"},
	{"180","#d7af87"},{"181","#d7afaf"},{"182","#d7afd7"},{"183","#d7afff"},{"184","#d7d700"},
	{"185","#d7d75f"},{"186","#d7d787"},{"187","#d7d7af"},{"188","#d7d7d7"},{"189","#d7d7ff"},
	{"190","#d7ff00"},{"191","#d7ff5f"},{"192","#d7ff87"},{"193","#d7ffaf"},{"194","#d7ffd7"},
	{"195","#d7ffff"},{"196","#ff0000"},{"197","#ff005f"},{"198","#ff0087"},{"199","#ff00af"},
	{"200","#ff00d7"},{"201","#ff00ff"},{"202","#ff5f00"},{"203","#ff5f5f"},{"204","#ff5f87"},
	{"205","#ff5faf"},{"206","#ff5fd7"},{"207","#ff5fff"},{"208","#ff8700"},{"209","#ff875f"},
	{"210","#ff8787"},{"211","#ff87af"},{"212","#ff87d7"},{"213","#ff87ff"},{"214","#ffaf00"},
	{"215","#ffaf5f"},{"216","#ffaf87"},{"217","#ffafaf"},{"218","#ffafd7"},{"219","#ffafff"},
	{"220","#ffd700"},{"221","#ffd75f"},{"222","#ffd787"},{"223","#ffd7af"},{"224","#ffd7d7"},
	{"225","#ffd7ff"},{"226","#ffff00"},{"227","#ffff5f"},{"228","#ffff87"},{"229","#ffffaf"},
	{"230","#ffffd7"},{"231","#ffffff"},{"232","#080808"},{"233","#121212"},{"234","#1c1c1c"},
	{"235","#262626"},{"236","#303030"},{"237","#3a3a3a"},{"238","#444444"},{"239","#4e4e4e"},
	{"240","#585858"},{"241","#626262"},{"242","#6c6c6c"},{"243","#767676"},{"244","#808080"},
	{"245","#8a8a8a"},{"246","#949494"},{"247","#9e9e9e"},{"248","#a8a8a8"},{"249","#b2b2b2"},
	{"250","#bcbcbc"},{"251","#c6c6c6"},{"252","#d0d0d0"},{"253","#dadada"},{"254","#e4e4e4"},
	{"255","#eeeeee"}
};


class RGBToHex
{
public:
	static std::string rgb(string color="0,0,0") {
		stringstream ss;
		int r=0,g=0,b=0;
		vector<string> splitColor = split(color, ",");
		r = std::stoi(splitColor[0]);
		g = std::stoi(splitColor[1]);
		b = std::stoi(splitColor[2]);
		char c[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;
		ss << c[(int)r / 16] << c[r % 16] << c[(int)g / 16] << c[g % 16] << c[(int)b / 16] << c[b % 16];
		return "#"+ss.str();
	}
};

QString getColoredLine(string line) {
	QString qline = stdString2QString(line);
	try {
		if (YAML::LoadFile("config/config.yml")["ConsoleColor"].as<bool>()) {
			qline = qline.replace("<", "&lt;");
			qline = qline.replace(">", "&gt;");
			qline = qline.replace("\033[0m", "</font>");
			qline = qline.replace("\033[m", "</font>");
			QRegExp color("\\033\\[38;2[\\d;]+m");
			while (color.indexIn(qline) > -1)
			{
				QString rgb = "rgb(";
				QString rgbcolor = color.cap(0);
				//提取color
				rgbcolor = rgbcolor.replace("\033[38;2;", "").replace("m", "").replace(";", ",");
				QString hexColor = stdString2QString(RGBToHex::rgb(QString2stdString(rgbcolor)));
				qline = qline.replace(color.cap(0), "<font color=\"" + hexColor + "\" style=\"white-space: pre-wrap\">");
			}

			int i = 0;
			QRegExp normal_color("\\033\\[[\\d;]+m");
			while (normal_color.indexIn(qline) > -1)
			{
				QString rgbcolor = normal_color.cap(0);
				if (G_colorParts.find(rgbcolor) != G_colorParts.end()) {
					QString HexColor = G_colorParts[rgbcolor];
					qline.replace(rgbcolor, "<font color=\"" + HexColor + "\" style=\"white-space: pre-wrap\">");
				}
				else {
					qline.replace(rgbcolor, "<font color=\"#fff\" style=\"white-space: pre-wrap\">");
				}
			}

			QRegExp xterm_color("\\033\\[38;5;[\\d]+m");
			while (xterm_color.indexIn(qline) > -1)
			{
				QString rgbcolor = xterm_color.cap(0);
				QString Xterm256Code = rgbcolor.replace("\033[38;5;", "").replace("m", "");
				if (Xtermcolor.find(Xterm256Code) != Xtermcolor.end()) {
					QString HexColor = Xtermcolor[Xterm256Code];
					qline.replace(rgbcolor, "<font color=\"" + HexColor + "\" style=\"white-space: pre-wrap\">");
				}
				else {
					qline.replace(rgbcolor, "<font color=\"#fff\" style=\"white-space: pre-wrap\">");
				} 
			}

			//删除多余的</font>
			string lline = "</font>";
			auto start = qline.mid(0, lline.length());
			if (start == "</font>") {
				qline = qline.remove(0, lline.length());
			}

			QRegExp font("<font(.+)>");
			if (font.indexIn(qline) == -1 && qline !="") {
				qline.replace("</font>", "");
				qline = "<font style=\"white-space: pre-wrap\">" + qline + "</font>";
			}
		}
		else {
			regex pattern("\033\\[(.+?)m");
			qline = stdString2QString(regex_replace(line, pattern, ""));
		}
	}
	catch (...) {
		regex pattern("\033\\[(.+?)m");
		qline = stdString2QString(regex_replace(line, pattern, ""));
	}
	return qline;
}