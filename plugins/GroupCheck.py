import bot
import re

logger = bot.Logger("GroupCheck")

#进行反馈
def ListenBDS(data):
    line = data["line"]
    regular = r"\[GroupCheck\]\s\[API\sOut\](.+)"
    pattern = re.compile(regular)
    if(re.search(regular, line)):
        result1 = pattern.findall(line)
        bot.sendAllGroupMsg(result1[0])
    

#注册插件
def Register():
    return {
        "name": "GroupCheck群签到",
        "info": "让你的玩家在群里签到吧!",
        "Author":"HuoHua",
        "Version":"0.0.1"
    }

#指令回调函数
def cmdcb(data):
    if(len(data) == 1):
        qqre = r"[1-9][0-9]{4,}"
        if(re.search(qqre, data[0])):
            playerQQ = data[0]
            playerName = bot.queryData("qq",playerQQ)
            if(playerName["playerName"] != None):
                bot.runCommand("check check \""+playerName["playerName"]+"\"\n")
            else:
                bot.sendAllGroupMsg("找不到关于"+data[0]+"的信息.")
    else:
        bot.sendAllGroupMsg("指令格式错误.")

#加载主函数
def Init():
    bot.setListener("onConsoleUpdate",ListenBDS)
    bot.registerCommand("check",cmdcb)
    logger.info("GroupCheck 群签到插件加载成功.")

Init()