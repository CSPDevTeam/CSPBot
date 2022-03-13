#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_cspbot.h"
#include "global.h"
#include "server.h"
#include "qdebug.h"
#include <string>

using namespace std;
QString stdString2QString(std::string str);

class CSPBot : public QMainWindow
{
    Q_OBJECT

public:
    CSPBot(QWidget *parent = Q_NULLPTR);
    bool serverStarted = false;

    //槽信号使用
    void inlog(std::string msgs) {
        QString msg;
        QByteArray byteArray(msgs.c_str(), msgs.length());
        msg = byteArray;
        emit ilog(msg);
        
    }
    void ipipelog(string msgs) {
        QString msg;
        msg = stdString2QString(msgs);
        emit pipelog(msg);
    }
    void changeStatus(bool t) {
        emit chstatus(t);
    }
    void chenableForce(bool a) {
        emit enableForce(a);
    }
    void chLabel(string name,string text) {
        QString q_name = stdString2QString(name);
        QString q_text = stdString2QString(text);

        emit updateText(q_name, q_text);
    }
    inline void ThreadCallback(int t) {
        emit signal_ThreadCallback(t);
    }

    inline void PacketCallback(string msg) {
        QString msgDict;
        msgDict = stdString2QString(msg);
        emit signal_PacketCallback(msgDict);
    }
    inline void MsgCallback(string group,string qq,string msg) {
        QString QGroup,QQq,QMsg;
        QGroup = stdString2QString(group);
        QQq = stdString2QString(qq);
        QMsg = stdString2QString(msg);
        emit signal_MsgCallback(QGroup,QQq,QMsg);
    }
    void InitPlayerTableView(QTableView* t, string head[], int head_length);
    void InitRegularTableView(QTableView* t, string head[], int head_length);
    void InitPluginTableView(QTableView* t, string head[], int head_length);
    
//输出日志信号
signals:
    void ilog(QString msg);
    void pipelog(QString msg);
    void chstatus(bool a);
    void enableForce(bool a);
    void updateText(QString name, QString text);
    void signal_ThreadCallback(int t);
    void signal_PacketCallback(QString dict);
    void signal_MsgCallback(QString QGroup, QString QQq, QString QMsg);
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
    void slotCallback(int t);
    void slotPacketCallback(QString dict);
    void slotMsgCallback(QString QGroup, QString QQq, QString QMsg);
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
