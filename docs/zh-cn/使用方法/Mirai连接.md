# 配置Mirai
1. 找到Mirai
2. 打开`config/net.mamoe.mirai-api-http/setting.yml`文件
3. 在adapters中添加websocket

```yaml
adapters: 
  - 'http'
  - 'ws'
```

4. 启动你的Mirai

<img src="./zh-cn/使用方法/项目介绍/3.png" alt="Mirai-Http-API输出" style="zoom:60%"/>

5. 当出现上图的时候就代表Mirai已经配置完成了

# 登录机器人
1. 在Mirai控制台中输入`login QQ号 QQ密码`再按回车键即可登录

2. 按照引导操作即可

- `注：建议使用autologin(自动登录) 在控制台输入help即可获取帮助`

# 配置CSPBot
1. 打开`config/config.yml`文件

<img src="./zh-cn/使用方法/项目介绍/3.png" alt="Mirai-Http-API输出" style="zoom:60%"/>

2. 把`[ws adapter] is listening at ws://xxxx:xxxx`中的`ws://xxxx:xxxx`粘贴到`connectUrl`

```yaml
connectUrl: 'ws://127.0.0.1:8080'
```

3. 把`Http api server is running with verifyKey: xxxx`中的`xxxx`复制到`key`

```yaml
key: 'ABCDEFG'
```

4. 把机器人的QQ号写入`qq`

```yaml
qq: 123456789
```

5. 把发消息的群聊写入`group`

```yaml
group: 
  - 123456
  - 654321
```