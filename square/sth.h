#ifndef STH_H
#define STH_H

#include <QThread>

class Sth: public QThread{
	Q_OBJECT
public:
	int stopsign;
	void stop();
protected:
	void run();
signals:	
	   void down();
};
#endif
