#include <QApplication>
#include "square.h"
#include <QPropertyAnimation>
#include "graph.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);
#if 1
	app.setApplicationName("Mu");
	Square *sq = new Square;
	sq->show();
#else
	QWidget qw;
	QPalette pl;
	pl.setColor(QPalette::Background, QColor(255, 255, 255));
	qw.setPalette(pl);
	qw.show();
	QPropertyAnimation qpa(&qw, "geometry");
	qpa.setDuration(10000);
	/*
	   qpa.setStartValue(QRect(110, arr[i]*20, 110, arr[i]*21));
	   qpa.setKeyValueAt(0.8, QRect(0, arr[i]*20, 220, arr[i]*21));
	   qpa.setEndValue(QRect(0, arr[i]*20+10, 220, arr[i]*20+10));
	   */
	qpa.setStartValue(QRect(0, 0, 100, 30));
	qpa.setEndValue(QRect(250, 250, 200, 60));
	qpa.start();
#endif

	return app.exec();
}
