#include "CPython.h"
#include "logger.h"
#include "global.h"
#include "server.h"
#include "mirai.h"

using namespace std;

QString stdString2QString(std::string str);

struct PyLogger {
	Logger thiz;

	PyLogger(const string& title) :thiz(title) {}
	void info(const string& msg) { thiz.info(msg); }
	void debug(const string& msg) { thiz.debug(msg); }
	void fatal(const string& msg) { thiz.error(msg); }
	void error(const string& msg) { thiz.error(msg); }
};

//API
string getVersion() {
	return Version;
}

bool runcmd(const string& cmd) {
	return server->sendCmd(cmd);
}

//Mirai API
void sendGroup(const string& group, const string& msg) {
	mi->sendGroupMsg(group,msg);
}

void sendAllGroup(const string& msg) {
	mi->sendAllGroupMsg(msg);
}

void recallMsg(const string& target) {
	mi->recallMsg(target);
}

void sendApp(const string & group, const string & code){
	mi->send_app(group, code);
}

void sendPacket(const string& packet) {
	mi->SendPacket(packet);
}

//设置监听器
void EnableListener(EventCode evc);
bool setListener(const string& eventName, const py::function& func) {
	auto event_code = magic_enum::enum_cast<EventCode>(eventName);
	if (!event_code)
		throw py::value_error("Invalid event name " + eventName);
		
	//添加回调函数
	g_cb_functions[event_code.value()].push_back(func);
	return true;
}

string motdbe(string host);
string motdje(string host);

string pymotdje(const string& host) {
	QRegExp r("(\\w.+):(\\w+)");
	int r_pos = r.indexIn(stdString2QString(host));
	if (r_pos > -1) {
		return motdje(host);
	}
	else {
		return "{}";
	}
}
string pymotdbe(const string& host) {
	QRegExp r("(\\w.+):(\\w+)");
	int r_pos = r.indexIn(stdString2QString(host));
	if (r_pos > -1) {
		return motdbe(host);
	}
	else {
		return "{}";
	}
}

PYBIND11_EMBEDDED_MODULE(bot, m) {
	using py::literals::operator""_a;
#pragma region Logger
	py::class_<PyLogger>(m, "Logger")
		.def(py::init<string>())
		.def("info", &PyLogger::info)
		.def("debug", &PyLogger::debug)
		.def("fatal", &PyLogger::fatal)
		.def("error", &PyLogger::error)
		;
#pragma endregion
#pragma region Functions
	m
		.def("getVersion", &getVersion)
		.def("runCommand", &runcmd)
		.def("sendGroupMsg", &sendGroup)
		.def("sendAllGroupMsg", &sendAllGroup)
		.def("recallMsg", &recallMsg)
		.def("sendApp", &sendApp)
		.def("sendPacket", &sendPacket)
		.def("setListener", &setListener)
		.def("motdbe", &pymotdbe)
		.def("motdje", &pymotdje);
		;
#pragma endregion
}
