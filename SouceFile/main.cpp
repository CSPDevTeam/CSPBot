#include "cspbot.h"
#include "stdafx.h"
#include "global.h"
#include "server.h"
#include "mirai.h"
#include "logger.h"
#include <QtWidgets/QApplication>
#include <yaml-cpp/yaml.h>


#pragma warning(push)
#pragma warning(disable: 4996)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#pragma warning(disable: 4005)

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

using namespace std;

CSPBot* win;
Server* server = new Server();
std::string Version = "v0.0.3";

string getConfig(string key) {
    try {
        YAML::Node config = YAML::LoadFile("config/config.yml");
        return config[key].as<string>();
    }
    catch (...) {
        return "";
    }

}

bool connectMirai();
vector<string> split(const string& str, const string& pattern);
string motdbe(string host);
string motdje(string host);
string FmtConsoleRegular(string cmd);
void InitPython();

Logger logger("main");


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");//GB2312也可以
    QTextCodec::setCodecForLocale(codec);//2

    CSPBot* w = new CSPBot;
    win = w;
    //连接ws
    w->show();

    //初始化py插件
    InitPython();

    //连接Mirai
    connectMirai();
    
    return a.exec();
}
