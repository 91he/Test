#include <QtGui>
#include "sth.h"

void Sth::stop(){
	stopsign = 1;
}

void Sth::run(){
	stopsign = 0;
	while(1){
		if(stopsign)
			break;
		emit down();
		QThread::msleep(800);
	}
}
