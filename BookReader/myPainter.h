#ifndef MYPAINTER_H
#define MYPAINTER_H
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QGradient>
#include <QBasicTimer>
#include <QTime>
#include "page.h"

class MyPainter: public QWidget
{
Q_OBJECT
public:
	MyPainter(QWidget *parent = NULL);
	void paintEvent(QPaintEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void timerEvent(QTimerEvent*);
	int w, h;
	double touchX, touchY;
	double touchBeginX, touchBeginY;
	StoryBook *book;
	QBasicTimer timer;
	QTime timeTracker;
	QPointF m_direction;
	QPainterPath currentPagePath;
	QPainterPath nextPagePath;
	QImage imageA;
	QImage imageB;
	QImage imageC;
};
#endif
