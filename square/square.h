#ifndef SQUARE_H
#define SQUARE_H
#include <QDialog>
#include <QMainWindow>
#include <stdbool.h>
#include "shape.h"
#include "sth.h"

class QWidget;
class QPainter;
class QKeyEvent;
class QThread;

class Square: public QDialog{
	Q_OBJECT
public:
	Square(QWidget *parent = 0);
	Shape *s;
	Graph *garr[11][22];
	Sth th;
	bool moveable(char m);
	int stoped;
private slots:
	void run();
protected:
	void keyPressEvent(QKeyEvent *event);
};
#endif
