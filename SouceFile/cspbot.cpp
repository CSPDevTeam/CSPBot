#include "cspbot.h"
#include "global.h"
#include <QString> 
#include <QMetaType>
#include "stdafx.h"
#include <qdebug.h>
#include <Windows.h>
#include <yaml-cpp/yaml.h>
#include "CPython.h"
#include "qmap.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include "mirai.h"
#include <QNetworkAccessManager>

using namespace std;

//API
QString stdString2QString(std::string str);
std::string QString2stdString(QString str);

//插入机器人日志
void CSPBot::insertLog(QString a) {
    try {
        ui.logout->setReadOnly(false);
        ui.logout->append(a);
        ui.logout->setReadOnly(true);
    }
    catch (const char e) {

    }
};

//插入BDS日志
void CSPBot::insertPipeLog(QString a) {
    try {
        ui.log->setReadOnly(false);
        ui.log->append(a);
        ui.log->moveCursor(QTextCursor::End);
        ui.log->setReadOnly(true);
    }
    catch (...) {

    }
}

//插入命令
void CSPBot::insertCmd() {
    try {
        string cmd = QString2stdString(ui.input_cmd->text());
        if (cmd != "") {
            ui.input_cmd->setText("");
            server->sendCmd(cmd+"\n");
        }
    }
    catch (...) {

    }
}

//改状态
void CSPBot::changestatus(bool a) {
    string red = "background-color:rgba(255, 0, 0);\nborder-radius:12px;\nborder:2px solid white;";
    string green = "background-color:rgba(0, 255, 0);\nborder-radius:12px;\nborder:2px solid white;";
    if (a) {
        ui.status->setStyleSheet(stdString2QString(green));
    }
    else {
        ui.status->setStyleSheet(stdString2QString(red));
    }
};

//启动进程
void CSPBot::Start_StopProcess(){
    string text = QString2stdString(ui.change->text());
    if (text == u8"启动") {
        server = new Server();
        server->start();
        ui.change->setText(u8"停止");
        ui.log->append(u8"[CSPBot] 已向进程发出启动命令");
        ui.forceStop->setEnabled(true);
    }
    else if(text == u8"停止") {
        server->stopServer();
        ui.log->append(u8"[CSPBot] 已向进程发出终止命令");
    }
}

//强制停止进程
void CSPBot::forceStopProcess() {
    auto temp = QMessageBox::warning(this, u8"警告",u8"服务器还在运行，你是否要强制停止?", QMessageBox::Yes | QMessageBox::No);
    if (temp == QMessageBox::Yes)
    {
        server->forceStopServer();
        ui.log->append(u8"[CSPBot] 已向进程发出强制终止命令");
    }
   
}

//更新强制停止按钮
void CSPBot::changeenableForce(bool a) {
    ui.forceStop->setEnabled(a);

    if (a) {
        ui.change->setText(u8"停止");
    }
    else {
        ui.change->setText(u8"启动");
    }
}

void CSPBot::clear_console() {
    ui.log->setText("");
}

void CSPBot::upText(QString name, QString text) {
    if (name == "world") {
        ui.world->setText(u8"服务器存档:" + text);
    }
    else if (name == "version") {
        ui.version->setText(u8"服务器版本:" + text);
    }
}


void CSPBot::ReloadPlugin() {
    //pluginReload();
}

void CSPBot::slotPacketCallback(QString dict) {
    string strDict = QString2stdString(dict);
    //转换为dict
    Callbacker packetcbe(EventCode::onReceivePacket);
    py::module pyJsonModule = py::module::import("json");
    py::dict msgDict = pyJsonModule.attr("loads")(strDict);
    packetcbe.insert("msg", msgDict);
    bool pakctecb = packetcbe.callback();
}

bool CSPBot::slotOtherCallback(QString cbe, const StringMap& qm) {
    string eventName = QString2stdString(cbe);
    auto event_code = magic_enum::enum_cast<EventCode>(eventName.c_str());
    if (!event_code) {
        return false;
    }

    EventCode ct = event_code.value();

    Callbacker cb(ct);
    QMapIterator<QString, QString> iterator(qm);
    while (iterator.hasNext()) {
        iterator.next();
        string key = QString2stdString(iterator.key());
        string value = QString2stdString(iterator.value());

        cb.insert(key.c_str(), py::str(value));
    }

    return cb.callback();
}

void CSPBot::slotCommandCallback(QString cbe, const StringVerctor& qm) {
    string type = QString2stdString(cbe);
    if (command.find(type) != command.end()) {
        py::list args;
        for (auto &i : qm) {
            args.append(py::str(i));
        }
        py::function cbe = command[type];
        cbe(args);
    }
}

bool CSPBot::slotThreadMirai(QString cbe, const StringMap& qm) {
    string type = QString2stdString(cbe);
    if (type == "sendGroup") {
        string group = QString2stdString(qm.value("group"));
        string msg = QString2stdString(qm.value("msg"));
        mi->sendGroupMsg(group, msg,false);
    }
    else if (type == "sendAllGroup") {
        string msg = QString2stdString(qm.value("msg"));
        mi->sendAllGroupMsg(msg,false);
    }
    else if (type == "recallMsg") {
        string target = QString2stdString(qm.value("target"));
        mi->recallMsg(target,false);
    }
    else if (type == "App") {
        string group = QString2stdString(qm.value("group"));
        string code = QString2stdString(qm.value("code"));
        mi->send_app(group, code);
    }
    else if (type == "sendPacket") {
        string code = QString2stdString(qm.value("packet"));
        mi->SendPacket(code);
     }
    return true;
}


CSPBot::CSPBot(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //窗口样式
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    //设置无边框
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    //阴影设置
    QPushButton* uis[14] = {ui.plugin_reload,ui.console ,ui.player ,ui.settings ,ui.re,ui.plugins,ui.run,ui.change,ui.close,ui.min,ui.botlog,ui.bdspath_button,ui.forceStop,ui.clear};
    for (auto bt : uis) {
        this->setGraphics(bt);
    }
    setAllGraphics();
    ui.log->setReadOnly(true);
    ui.logout->setReadOnly(true);

    //颜色设置
    ui.console->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    ui.playerView->horizontalHeader()->setStretchLastSection(true);

    //表格设置
    InitTable();


    //滚动条
    QScrollBar* bars[3] = {ui.log->verticalScrollBar(),ui.playerView->verticalScrollBar(),ui.logout->verticalScrollBar()};
    for (QScrollBar* bar : bars) {
        setAllScrollbar(bar);
    }
    
    //Enable
    ui.forceStop->setEnabled(false);
    setBackground();
    ui.log->moveCursor(QTextCursor::End);

    //自动更新
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(InitTable()));
    timer->start(3*1000);

    //绑定事件
    connect(ui.close, SIGNAL(clicked()), this, SLOT(on_actionClose_triggered()));
    connect(ui.min, SIGNAL(clicked()), this, SLOT(on_actionMinimize_triggered()));
    connect(ui.console, SIGNAL(clicked()), this, SLOT(switchPage()));
    connect(ui.player, SIGNAL(clicked()), this, SLOT(switchPage()));
    connect(ui.settings, SIGNAL(clicked()), this, SLOT(switchPage()));
    connect(ui.re, SIGNAL(clicked()), this, SLOT(switchPage()));
    connect(ui.plugins, SIGNAL(clicked()), this, SLOT(switchPage()));
    connect(ui.botlog, SIGNAL(clicked()), this, SLOT(switchPage()));
    connect(ui.run, SIGNAL(clicked()), this, SLOT(insertCmd()));
    connect(ui.change, SIGNAL(clicked()), this, SLOT(Start_StopProcess()));
    connect(ui.forceStop, SIGNAL(clicked()), this, SLOT(forceStopProcess()));
    connect(ui.clear, SIGNAL(clicked()), this, SLOT(clear_console()));
    connect(ui.plugin_reload, SIGNAL(clicked()), this, SLOT(ReloadPlugin()));
    connect(this, SIGNAL(ilog(QString)), this, SLOT(insertLog(QString)));
    connect(this, SIGNAL(pipelog(QString)), this, SLOT(insertPipeLog(QString)));
    connect(this, SIGNAL(chstatus(bool)), this, SLOT(changestatus(bool)));
    connect(this, SIGNAL(enableForce(bool)), this, SLOT(changeenableForce(bool)));
    connect(this, SIGNAL(updateText(QString, QString)), this, SLOT(upText(QString, QString)));
    connect(this, SIGNAL(signal_PacketCallback(QString)), this, SLOT(slotPacketCallback(QString)));

    //注册并绑定
    qRegisterMetaType<StringMap>("StringMap");
    qRegisterMetaType<StringVerctor>("StringVerctor");
    connect(
        this, 
        SIGNAL(signal_OtherCallback(QString, const StringMap&)), 
        this, 
        SLOT(slotOtherCallback(QString, const StringMap&))
    );
    connect(
        this,
        SIGNAL(signal_ThreadMirai(QString, const StringMap&)),
        this,
        SLOT(slotThreadMirai(QString, const StringMap&))
    );
    connect(
        this,
        SIGNAL(signal_CommandCallback(QString, const StringVerctor&)),
        this,
        SLOT(slotCommandCallback(QString, const StringVerctor&))
    );

    
}

void CSPBot::setBackground() {
    on_horizontalSlider_sliderMoved(90);
}

QPixmap getRoundRectPixmap(QPixmap srcPixMap, const QSize& size, int radius)
{
    //不处理空数据或者错误数据
    if (srcPixMap.isNull()) {
        return srcPixMap;
    }

    //获取图片尺寸
    int imageWidth = size.width();
    int imageHeight = size.height();

    //处理大尺寸的图片,保证图片显示区域完整
    QPixmap newPixMap = srcPixMap.scaled(imageWidth, (imageHeight == 0 ? imageWidth : imageHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QPixmap destImage(imageWidth, imageHeight);
    destImage.fill(Qt::transparent);
    QPainter painter(&destImage);
    // 抗锯齿
    painter.setRenderHints(QPainter::Antialiasing, true);
    // 图片平滑处理
    painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
    // 将图片裁剪为圆角
    QPainterPath path;
    QRect rect(0, 0, imageWidth, imageHeight);
    path.addRoundedRect(rect, radius, radius);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, imageWidth, imageHeight, newPixMap);
    return destImage;
}

void CSPBot::on_horizontalSlider_sliderMoved(int position)
{
    try{
        //设置新的图片的透明度
        QPixmap pix1_NO("Images/bg.jpg");
        QPixmap pix1_ = getRoundRectPixmap(pix1_NO, pix1_NO.size(), 10);

        QPixmap temp(pix1_.size());
        temp.fill(Qt::transparent);

        QPainter p1(&temp);
        p1.setCompositionMode(QPainter::CompositionMode_Source);
        p1.drawPixmap(0, 0, pix1_);
        p1.setCompositionMode(QPainter::CompositionMode_DestinationIn);

        //根据QColor中第四个参数设置透明度，此处position的取值范围是0～255
        p1.fillRect(temp.rect(), QColor(0, 0, 0, position));
        p1.end();

        pix1_ = temp;
        temp.scaled(ui.background->size(), Qt::KeepAspectRatio);
        ui.background->setScaledContents(true);
        ui.background->setPixmap(pix1_);
    }
    catch (...) {

    }
    
}


void CSPBot::InitTable() {
    string playerhead[3] = { u8"玩家名",u8"XUID",u8"QQ号" };
    InitPlayerTableView(ui.playerView, playerhead, 3);
    string regularhead[4] = { u8"正则",u8"来源",u8"执行",u8"权限" };
    InitRegularTableView(ui.reView, regularhead, 4);
    string pluginhead[5] = {u8"文件名",u8"插件",u8"介绍",u8"版本",u8"作者"};
    InitPluginTableView(ui.pluginView, pluginhead,5);
}

void CSPBot::setAllScrollbar(QScrollBar* bar) {
    bar->setStyleSheet("QScrollBar:vertical"
        "{"
        "width:8px;"
        "background:rgba(0,0,0,0%);"
        "margin:0px,0px,0px,0px;"
        "padding-top:9px;"
        "padding-bottom:9px;"
        "}"
        "QScrollBar::handle:vertical"
        "{"
        "width:8px;"
        "background:rgba(0,0,0,25%);"
        " border-radius:4px;"
        "min-height:20;"
        "}"
        "QScrollBar::handle:vertical:hover"
        "{"
        "width:8px;"
        "background:rgba(0,0,0,50%);"
        " border-radius:4px;"
        "min-height:20;"
        "}"
        "QScrollBar::add-line:vertical"
        "{"
        "height:9px;width:8px;"
        "border-image:url(:/a/Images/a/3.png);"
        "subcontrol-position:bottom;"
        "}"
        "QScrollBar::sub-line:vertical"
        "{"
        "height:9px;width:8px;"
        "border-image:url(:/a/Images/a/1.png);"
        "subcontrol-position:top;"
        "}"
        "QScrollBar::add-line:vertical:hover"
        "{"
        "height:9px;width:8px;"
        "border-image:url(:/a/Images/a/4.png);"
        "subcontrol-position:bottom;"
        "}"
        "QScrollBar::sub-line:vertical:hover"
        "{"
        "height:9px;width:8px;"
        "border-image:url(:/a/Images/a/2.png);"
        "subcontrol-position:top;"
        "}"
        "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical"
        "{"
        "background:rgba(0,0,0,10%);"
        "border-radius:4px;"
        "}"
    );
}

void CSPBot::setAllGraphics() {
    QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-3, 3);
    shadow_effect->setColor(Qt::gray);
    shadow_effect->setBlurRadius(6);
    ui.input_cmd->setGraphicsEffect(shadow_effect);
    QGraphicsDropShadowEffect* shadow_effect2 = new QGraphicsDropShadowEffect(this);
    shadow_effect2->setOffset(-5, 5);
    shadow_effect2->setColor(Qt::gray);
    shadow_effect2->setBlurRadius(8);
    ui.log->setGraphicsEffect(shadow_effect2);
    QGraphicsDropShadowEffect* shadow_effect3= new QGraphicsDropShadowEffect(this);
    shadow_effect3->setOffset(-5, 5);
    shadow_effect3->setColor(Qt::gray);
    shadow_effect3->setBlurRadius(10);
    ui.centralWidget->setGraphicsEffect(shadow_effect3);
    QGraphicsDropShadowEffect* shadow_effect4 = new QGraphicsDropShadowEffect(this);
    shadow_effect4->setOffset(-5, 5);
    shadow_effect4->setColor(Qt::gray);
    shadow_effect4->setBlurRadius(10);
    ui.Icon->setGraphicsEffect(shadow_effect4);
    QGraphicsDropShadowEffect* shadow_effect5 = new QGraphicsDropShadowEffect(this);
    shadow_effect5->setOffset(-5, 5);
    shadow_effect5->setColor(Qt::gray);
    shadow_effect5->setBlurRadius(10);
    ui.Title_Name->setGraphicsEffect(shadow_effect5);
    QGraphicsDropShadowEffect* shadow_effect6 = new QGraphicsDropShadowEffect(this);
    shadow_effect6->setOffset(0, 5);
    shadow_effect6->setColor(Qt::gray);
    shadow_effect6->setBlurRadius(10);
    ui.status->setGraphicsEffect(shadow_effect6);
    QGraphicsDropShadowEffect* shadow_effect7 = new QGraphicsDropShadowEffect(this);
    shadow_effect7->setOffset(-5, 5);
    shadow_effect7->setColor(Qt::gray);
    shadow_effect7->setBlurRadius(10);
    ui.playerView->setGraphicsEffect(shadow_effect7);
    QGraphicsDropShadowEffect* shadow_effect8 = new QGraphicsDropShadowEffect(this);
    shadow_effect8->setOffset(-5, 5);
    shadow_effect8->setColor(Qt::gray);
    shadow_effect8->setBlurRadius(10);
    ui.bdspath_input->setGraphicsEffect(shadow_effect8);
    QGraphicsDropShadowEffect* shadow_effect9 = new QGraphicsDropShadowEffect(this);
    shadow_effect9->setOffset(-5, 5);
    shadow_effect9->setColor(Qt::gray);
    shadow_effect9->setBlurRadius(10);
    ui.status_widget->setGraphicsEffect(shadow_effect9);
    QGraphicsDropShadowEffect* shadow_effect10 = new QGraphicsDropShadowEffect(this);
    shadow_effect10->setOffset(-5, 5);
    shadow_effect10->setColor(Qt::gray);
    shadow_effect10->setBlurRadius(10);
    ui.reView->setGraphicsEffect(shadow_effect10);
    QGraphicsDropShadowEffect* shadow_effect11 = new QGraphicsDropShadowEffect(this);
    shadow_effect11->setOffset(-5, 5);
    shadow_effect11->setColor(Qt::gray);
    shadow_effect11->setBlurRadius(10);
    ui.pluginView->setGraphicsEffect(shadow_effect11);
}

void CSPBot::setGraphics(QPushButton* bt) {
    QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-5, 5);
    shadow_effect->setColor(Qt::gray);
    shadow_effect->setBlurRadius(8);
    bt->setGraphicsEffect(shadow_effect);
}

void CSPBot::InitPlayerTableView(QTableView*t,string head[],int head_length)
{
    try {
        YAML::Node node = YAML::LoadFile("data/player.yml"); //读取player配置文件
        int line_num = node.size();
        QStringList strHeader;
        for (int i = 0; i < head_length;i++) {
            try {
                string header = head[i];
                QByteArray byteArray(header.c_str(), header.length());
                QString msg = byteArray;
                strHeader << msg;
            }
            catch (...) {

            }
        
        }
    
        QStandardItemModel* m_model = new QStandardItemModel();
        //添加表头数据
        m_model->setHorizontalHeaderLabels(strHeader);
        //设置列数
        m_model->setColumnCount(strHeader.size());
        //设置行数
        m_model->setRowCount(line_num);
    
        //隐藏列表头
        t->verticalHeader()->hide();

        //setModel
        t->setModel(m_model);
        //居中显示并设置文本
        int in = 0;
        for (YAML::Node i:node)
        {
            string playerName = i["playerName"].as<string>();
            string xuid = i["xuid"].as<string>();
            string qq = i["qq"].as<string>();
            QStandardItem* item1 = new QStandardItem(stdString2QString(playerName));
            item1->setTextAlignment(Qt::AlignCenter);
            QStandardItem* item2 = new QStandardItem(stdString2QString(xuid));
            item2->setTextAlignment(Qt::AlignCenter);
            QStandardItem* item3 = new QStandardItem(stdString2QString(qq));
            item3->setTextAlignment(Qt::AlignCenter);
            m_model->setItem(in, 0, item1);
            m_model->setItem(in, 1, item2);
            m_model->setItem(in, 2, item3);
            in++;
        }

        //表头设置最后一列数据填充整个控件
        t->horizontalHeader()->setStretchLastSection(true);

        //表头内容均分填充整个控件
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  
    }
    catch (...) {

    }
    
}

void CSPBot::InitRegularTableView(QTableView* t, string head[], int head_length)
{
    try {
        YAML::Node node = YAML::LoadFile("data/regular.yml"); //读取player配置文件
        int line_num = node.size();
        QStringList strHeader;
        for (int i = 0; i < head_length; i++) {
            string header = head[i];
            strHeader << stdString2QString(header);
        }

        QStandardItemModel* m_model = new QStandardItemModel();
        //添加表头数据
        m_model->setHorizontalHeaderLabels(strHeader);
        //设置列数
        m_model->setColumnCount(strHeader.size());
        //设置行数
        m_model->setRowCount(line_num);

        //隐藏列表头
        t->verticalHeader()->hide();

        //setModel.
        t->setModel(m_model);
        //居中显示并设置文本
        int in = 0;
        for (YAML::Node i : node)
        {
            string Regular = i["Regular"].as<string>();
            string Action = i["Action"].as<string>();
            string From = i["From"].as<string>();
            string Permissions = u8"无限制";
            if (i["Permissions"].as<bool>()&&From != "console") {
                Permissions = u8"是";
            }
            else if(!i["Permissions"].as<bool>() && From != "console"){
                Permissions = u8"否";
            }
            QStandardItem* item1 = new QStandardItem(stdString2QString(Regular));
            item1->setTextAlignment(Qt::AlignCenter);
            QStandardItem* item2 = new QStandardItem(stdString2QString(From));
            item2->setTextAlignment(Qt::AlignCenter);
            QStandardItem* item3 = new QStandardItem(stdString2QString(Action));
            item3->setTextAlignment(Qt::AlignCenter);
            QStandardItem* item4 = new QStandardItem(stdString2QString(Permissions));
            item4->setTextAlignment(Qt::AlignCenter);
            m_model->setItem(in, 0, item1);
            m_model->setItem(in, 1, item2);
            m_model->setItem(in, 2, item3);
            m_model->setItem(in, 3, item4);
            in++;
        }

        //表头设置最后一列数据填充整个控件
        t->horizontalHeader()->setStretchLastSection(true);

        //表头内容均分填充整个控件
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    catch (...) {

    }
    
}

void CSPBot::InitPluginTableView(QTableView* t, string head[], int head_length)
{
    int line_num = plugins.size();
    QStringList strHeader;
    for (int i = 0; i < head_length; i++) {
        try {
            string header = head[i];
            strHeader << stdString2QString(header);
        }
        catch (...) {

        }

    }

    QStandardItemModel* m_model = new QStandardItemModel();
    //添加表头数据
    m_model->setHorizontalHeaderLabels(strHeader);
    //设置列数
    m_model->setColumnCount(strHeader.size());
    //设置行数
    m_model->setRowCount(line_num);

    //隐藏列表头
    t->verticalHeader()->hide();

    //setModel
    t->setModel(m_model);
    //居中显示并设置文本
    int in = 0;
    for (auto &i: plugins)
    {
        QStandardItem* item1 = new QStandardItem(stdString2QString(i.first));
        QStandardItem* item2 = new QStandardItem(stdString2QString(i.second.name));
        QStandardItem* item3 = new QStandardItem(stdString2QString(i.second.info));
        QStandardItem* item4 = new QStandardItem(stdString2QString(i.second.version));
        QStandardItem* item5 = new QStandardItem(stdString2QString(i.second.author));

        //居中文本
        item1->setTextAlignment(Qt::AlignCenter);
        item2->setTextAlignment(Qt::AlignCenter);
        item3->setTextAlignment(Qt::AlignCenter);
        item4->setTextAlignment(Qt::AlignCenter);
        item5->setTextAlignment(Qt::AlignCenter);

        m_model->setItem(in, 0, item1);
        m_model->setItem(in, 1, item2);
        m_model->setItem(in, 2, item3);
        m_model->setItem(in, 3, item4);
        m_model->setItem(in, 4, item5);
        in++;
    }
    

    //表头设置最后一列数据填充整个控件
    t->horizontalHeader()->setStretchLastSection(true);

    //表头内容均分填充整个控件
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void CSPBot::switchPage() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());//得到按下的按钮的指针
    QPushButton* btns[6] = {ui.console,ui.player,ui.settings,ui.re,ui.plugins,ui.botlog};
    for (auto i : btns) {
        i->setStyleSheet("QPushButton{\nbackground-color:rgba(239, 239, 239,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(0, 170, 255,90);\n}");
    }
    if (button == ui.console) {
        ui.stack->setCurrentIndex(0);
        button->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    }
    else if (button == ui.player) {
        ui.stack->setCurrentIndex(1);
        button->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    }
    else if (button == ui.settings) {
        ui.stack->setCurrentIndex(2);
        button->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    }
    else if (button == ui.re) {
        ui.stack->setCurrentIndex(3);
        button->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    }
    else if (button == ui.plugins) {
        ui.stack->setCurrentIndex(4);
        button->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    }
    else if (button == ui.botlog) {
        ui.stack->setCurrentIndex(5);
        button->setStyleSheet("QPushButton{\nbackground-color:rgba(255, 170, 0,90);\ncolor:white;\nborder-radius:15px;\n}\nQPushButton:hover{\nbackground-color:rgba(255, 170, 0,90);\n}");
    }
}

//标题栏的长度
const static int pos_min_x = 0;
const static int pos_max_x = 1010;
const static int pos_min_y = 0;
const static int pos_max_y = 30;
//自己实现的窗口拖动操作
void CSPBot::mousePressEvent(QMouseEvent* event)
{
    mousePosition = event->pos();
    //只对标题栏范围内的鼠标事件进行处理
    if (mousePosition.x() <= pos_min_x)
        return;
    if (mousePosition.x() >= pos_max_x)
        return;
    if (mousePosition.y() <= pos_min_y)
        return;
    if (mousePosition.y() >= pos_max_y)
        return;
    isMousePressed = true;
}
void CSPBot::mouseMoveEvent(QMouseEvent* event)
{
    if (isMousePressed == true)
    {
        QPoint movePot = event->globalPos() - mousePosition;
        move(movePot);
    }
}
void CSPBot::mouseReleaseEvent(QMouseEvent* event)
{
    isMousePressed = false;
}

void CSPBot::on_actionMinimize_triggered()
{
    showMinimized();
    //最大化 showMaximized()；
}
void CSPBot::on_actionClose_triggered()
{
    //closeEvent();
    //系统自定义的窗口关闭函数
    close();
}

void CSPBot::closeEvent(QCloseEvent* event) {
    if (server!= nullptr && server->started) {
        auto temp = QMessageBox::warning(
            this, 
            u8"警告", 
            u8"服务器还在运行，你是否要关闭?", 
            QMessageBox::Yes | QMessageBox::No
        );
        if (temp == QMessageBox::Yes)
        {
            server->forceStopServer();
            Callbacker cbe(EventCode::onStop);
            if (cbe.callback()) {
                event->accept();
            }
            else {
                event->ignore();
            }
            
        }
        else
        {
            event->ignore();
        }
        
    }
}




