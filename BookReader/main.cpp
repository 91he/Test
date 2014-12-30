#include <QtGui>
#include <QtWidgets/QApplication>
#include "myPainter.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);
	MyPainter painter;//= new MyPainter();
	painter.show();
	return app.exec();
}
