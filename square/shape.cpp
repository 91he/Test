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
	/*
	switch(s){
		case 1:
			x1 = 20, y1 = 0, x2 = 20, y2 = 20, x3 = 20, y3 = 40, x4 = 20, y4 = 60;
			sprintf(color, "red");
			break;
		case 2:
			x1 = 0, y1 = 20, x2 = 20, y2 = 20, x3 = 40, y3 = 20, x4 = 60, y4 = 20;
			sprintf(color, "red");
			break;
		case 3:
			sprintf(color, "green");
			x1 = 20, y1 = 20, x2 = 40, y2 = 20, x3 = 20, y3 = 40, x4 = 40, y4 = 40;
			break;
		case 4:
			sprintf(color, "blue");
			x1 = 20, y1 = 0, x2 = 20, y2 = 20, x3 = 20, y3 = 40, x4 = 40, y4 = 40;
			break;
		case 5:
			sprintf(color, "blue");
			x1 = 0, y1 = 20, x2 = 20, y2 = 20, x3 = 40, y3 = 20, x4 = 40, y4 = 0;
			break;
		case 6:
			sprintf(color, "blue");
			x1 = 0, y1 = 0, x2 = 20, y2 = 0, x3 = 20, y3 = 20, x4 = 20, y4 = 40;
			break;
		case 7:
			sprintf(color, "blue");
			x1 = 0, y1 = 20, x2 = 20, y2 = 20, x3 = 40, y3 = 20, x4 = 0, y4 = 40;
			break;
		case 8:
			sprintf(color, "cyan");
			x1 = 20, y1 = 0, x2 = 20, y2 = 20, x3 = 20, y3 = 40, x4 = 0, y4 = 40;
			break;
		case 9:
			sprintf(color, "cyan");
			x1 = 0, y1 = 20, x2 = 20, y2 = 20, x3 = 40, y3 = 20, x4 = 40, y4 = 40;
			break;
		case 10:
			sprintf(color, "cyan");
			x1 = 20, y1 = 0, x2 = 20, y2 = 20, x3 = 20, y3 = 40, x4 = 40, y4 = 0;
			break;
		case 11:
			sprintf(color, "cyan");
			x1 = 0, y1 = 0, x2 = 0, y2 = 20, x3 = 20, y3 = 20, x4 = 40, y4 = 20;
			break;
		case 12:
			sprintf(color, "magenta");
			x1 = 20, y1 = 0, x2 = 0, y2 = 20, x3 = 20, y3 = 20, x4 = 40, y4 = 20;
			break;
		case 13:
			sprintf(color, "magenta");
			x1 = 20, y1 = 0, x2 = 0, y2 = 20, x3 = 20, y3 = 20, x4 = 20, y4 = 40;
			break;
		case 14:
			sprintf(color, "magenta");
			x1 = 0, y1 = 20, x2 = 20, y2 = 20, x3 = 40, y3 = 20, x4 = 20, y4 = 40;
			break;
		case 15:
			sprintf(color, "magenta");
			x1 = 20, y1 = 0, x2 = 20, y2 = 20, x3 = 20, y3 = 40, x4 = 40, y4 = 20;
			break;
		case 16:
			sprintf(color, "yellow");
			x1 = 20, y1 = 0, x2 = 20, y2 = 20, x3 = 40, y3 = 20, x4 = 40, y4 = 40;
			break;
		case 17:
			sprintf(color, "yellow");
			x1 = 20, y1 = 0, x2 = 40, y2 = 0, x3 = 0, y3 = 20, x4 = 20, y4 = 20;
			break;
		case 18:
			sprintf(color, "gray");
			x1 = 20, y1 = 0, x2 = 0, y2 = 20, x3 = 20, y3 = 20, x4 = 0, y4 = 40;
			break;
		case 19:
			sprintf(color, "gray");
			x1 = 0, y1 = 0, x2 = 20, y2 = 0, x3 = 20, y3 = 20, x4 = 40, y4 = 20;
			break;
	}
*/
	return color;
}

void Shape::change(){
	shape = changeArr[(unsigned int)shape];
	/*
	switch(shape){
		case 1:
		case 4:
		case 5:
		case 6:
		case 8:
		case 9:
		case 10:
		case 12:
		case 13:
		case 14:
		case 16:
		case 18:
			shape++;
			break;
		case 2:
		case 17:
		case 19:
			shape--;
			break;
		case 7:
		case 11:
		case 15:
			shape -= 3;
			break;
	}
	*/
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
