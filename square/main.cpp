#include <QApplication>
#include "square.h"
#include "graph.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);
	Square *sq = new Square;
	sq->show();
	return app.exec();
}
