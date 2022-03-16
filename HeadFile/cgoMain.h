#include <stdio.h>
typedef void(*MsgHandler)(__int64,char*);
MsgHandler CppMsgHandler;
void RunMsgHandler(__int64 uniqueid,char* cmd){
	CppMsgHandler(uniqueid,cmd);
}
void SetMsgHandler(void* i){
	CppMsgHandler =(MsgHandler) i;
}

typedef void(*Logger)(int,char*);
Logger CppLogger;
void Log(int loglvl,char* text){
	CppLogger(loglvl,text);
}
void SetLogger(void* i){
	CppLogger =(Logger) i;
}