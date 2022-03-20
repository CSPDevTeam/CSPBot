import urllib.request
import bot
import json
import webbrowser

logger = bot.Logger("CheckUpdate")

def Check():
    url = 'https://api.github.com/repos/HuoHuas001/CSPBot/releases/latest'
    with urllib.request.urlopen(url) as response:
        j = json.loads(response.read().decode('utf-8'))
        version = j["tag_name"]
        botVersion = bot.getVersion()

        if version != botVersion:
            rt = bot.tip(
                "question",
                "检测到更新",
                "检测到更新,最新版本:"+version+",您的版本为:"+botVersion+"\n是否更新?",
                ["Yes","No"]
            )
            if(rt == "Yes"):
                webbrowser.open(j["html_url"])
            else:
                logger.warn("您选择了不更新，您可能无法使用最新功能")
        else:
            logger.info("您的CSPBot版本为最新版本,无需更新.")

def Register():
    return {
        "name": "CheckUpdate",
        "info": "检查CSPBot更新",
        "Author":"HuoHua",
        "Version":"0.0.1"
    }

def InitPlugin():
    logger.info("检测更新插件加载完成,作者:HuoHua")
    apiList = bot.getAllAPIList()
    if "tip" in apiList:
        Check()


InitPlugin()