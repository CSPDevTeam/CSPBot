class Logger:
    def info(msg:str):
        """输出INFO信息"""
        ...
    def debug(msg:str):
        """输出DEBUG调试信息"""
        ...
    def error(msg:str):
        """输出Error错误信息"""
        ...
    def warn(msg:str):
        """输出Warn警告信息"""
        ...

def getVersion() -> str:
    """获取Bot版本字符串"""
    ...

def runCommand(cmd:str) -> bool:
    """
    向服务端发送命令
    
    :param cmd: 命令
    """
    ...

def sendGroupMsg(group:str,msg:str):
    """
    向指定的群发送消息
    
    :param group: 群聊
    :param msg: 消息
    """
    ...

def sendAllGroupMsg(msg:str):
    """
    向所有群发送消息
    
    :param msg: 消息
    """
    ...

def recallMsg(target:str):
    """
    撤回消息
    
    :param target: 消息Target
    """
    ...

def sendApp(group:str,code:str):
    """
    向指定的群发送APP消息
    
    :param group: 群聊
    :param code: APP Code
    """
    ...

def sendPacket(packet:str):
    """
    向Mirai发送自定义包
    
    :param cmd: 命令
    """
    ...

def setListener(event: str, function: Callable[[Any], Optional[bool]]):
    """
    设置监听器

    :param event: 监听器名称
    :param function: 回调函数
    """
    ...

def motdbe(ip_port:str) -> dict:
    """
    获取BE的Motd信息
    
    :param ip_port: 地址及端口 例:127.0.0.1:19132
    """
    ...

def motdje(ip_port:str) -> dict:
    ...