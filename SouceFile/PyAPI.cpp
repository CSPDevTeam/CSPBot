#include "CPython.h"
#include "logger.h"
#include "global.h"
#include "server.h"
#include "mirai.h"
#include "qmessagebox.h"

using namespace std;

//API
QString stdString2QString(std::string str);
std::string QString2stdString(QString str);

//Buttons
//Button
enum SelfStandardButton {
	NoButton=0,
	Ok=1024,
	Save,
	SaveAll,
	Open,
	Yes,
	YesToAll,
	No,
	NoToAll,
	Abort,
	Retry,
	Ignore,
	Close,
	Cancel,
	Discard,
	Help,
	Apply,
	Reset,
	RestoreDefaults,
};


struct PyLogger {
	Logger thiz;

	PyLogger(const string& title) :thiz("Plugin ["+title+"]") {}
	void info(const string& msg) { thiz.info(msg); }
	void debug(const string& msg) { thiz.debug(msg); }
	void warn(const string& msg) { thiz.warn(msg); }
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

string QButtonToString(QMessageBox::StandardButton c)
{
	if(c == QMessageBox::NoButton){
		return "NoButton";
	}
	else if(c == QMessageBox::Ok){
		return "Ok";
	}
	else if (c == QMessageBox::Save) {
		return "Save";
	}
	else if (c == QMessageBox::SaveAll) {
		return "SaveAll";
	}
	else if (c == QMessageBox::Open) {
		return "Open";
	}
	else if (c == QMessageBox::Yes) {
		return "Yes";
	}
	else if (c == QMessageBox::YesToAll) {
		return "YesToAll";
	}
	else if (c == QMessageBox::No) {
		return "No";
	}
	else if (c == QMessageBox::NoToAll) {
		return "NoToAll";
	}
	else if (c == QMessageBox::Abort) {
		return "Abort";
	}
	else if (c == QMessageBox::Retry) {
		return "Retry";
	}
	else if (c == QMessageBox::Ignore) {
		return "Ignore";
	}
	else if (c == QMessageBox::Close) {
		return "Close";
	}
	else if (c == QMessageBox::Cancel) {
		return "Cancel";
	}
	else if (c == QMessageBox::Discard) {
		return "Discard";
	}
	else if (c == QMessageBox::Help) {
		return "Help";
	}
	else if (c == QMessageBox::Apply) {
		return "Apply";
	}
	else if (c == QMessageBox::Reset) {
		return "Reset";
	}
	else if (c == QMessageBox::RestoreDefaults) {
		return "RestoreDefaults";
	}
	else{
		return "";
	}
}

QMessageBox::StandardButton StringToQButton(string c)
{
	if(c == "NoButton"){
		return QMessageBox::NoButton;
	}
	else if (c == "Ok") {
		return QMessageBox::Ok;
	}
	else if (c == "Save") {
		return QMessageBox::Save;
	}
	else if (c == "SaveAll") {
		return QMessageBox::SaveAll;
	}
	else if (c == "Open") {
		return QMessageBox::Open;
	}
	else if (c == "Yes") {
		return QMessageBox::Yes;
	}
	else if (c == "YesToAll") {
		return QMessageBox::YesToAll;
	}
	else if (c == "No") {
		return QMessageBox::No;
	}
	else if (c == "NoToAll") {
		return QMessageBox::NoToAll;
	}
	else if (c == "Abort") {
		return QMessageBox::Abort;
	}
	else if (c == "Retry") {
		return QMessageBox::Retry;
	}
	else if (c == "Ignore") {
		return QMessageBox::Ignore;
	}
	else if (c == "Close") {
		return QMessageBox::Close;
	}
	else if (c == "Cancel") {
		return QMessageBox::Cancel;
	}
	else if (c == "Discard") {
		return QMessageBox::Discard;
	}
	else if (c == "Help") {
		return QMessageBox::Help;
	}
	else if (c == "Apply") {
		return QMessageBox::Apply;
	}
	else if (c == "Reset") {
		return QMessageBox::Reset;
	}
	else if (c == "RestoreDefaults") {
		return QMessageBox::RestoreDefaults;
	}
	else {
		return QMessageBox::FlagMask;
	}
}

//构造弹窗
string ShowTipWindow(
	const string& type, //Type
	const string& title,
	const string& content,
	py::list buttonType

) {
	

	QFlags<QMessageBox::StandardButton> btn;
	for (auto i : buttonType) {
		string Btype = py::str(i);
		auto event_code = StringToQButton(Btype);

		if (event_code == QMessageBox::FlagMask) {
			throw py::value_error("Invalid StandardButton name " + Btype);
		}

		btn = btn | event_code;
	}
	//展示窗口
	QMessageBox::StandardButton Choosedbtn;
	if (type == "information") {
		Choosedbtn = QMessageBox::information(
			win,
			stdString2QString(title),
			stdString2QString(content),
			btn
		);
	}
	//询问
	else if (type == "question") {
		Choosedbtn = QMessageBox::question(
			win,
			stdString2QString(title),
			stdString2QString(content),
			btn
		);
	}
	//警告
	else if (type == "warning") {
		Choosedbtn = QMessageBox::warning(
			win,
			stdString2QString(title),
			stdString2QString(content),
			btn
		);
	}
	//错误
	else if (type == "critical") {
		Choosedbtn = QMessageBox::critical(
			win,
			stdString2QString(title),
			stdString2QString(content),
			btn
		);
	}
	//未知
	else {
		throw py::value_error("Invalid TipWindowType name " + type);
		return "";
	}

	//返回值
	qDebug() << stdString2QString(QButtonToString(Choosedbtn)) << (Choosedbtn == QMessageBox::Yes);
	return QButtonToString(Choosedbtn);
}

py::list getAllAPIList() {
	py::list apilist;
	apilist.append("Logger");
	apilist.append("getVersion");
	apilist.append("runCommand");
	apilist.append("sendGroupMsg");
	apilist.append("sendAllGroupMsg");
	apilist.append("recallMsg");
	apilist.append("sendApp");
	apilist.append("sendPacket");
	apilist.append("setListener");
	apilist.append("motdbe");
	apilist.append("motdje");
	apilist.append("tip");
	apilist.append("getAllAPIList");
	return apilist;
}

PYBIND11_EMBEDDED_MODULE(bot, m) {
	using py::literals::operator""_a;
#pragma region Logger
	py::class_<PyLogger>(m, "Logger")
		.def(py::init<string>())
		.def("info", &PyLogger::info)
		.def("debug", &PyLogger::debug)
		.def("error", &PyLogger::error)
		.def("warn", &PyLogger::warn)
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
		.def("motdje", &pymotdje)
		.def("tip", &ShowTipWindow)
		.def("getAllAPIList",&getAllAPIList);
		;
#pragma endregion
}
