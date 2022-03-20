#include "CPython.h"
#include "logger.h"
#include <vector>
#include <set>
#include <unordered_map>

#define PLUGIN_PATH "plugins\\"
#define PYLIBRARY_PATH "PyLibrary\\"

using namespace std;
namespace fs = filesystem;

bool PluginManager::registerPlugin(
    py::module handler,
    std::string name, 
    std::string info,
    std::string autor,
    std::string version) {

    string SelfName = name, SelfInfo = info, SelfAutor = autor, SelfVersion = version;

    //检测插件是否已被注册
    if (handler != nullptr)             // DLL Plugin
    {
        if (getPlugin(handler) != nullptr)
        {
            for (auto& i : plugins) {
                return handler == i.second.m;
            }
        }
        else if (getPlugin(name) != nullptr) {
            return false;
        }
    }

    //从函数获取
    try {
        py::dict data = handler.attr("Register")();
        SelfName = py::str(data["name"]);
        SelfInfo = py::str(data["info"]);
        SelfAutor = py::str(data["Author"]);
        SelfVersion = py::str(data["Version"]);
    }
    catch (...) {

    }
     
   

    Plugin plugin{SelfName,SelfInfo,SelfAutor,SelfVersion};
    plugin.m = handler;
    plugins.emplace(name, plugin);
    return true;
}

bool PluginManager::hasPlugin(std::string name) {
    return getPlugin(name) != nullptr;
}

Plugin* PluginManager::getPlugin(std::string name) {
    Plugin* res = nullptr;
    for (auto& it : plugins) {
        if (it.second.name == name) {
            res = &it.second;
        }
    }
    return res;
}

Plugin* PluginManager::getPlugin(py::module handle) {
    if (!handle)
        return nullptr;
    for (auto& it : plugins) {
        if (it.second.m == handle) {
            return &it.second;
        }
    }
    return nullptr;
}

bool PluginManager::LoadPlugin() {
    //加载文件
    try {
        //添加path
        py::module sys = py::module::import("sys");
        sys.attr("path").attr("append")(PLUGIN_PATH);
        sys.attr("path").attr("append")(PYLIBRARY_PATH);


        for (auto& info : fs::directory_iterator(PLUGIN_PATH)) {
            string filename = info.path().filename().u8string();
            if (info.path().extension() == ".py"||
                info.path().extension() == ".pyc"||
                info.path().extension() == ".pyd") {
                string name(info.path().stem().u8string());

                //忽略以'_'开头的文件
                if (name.front() == '_') {
                    pluginLogger.warn("Ignoring {}", name);
                    continue;
                }
                else {
                    pluginLogger.info("Loading Plugin {}", name);
                    auto m = py::module_::import(name.c_str());
                    if (m)
                    {
                        if (getPlugin(m) == nullptr)
                            registerPlugin(m,name, name);
                    }
                    else
                    {
                        pluginLogger.error("Fail to load the plugin {}!", name);
                    }
                    Callbacker cbe(EventCode::onImport);
                    cbe.insert("module", m);
                    cbe.insert("name", py::str(name));
                    cbe.callback();
                }
            }
        }
    }
    catch (const std::exception& e) {
        pluginLogger.error(e.what());
    }
    return true;
}

void InitPython() {
    if (!fs::exists(PLUGIN_PATH))
        fs::create_directory(PLUGIN_PATH);
    //设置模块搜索路径
    wstring plugins_path = PLUGIN_PATH L";";
    plugins_path.append(Py_GetPath());
    Py_SetPath(plugins_path.c_str());

    //初始化解释器
    py::initialize_interpreter();

    //加载插件
    PluginManager::LoadPlugin();
}