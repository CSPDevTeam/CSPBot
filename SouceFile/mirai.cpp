//引入头文件
#include "mirai.h"
#include "global.h"
#include "CPython.h"
#include "logger.h"
#include <qmessagebox.h>
#include <qdebug.h>
#include <string>
#include "Command.h"

class Callbacker;
string getConfig(string key);
QString stdString2QString(std::string str);
string FmtGroupRegular(string qqid,string qqnick,string cmd);

//构建Client
Logger* mirai_logger = new Logger("Mirai");
std::string QString2stdString(QString str);
Mirai* mi = new Mirai();
string m_replace(string strSrc, const string& oldStr, const string& newStr, int count = -1);

void selfGroupCatchLine(QString line,string group="0",string qq="0",string qqnick="") {
	YAML::Node regular = YAML::LoadFile("data/regular.yml");
	YAML::Node config = YAML::LoadFile("config/config.yml");
	//读取正则组
	for (YAML::Node i : regular) {
		string Regular = i["Regular"].as<string>();
		string Action = i["Action"].as<string>();
		string From = i["From"].as<string>();
		bool Permissions = i["Permissions"].as<bool>();

		QRegExp r(stdString2QString(Regular));
		int r_pos = r.indexIn(line);

		bool qqAdmin = false;
		for (auto i : config["admin"]) {
			string AdminQQ = std::to_string(i.as<long long>());
			if (AdminQQ == qq) {
				qqAdmin = true;
				break;
			}
		}

		//执行操作
		if (r_pos > -1 && From == "group" && (Permissions==false || (Permissions==true&&qqAdmin))) {
			string Action_type = Action.substr(0, 2);
			int num = 0;
			for (auto replace : r.capturedTexts()) {
				Action = m_replace(Action, "$" + std::to_string(num), QString2stdString(replace));
				num++;
			}
			if (Action_type == "<<") {
				string cmd = Action.erase(0, 2);
				cmd = FmtGroupRegular(qq, qqnick, cmd);
				server->sendCmd(cmd + "\n");
			}
			else if (Action_type == ">>") {
				string cmd = Action.erase(0, 2);
				cmd = FmtGroupRegular(qq, qqnick, cmd);
				mi->sendAllGroupMsg(cmd);
			}
			else {
				string ac = FmtGroupRegular(qq, qqnick, Action);
				Command::CustomCmd(ac, group);
			}
		}
	}
}

//事件
void MiraiEvent(json msg_json){

}

//Msg
void onText(WebSocketClient& client, string msg) {
	json msg_json = json::parse(msg);
	string syncId = msg_json["syncId"].get<string>();
	mirai_logger->debug(msg_json.dump());
	win->PacketCallback(msg);
	//登录包
	if (syncId == "1") {
		mirai_logger->info(u8"{}登录 Mirai 成功", msg_json["data"]["nickname"].get<string>());
		mi->logined = true;
		win->OtherCallback("onLogin");
	}
	//发消息包
	else if (syncId == "2") {
		int msgId = msg_json["data"]["messageId"];
		if (msgId == -1) {
			mirai_logger->warn(u8"已发出信息但可能遭到屏蔽");
		}
	}
	else if (syncId == "-1") {
		//消息处理
		if (msg_json["data"].find("type")!=msg_json["data"].end() && msg_json["data"]["type"] == "GroupMessage") {
			string qq = std::to_string(msg_json["data"]["sender"]["id"].get<long long>());
			string qqnick = msg_json["data"]["sender"]["memberName"].get<string>();
			string group = std::to_string(msg_json["data"]["sender"]["group"]["id"].get<long long>());
			string msg = "";

			vector<string> allowGroup;
			YAML::Node node = YAML::LoadFile("config/config.yml");
			for (auto i:node["group"]) {
				allowGroup.push_back(i.as<string>());
			}

			if (std::find(allowGroup.begin(),allowGroup.end(),group) != allowGroup.end()) {
				for (auto i : msg_json["data"]["messageChain"]) {
					if (i["type"] == "Plain") {
						msg += u8" " + i["text"].get<string>();
					}
					else if (i["type"] == "Image") {
						msg += u8" [图片]";
					}
					else if (i["type"] == "Quote") {
						msg += u8" [回复的消息]";
					}
					else if (i["type"] == "At") {
						msg += u8" " + i["display"].get<string>();
					}
					else if (i["type"] == "AtAll") {
						msg += u8" [@全体成员]";
					}
					else if (i["type"] == "Face") {
						msg += u8" [QQ表情]";
					}
					else if (i["type"] == "FlashImage") {
						msg += u8" [闪照]";
					}
					else if (i["type"] == "Voice") {
						msg += u8" [语音]";
					}
					else if (i["type"] == "Xml") {
						msg += u8" [XML消息]";
					}
					else if (i["type"] == "Json") {
						msg += u8" [JSON消息]";
					}
					else if (i["type"] == "App") {
						msg += u8" [小程序]";
					}
					else if (i["type"] == "Poke") {
						msg += u8" [戳一戳]";
					}
					else if (i["type"] == "Dice") {
						msg += u8" [未知消息]";
					}
					else if (i["type"] == "MusicShare") {
						msg += u8" [音乐分享]" + i["musicUrl"];
					}
					else if (i["type"] == "ForwardMessage") {
						msg += u8" [转发的消息]";
					}
					else if (i["type"] == "File") {
						msg += u8" [文件]" + i["name"].get<string>();
					}
					else if (i["type"] == "Source") {}
					else {
						msg += u8" [未知消息]";
					}
				}
				msg = msg.erase(0, 1);

				//Callback
				std::unordered_map<string, string> args;
				args.emplace("group", group);
				args.emplace("msg", msg);
				args.emplace("qq", qq);
				args.emplace("qqnick", qqnick);
				win->OtherCallback("onReceiveMsg", args);
				selfGroupCatchLine(stdString2QString(msg), group, qq,qqnick);
				
			}
		}
		//事件处理(群成员改名)
		else if (msg_json["data"].find("type") != msg_json["data"].end() && \
			msg_json["data"]["type"] == "MemberCardChangeEvent") {

		}
	}
	
}

void onError(WebSocketClient& client, string msg) {
	mirai_logger->error(msg);
	win->OtherCallback("onConnectError");
}

void onLost(WebSocketClient& client, int code) {
	mirai_logger->error("ConnectLost");
	win->OtherCallback("onConnectLost");
}

//API
bool connectMirai() {
	bool connected = mi->login();
	if (connected) {
		mirai_logger->info(u8"连接到Mirai成功");
	}
	else {
		QMessageBox::critical(win, u8"Mirai错误", u8"无法连接到Mirai", QMessageBox::Ok);
		mirai_logger->error(u8"无法连接到 Mirai");
	}
	
	mi->botProfile();
	return connected;
}

WsClient::WsClient() {
	ws.OnTextReceived([]
	(WebSocketClient& client, string msg) {
			onText(client, msg);
		});

	ws.OnError([]
	(WebSocketClient& client, string msg) {
			onError(client, msg);
		});

	ws.OnLostConnection([]
	(WebSocketClient& client, int code) {
			onLost(client, code);
		});
}

int WsClient::getStatus() {
	return (int)ws.GetStatus();
}

bool WsClient::connect(string url) {
	try {
		ws.Connect(url);
		this->connected = true;
		return true;
	}
	catch (std::runtime_error e) {
		return false;
	}
}

bool WsClient::close() {
	if (this->connected) {
		try {
			ws.Close();
			return true;
		}
		catch (std::runtime_error e) {}
	}
	return false;
}

bool WsClient::send(string msg) {
	if (this->connected) {
		try {
			ws.SendText(msg);
			return true;
		}
		catch (std::runtime_error e) {}
	}
	return false;
}

bool WsClient::shutdown() {
	if (this->connected) {
		try {
			ws.Shutdown();
			return true;
		}
		catch (std::runtime_error e) {}
	}
	return false;
}

Mirai::Mirai() {
	WsClient* wsc = new WsClient();
	this->ws = wsc;
}

bool Mirai::login() {
	try {
		string qq = getConfig("qq");
		string key = getConfig("key");
		string wsUrl = getConfig("connectUrl");
		return this->ws->connect(wsUrl + "/all?verifyKey=" + key + "&qq=" + qq);
	}
	catch (const char e) {
		return false;
	}
}

void Mirai::botProfile() {
	string mj = "{\"syncId\": 1,\"command\" : \"botProfile\",\"subCommand\":null,\"content\":{}}";
	this->ws->send(mj);
}

void Mirai::sendGroupMsg(string group, string msg,bool callback) {
	if (this->logined) {
		string mj = "{\"syncId\": 2, \"command\":\"sendGroupMessage\", \"subCommand\" : null,\
					\"content\": {\"target\":" + group + ", \"messageChain\": [{ \"type\":\"Plain\", \"text\" : \"" + msg + "\"}]}}";
		std::unordered_map<string, string> args;
		args.emplace("group", group);
		args.emplace("msg", msg);
		win->OtherCallback("onSendMsg", args);
		this->ws->send(mj);
	}

}

void Mirai::recallMsg(string target, bool callback) {
	//recall
	if (this->logined) {
		string mj = "{\"syncId\": 3,\"command\" : \"recall\",\"subCommand\":null,\"content\":{\"target\":" + target + "}}";
		std::unordered_map<string, string> args;
		args.emplace("target", target);
		win->OtherCallback("onRecall", args);
		this->ws->send(mj);
	}
}

void Mirai::sendAllGroupMsg(string msg, bool callback) {
	YAML::Node config = YAML::LoadFile("config/config.yml")["group"];
	for (auto igroup : config) {
		string group = std::to_string(igroup.as<long long>());
		if (this->logined) {
			this->sendGroupMsg(group, msg,callback);
		}
	}

}

void Mirai::changeName(string qq,string group,string name) {
	string packet_Json = "{\"syncId\": 4,\"command\" : \"memberInfo\",\"subCommand\":\"update\",\"content\":{\"target\":" + group + ",\"memberId\":" + qq + ",\"info\" : {\"name\":" + name + "}}}";
	if (this->logined) {
		this->ws->send(packet_Json);
	}
}

void Mirai::send_app(string group, string code) {
	string packet_Json = "{\"syncId\": 5,\"command\" : \"sendGroupMessage\",\"subCommand\" : null,\"content\" : {\"target\":" + group + ",\"messageChain\" : [{\"type\": \"App\",\"content\" :" + code + "}]}}";
	if (this->logined) {
		this->ws->send(packet_Json);
	}
}

void Mirai::SendPacket(string packet) {
	this->ws->send(packet);
}