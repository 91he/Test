#ifndef SQUARE_H
#define SQUARE_H
#include <QDialog>
#include <QMainWindow>
#include <stdbool.h>
#include <phonon>
#include "shape.h"
#include "sth.h"

class QWidget;
class QPainter;
class QKeyEvent;
class QThread;
class QPropertyAnimation;
class QRect;
class QPalette;
class QLabel;

class Square: public QDialog{
	Q_OBJECT
public:
	Square(QWidget *parent = 0);
	~Square();
	QWidget *qw;
	QPropertyAnimation *qpa;
	Shape *s;
	Shape *s2;
	unsigned int score;
	QLabel *scoreLabel;
	QLabel *sL;
	QWidget *topRight;
	Graph *garr[11][22];
	Sth th;
	bool moveable(char m);
	int stoped;
	Phonon::MediaObject *sd;
protected:signals:
	void mksd();
private slots:
	void run();
protected:
	void keyPressEvent(QKeyEvent *event);
	void paintEvent(QPaintEvent *event);
};
#endif
