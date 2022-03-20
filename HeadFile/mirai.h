#pragma once
#include <LightWSClient/WebSocketClient.h>
#include <Nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

using namespace cyanray;
using json = nlohmann::json;

class WsClient
{
private:
    WebSocketClient ws;    
	bool connected = false;
public:
    explicit WsClient();
    //API
    bool send(string msg);
    bool connect(string url);
    bool close();
    bool shutdown();
    int getStatus();
};
void onText(WebSocketClient& client, string msg);
void onError(WebSocketClient& client, string msg);
void onLost(WebSocketClient& client, int code);
string getConfig(string key);



class Mirai {
public:
	explicit Mirai();
	bool login();
	void sendGroupMsg(string group, string msg, bool callback = true);
	void sendAllGroupMsg(string msg, bool callback = true);
	void recallMsg(string target, bool callback=true);
	void botProfile();
	void changeName(string qq, string group, string name);
	void send_app(string group, string code);

	void SendPacket(string packet);

	bool logined = false;
private:
	WsClient* ws;
};

