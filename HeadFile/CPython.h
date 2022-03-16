#pragma once

//引入头文件
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <string>
#include "logger.h"

namespace py = pybind11;
inline Logger pluginLogger("PluginModle");

/*struct BotVersion {
	int major;
	int minor;
	int revision;
};*/

struct Plugin {
	std::string name;
	std::string info;
	py::module m;
};

class PluginManager {
public:
	static bool registerPlugin(py::module handle,std::string name,std::string info);
	static Plugin* getPlugin(std::string name);
	static Plugin* getPlugin(py::module handler);
	static bool hasPlugin(std::string name);
	static bool LoadPlugin();
private:

};

enum EventCode
{
	onServerStart, //OK
	onServerStop, //OK 
	onSendCommand, //OK intercept
	onReceiveMsg, //OK intercept
	onReceivePacket, //OK
	onStop, //OK
	onLogin, //OK
	onImport, //OK
	onSendMsg, //OK intercept
	onRecall, //OK intercept
	onConnectError, //OK
	onConnectLost, //OK
};

inline std::unordered_map<EventCode, vector<py::function>> g_cb_functions;
inline std::unordered_map<std::string, Plugin> plugins;
inline std::unordered_map<EventCode, bool> enableEvent;

//事件回调，初始化对象将申请GIL
class Callbacker {
public:
	Callbacker(EventCode t) :type_(t), arg_() {}
	~Callbacker() {
		pluginLogger.debug("{}", arg_.ref_count());
	}

	//事件回调
	inline bool callback() {
		bool pass = true;
		arg_.inc_ref();//TODO: 为什么加1
		for (auto cb : g_cb_functions[type_]) {
			try {
				if (arg_.size() > 0) {
					pass = cb(arg_) != py::bool_(false);
				}
				else {
					pass = cb() != py::bool_(false);
				}
				
			}
			catch (const std::exception& e) {
				pluginLogger.error(e.what());
			}
		}
		return pass;
	}

	inline Callbacker& insert(const char* key, const py::object& item) {
		arg_[key] = item;
		return *this;
	}

private:
	EventCode type_;
	py::dict arg_;
	py::gil_scoped_acquire gil_;
};

