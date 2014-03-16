#include <QtGui>
#include "graph.h"

Graph::Graph(const char *color, QWidget *parent)
	:QWidget(parent)
{
	strcpy(this->color, color);// = QString(color);
	this->resize(22, 22);
}

void Graph::paintEvent(QPaintEvent *event){
	QPainter pt(this);
	QColor color(255, 255, 255);
	pt.setPen(QPen(QBrush(color), 3));
	pt.drawRect(0, 0, 21, 21);
	pt.fillRect(2, 2, 18, 18, QBrush(QColor(this->color)));
}
