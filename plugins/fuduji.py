import bot
import threading
logger = bot.Logger("复读机")


def repeat(data):
    msg = data["msg"]
    group = data["group"]
    changemsg = ''
    if msg[0] == "我":
        changemsg = '我也'+msg[1:]
        logger.debug("复读机处理消息: "+changemsg)
        bot.sendGroupMsg(group,changemsg)

def Register():
    return {
        "name": "复读机",
        "info": "复读你的消息",
        "Author":"HuoHua",
        "Version":"0.0.1"
    }

def Init():
    logger.info("复读机初始化...")
    bot.setListener("onReceiveMsg", repeat)


Init()