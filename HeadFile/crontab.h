#pragma once

#include <qthread.h>

class Crontab :public QThread
{
	Q_OBJECT
protected:
	void run();
public:
	Crontab(QObject* parent = NULL) {};
	~Crontab() {};
signals:
	void stoped();
};