#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_cspbot.h"
#include "global.h"
#include "server.h"
#include "qdebug.h"
#include "qmap.h"
#include <string>
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include "qnetworkaccessmanager.h"

using namespace std;

//转换API
QString stdString2QString(std::string str);

class CSPBot : public QMainWindow
{
    Q_OBJECT

public:
    //注册线程类型
    typedef QMap<QString, QString> StringMap;

    CSPBot(QWidget *parent = Q_NULLPTR);
    bool serverStarted = false;

    //槽信号使用
    void inlog(std::string msgs) {
        QString msg;
        QByteArray byteArray(msgs.c_str(), msgs.length());
        msg = byteArray;
        emit ilog(msg);
        
    }

    //向BDS控制台添加日志
    void ipipelog(string msgs) {
        QString msg;
        msg = stdString2QString(msgs);
        emit pipelog(msg);
    }

    //切换运行状态指示器
    void changeStatus(bool t) {
        emit chstatus(t);
    }

    //允许使用强制停止
    void chenableForce(bool a) {
        emit enableForce(a);
    }

    //更改Label文字
    void chLabel(string name,string text) {
        QString q_name = stdString2QString(name);
        QString q_text = stdString2QString(text);

        emit updateText(q_name, q_text);
    }

    //Callback
    inline void PacketCallback(string msg) {
        QString msgDict;
        msgDict = stdString2QString(msg);
        emit signal_PacketCallback(msgDict);
    }
    
    //其他线程Call
    inline bool OtherCallback(string cb, std::unordered_map<string, string> args={}) {
        QMap<QString, QString> qm;
        QString cbeCode = stdString2QString(cb);
        //传参
        for (auto& i : args) {
            QString key = stdString2QString(i.first);
            QString value = stdString2QString(i.second);
            qm.insert(key, value);
        }

        //Callback
        return emit signal_OtherCallback(cbeCode, qm);
    }

    //表格自动写入
    void InitPlayerTableView(QTableView* t, string head[], int head_length);
    void InitRegularTableView(QTableView* t, string head[], int head_length);
    void InitPluginTableView(QTableView* t, string head[], int head_length);

    //HTTP请求
    void onSendHttpsRequest();
    
//Signals信号
signals:
    void ilog(QString msg);
    void pipelog(QString msg);
    void chstatus(bool a);
    void enableForce(bool a);
    void updateText(QString name, QString text);
    void signal_PacketCallback(QString dict);
    bool signal_OtherCallback(QString cbe, const StringMap& qm);

//Slot函数
private slots:
    void on_actionMinimize_triggered();//最小化窗口
    void on_actionClose_triggered();//关闭窗口
    void switchPage();//切换函数
    void insertLog(QString a);
    void insertPipeLog(QString a);
    void insertCmd();
    void changestatus(bool a);
    void Start_StopProcess();
    void forceStopProcess();
    void clear_console();
    void upText(QString name, QString text);
    void changeenableForce(bool a);
    void InitTable();
    void ReloadPlugin();
    void slotPacketCallback(QString dict);
    bool slotOtherCallback(QString cbe, const StringMap& qm);

//自有函数
private:
    Ui::CSPBotClass ui;
    void mouseMoveEvent(QMouseEvent* event);
    void setAllScrollbar(QScrollBar* bar);
    void setAllGraphics();
    void setGraphics(QPushButton* bt);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent*);
    void InitTableView(QTableView* t,string head[],int head_length, int line_num){};
    void setBackground();
    void on_horizontalSlider_sliderMoved(int position);
    
    //关闭事件
    void closeEvent(QCloseEvent* event);
    QPoint mousePosition;
    bool isMousePressed;
};
