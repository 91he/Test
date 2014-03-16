#include "shape.h"
#include <stdio.h>
#include <time.h>

unsigned char changeArr[20] = {0, 2, 
	1, 3, 5, 6, 7, 4, 9, 10, 11, 8, 
	13, 14, 15, 12, 17, 16, 19, 18};

const char *colorArr[20] = {"black", "red",
	"red", "green", "blue", "blue", 
	"blue", "blue", "cyan", "cyan",
	"cyan", "cyan", "magenta", "magenta",
	"magenta", "magenta", "yellow", 
	"yellow", "gray", "gray"};

struct shapeArr shapeArr[20] = {
	{0, 0, 0, 0, 0, 0, 0, 0},
	{20, 0, 20, 20, 20, 40, 20, 60},
	{0, 20, 20, 20, 40, 20, 60, 20},
	{20, 20, 40, 20, 20, 40, 40, 40},
	{20, 0, 20, 20, 20, 40, 40, 40},
	{0, 20, 20, 20, 40, 20, 40, 0},
	{0, 0, 20, 0, 20, 20, 20, 40},
	{0, 20, 20, 20, 40, 20, 0, 40},
	{20, 0, 20, 20, 20, 40, 0, 40},
	{0, 20, 20, 20, 40, 20, 40, 40},
	{20, 0, 20, 20, 20, 40, 40, 0},
	{0, 0, 0, 20, 20, 20, 40, 20},
	{20, 0, 0, 20, 20, 20, 40, 20},
	{20, 0, 0, 20, 20, 20, 20, 40},
	{0, 20, 20, 20, 40, 20, 20, 40},
	{20, 0, 20, 20, 20, 40, 40, 20},
	{20, 0, 20, 20, 40, 20, 40, 40},
	{20, 0, 40, 0, 0, 20, 20, 20},
	{20, 0, 0, 20, 20, 20, 0, 40},
	{0, 0, 20, 0, 20, 20, 40, 20}
};

Shape::Shape(QWidget *parent)
	:QWidget(parent)
{
	shape = rand()%19+1;
	char *color = make(shape);
	g1 = new Graph(color, this);
	g2 = new Graph(color, this);
	g3 = new Graph(color, this);
	g4 = new Graph(color, this);
	g1->resize(22, 22);
	g2->resize(22, 22);
	g3->resize(22, 22);
	g4->resize(22, 22);
	g1->move(x1, y1);
	g2->move(x2, y2);
	g3->move(x3, y3);
	g4->move(x4, y4);
	x = 80;
	y = 0;
	this->move(x, y);
}

Shape::~Shape(){
	delete g1;
	delete g2;
	delete g3;
	delete g4;
}

char* Shape::make(char s){
	static char color[8];
	sprintf(color, "%s", colorArr[(unsigned int)s]);
	x1 = shapeArr[(unsigned int)s].x1;
	x2 = shapeArr[(unsigned int)s].x2;
	x3 = shapeArr[(unsigned int)s].x3;
	x4 = shapeArr[(unsigned int)s].x4;
	y1 = shapeArr[(unsigned int)s].y1;
	y2 = shapeArr[(unsigned int)s].y2;
	y3 = shapeArr[(unsigned int)s].y3;
	y4 = shapeArr[(unsigned int)s].y4;

	return color;
}

void Shape::change(){
	shape = changeArr[(unsigned int)shape];
	make(shape);
	g1->move(x1, y1);
	g2->move(x2, y2);
	g3->move(x3, y3);
	g4->move(x4, y4);
}

void Shape::moveLeft(){
	x -= 20;
	this->move(x, y);
}
void Shape::moveRight(){
	x += 20;
	this->move(x, y);
}
void Shape::moveDown(){
	y += 20;
	this->move(x, y);
}
