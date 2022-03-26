#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include "cspbot.h"
#include "global.h"
#include <filesystem>
#include <FMT/chrono.h>
#include <FMT/color.h>
#include <FMT/core.h>
#include <FMT/os.h>
#include <FMT/printf.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <utility>
#include <magic_enum.hpp>

using namespace std;

string getConfig(string key);
class Logger {
public:
	inline Logger(string moduleName) {
		Module = moduleName;
	}

	template <typename... Args>
	inline void info(string msg, const Args&... args) {
		try {
			string str = fmt::format(msg, args...);
			win->inlog("< font color = \"#008000\">" + getTime() + " I /" + Module + ": " + str + "\n</font>");
		}
		catch (...) {
			win->inlog("< font color = \"#008000\">" + getTime() + " I /" + Module + ": " + msg + "\n</font>");
		}
		
	};
	template <typename... Args>
	inline void error(string msg, const Args&... args) {
		try {
			string str = fmt::format(msg, args...);
			win->inlog("< font color = \"#FF0000\">" + getTime() + " E /" + Module + ": " + str + "\n</font>");
		}
		catch (...) {
			win->inlog("< font color = \"#FF0000\">" + getTime() + " E /" + Module + ": " + msg + "\n</font>");
		}
	};
	template <typename... Args>
	inline void warn(string msg, const Args&... args) {
		try {
			string str = fmt::format(msg, args...);
			win->inlog("< font color = \"#FFCC66\">" + getTime() + " W /" + Module + ": " + str + "\n</font>");
		}
		catch (...) {
			win->inlog("< font color = \"#FFCC66\">" + getTime() + " W /" + Module + ": " + msg + "\n</font>");
		}
		
	};
	template <typename... Args>
	inline void debug(string msg, const Args&... args) {
		try {
			string str = fmt::format(msg, args...);
			win->inlog("< font color = \"#6699FF\">" + getTime() + " D /" + Module + ": " + str + "\n</font>");
		}
		catch (...) {
			win->inlog("< font color = \"#6699FF\">" + getTime() + " D /" + Module + ": " + msg + "\n</font>");
		}
	};
private:
	inline string getTime() {
		time_t tt = time(NULL);
		struct tm* t = localtime(&tt);
		std::ostringstream buffer;
		string s = std::to_string(t->tm_sec);
		if (s.length() == 1) {
			s = "0" + s;
		}
		buffer << t->tm_year + 1900 << "-" << t->tm_mon + 1 << "-" << t->tm_mday << " " << t->tm_hour << ":" << t->tm_min << ":" << s;
		string time = buffer.str();
		return time;
	};

	inline int getLevel() {
		string level = getConfig("LoggerLevel");
		if (level == "info") {
			return 2;
		}
		else if (level == "warn") {
			return 3;
		}
		else if (level == "error") {
			return 4;
		}
		else {
			return 1;
		}
	};

	string Module="";
};
