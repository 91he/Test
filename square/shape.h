#ifndef SHAPE_H
#define SHAPE_H
#include <QWidget>
#include "graph.h"
#include <stdio.h>

class Shape : public QWidget{
	Q_OBJECT
public:
	Shape(QWidget *parent = 0);
	~Shape();
	char* make(char s);
	void change();
	void moveLeft();
	void moveRight();
	void moveDown();
public:
	int x, y;
	int x1, y1;
	int x2, y2;
	int x3, y3;
	int x4, y4;
	char shape;
	Graph *g1;
	Graph *g2;
	Graph *g3;
	Graph *g4;
};

struct shapeArr{
	int x1;
	int y1;
	int x2;
	int y2;
	int x3;
	int y3;
	int x4;
	int y4;
};

#endif
