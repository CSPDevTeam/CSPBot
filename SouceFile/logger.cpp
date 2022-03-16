#include "logger.h"
#include <Nlohmann/json.hpp>

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
