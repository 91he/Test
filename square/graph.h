#ifndef GRAPH_H
#define GRAPH_H
#include <QWidget>
#include <QPushButton>
#include <string.h>

class QPaintEvent;
class QPainter;

class Graph : public QWidget{
	Q_OBJECT
public:
	Graph(const char *color, QWidget *parent = 0);
protected:
	void paintEvent(QPaintEvent *event);
public:
	char color[8];
};
#endif
