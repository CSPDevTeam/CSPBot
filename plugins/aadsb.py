import bot
import json
import threading
import time
import sys

logger = bot.Logger("hello")

def cbe(data):
    logger.info(data)

bot.setListener("onSendMsg",cbe)

def onInitPos():
    logger.info("初始化...")

onInitPos()

eval()