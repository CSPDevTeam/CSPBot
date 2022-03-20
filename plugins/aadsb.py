import bot
import json
import threading
import time
import sys

logger = bot.Logger("hello")

def cbe(data):
    logger.info("收到消息: "+data["msg"])

bot.setListener("onSendMsg",cbe)

def cmdcb(data):
    logger.info("收到命令: "+str(data))

bot.registerCommand("test",cmdcb)

def onInitPos():
    logger.info("初始化...")

onInitPos()