#include <stdio.h>
#include "myPainter.h"
#include "page.h"
#define PI 3.1415926

MyPainter::MyPainter(QWidget *parent){
	w = 480, h = 800;
	resize(w, h);
	move(300, 100);
	touchBeginX = w;
	touchBeginY = h;
	touchX = w - 1;
	touchY = h - 1;

	QFont f("times new roman,utopia");
	f.setStyleStrategy(QFont::ForceOutline);
	f.setPointSize(13);
	f.setStyleHint(QFont::Times);
	book = new StoryBook("test.txt", w, h, 5, f);
	book->drawPage(currentPagePath);
	book->drawNextPage(nextPagePath);

	imageA = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
	imageA.fill(Qt::lightGray);
	QPainter painterA(&imageA);
	painterA.setBrush(QBrush(Qt::black));
	painterA.drawPath(currentPagePath);

	imageB = imageA;

	imageC = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
	imageC.fill(Qt::lightGray);
	QPainter painterC(&imageC);
	painterC.setBrush(QBrush(Qt::black));
	painterC.drawPath(nextPagePath);
}

void MyPainter::paintEvent(QPaintEvent *e){
	QPainter widgetPainter(this);
	QImage tmpImageA(imageA);
	QImage tmpImageC(imageC);
	QImage tmpImageB(imageB);
    QPainter painterA(&tmpImageA);
	QPainter painterB(&tmpImageB);
	QPainter painterC(&tmpImageC);
	double relativeX, relativeY;
	double cornerX, cornerY, cornerB;
	double r, d;
	double k, k1, b1;
	double bezierControl1AX, bezierControl1AY;
	double bezierControl1BX, bezierControl1BY;
	double bezierStart1X, bezierStart1Y;
	double bezierEnd1X, bezierEnd1Y;
	double bezierVertex1X, bezierVertex1Y;

	double bezierControl2AX, bezierControl2AY;
	double bezierControl2BX, bezierControl2BY;
	double bezierStart2X, bezierStart2Y;
	double bezierEnd2X, bezierEnd2Y;
	double bezierVertex2X, bezierVertex2Y;

	relativeX = touchX - touchBeginX;
	relativeY = touchY - touchBeginY;
	relativeX = relativeX < 0 ? relativeX : -1;
	relativeY = relativeY < 0 ? relativeY : -1;

	k = relativeY / relativeX;
	k1 = -1 / k;

	cornerX = w + relativeX;
	cornerY = h + relativeY;
	cornerB = cornerY - k1 * cornerX;

	d = qSqrt(relativeX * relativeX + relativeY * relativeY);
	r = d / 4;

	bezierStart1X = w;
	bezierStart1Y = h - (h - k1 * bezierStart1X - cornerY + k1 * cornerX) * (d + PI * r) / (2 * d);
	b1 = bezierStart1Y - k1 * bezierStart1X;

	bezierStart2Y = h;
	bezierStart2X = (bezierStart2Y - b1) / k1;

	bezierEnd2Y = cornerY + (d - PI * r) * (h - cornerY) / d;
	bezierEnd2X = (bezierEnd2Y - b1) / k1;

	qreal tcX = (bezierStart2X + bezierEnd2X) / 2 + 4 * r / (3 * qSqrt(k * k + 1));
	qreal tcY = (bezierStart2Y + bezierEnd2Y) / 2 + 4 * r / (3 * qSqrt(k1 * k1 + 1));

	bezierVertex2X = (bezierStart2X + bezierEnd2X) / 2 + r / qSqrt(k * k + 1);
	bezierVertex2Y = (bezierStart2Y + bezierEnd2Y) / 2 + r / qSqrt(k1 * k1 + 1);

	bezierControl2AY = h;
	bezierControl2AX = (bezierControl2AY - tcY + k1 * tcX) / k1;

	bezierControl2BX = 2 * tcX - bezierControl2AX;
	bezierControl2BY = 2 * tcY - bezierControl2AY;

	bezierEnd1X = cornerX + (d - PI * r) * (w - cornerX) / d;
	bezierEnd1Y = k1 * bezierEnd1X + b1;

	bezierVertex1X = (bezierStart1X + bezierEnd1X) / 2 + r / qSqrt(k * k + 1);
	bezierVertex1Y = (bezierStart1Y + bezierEnd1Y) / 2 + r / qSqrt(k1 * k1 + 1);

	tcX = (bezierStart1X + bezierEnd1X) / 2 + 4 * r / (3 * qSqrt(k * k + 1));
	tcY = (bezierStart1Y + bezierEnd1Y) / 2 + 4 * r / (3 * qSqrt(k1 * k1 + 1));

	bezierControl1AX = w;
	bezierControl1AY = k1 * bezierControl1AX + tcY - k1 * tcX;

	bezierControl1BX = 2 * tcX - bezierControl1AX;
	bezierControl1BY = 2 * tcY - bezierControl1AY;

	QPainterPath pathA;
	pathA.moveTo(0, 0);
	pathA.lineTo(w, 0);
	pathA.lineTo(qRound(bezierStart1X), qRound(bezierStart1Y));
	pathA.cubicTo(qRound(bezierControl1AX), qRound(bezierControl1AY), 
			qRound(bezierControl1BX), qRound(bezierControl1BY),
			qRound(bezierEnd1X), qRound(bezierEnd1Y));
	pathA.lineTo(qRound(cornerX), qRound(cornerY));
	pathA.lineTo(qRound(bezierEnd2X), qRound(bezierEnd2Y));
	pathA.cubicTo(qRound(bezierControl2BX), qRound(bezierControl2BY), 
			qRound(bezierControl2AX), qRound(bezierControl2AY),
			qRound(bezierStart2X), qRound(bezierStart2Y));
	pathA.lineTo(0, h);
	pathA.closeSubpath();

	//QLinearGradient linearA(QPointF((cornerX + w) / 2, (cornerY + h) /2), QPointF(cornerX, cornerY));
	//linearA.setColorAt(0.2, Qt::white);
	//linearA.setColorAt(0.7, Qt::lightGray);
	//linearA.setColorAt(0.3, QColor(210, 210, 210, 215));
	//linearA.setColorAt(0.8, QColor(215, 215, 215, 0));
	/*
	painterA.setPen(Qt::NoPen);
	painterA.setBrush(linearA);
	painterA.drawPath(pathA);
	*/

	QPainterPath pathB;
	pathB.moveTo(w, h);
	pathB.lineTo(qRound(bezierStart1X), qRound(bezierStart1Y));
	pathB.cubicTo(qRound(bezierControl1AX), qRound(bezierControl1AY), 
			qRound(bezierControl1BX), qRound(bezierControl1BY),
			qRound(bezierEnd1X), qRound(bezierEnd1Y));
	pathB.lineTo(qRound(cornerX), qRound(cornerY));
	pathB.lineTo(qRound(bezierEnd2X), qRound(bezierEnd2Y));
	pathB.cubicTo(qRound(bezierControl2BX), qRound(bezierControl2BY), 
			qRound(bezierControl2AX), qRound(bezierControl2AY),
			qRound(bezierStart2X), qRound(bezierStart2Y));
	pathB.closeSubpath();

	QLinearGradient linearB(QPointF(w, h), QPointF(cornerX, cornerY));
	linearB.setColorAt(0.6, QColor(0xff404040));
	linearB.setColorAt(0.9, QColor(0xffaaaaaa));
	painterB.setPen(Qt::NoPen);
	painterB.setBrush(linearB);

	QPainterPath pathD;
	pathD.moveTo(qRound(cornerX), qRound(cornerY));
	pathD.lineTo(qRound(bezierVertex1X), qRound(bezierVertex1Y));
	pathD.lineTo(qRound(bezierVertex2X), qRound(bezierVertex2Y));
	pathD.closeSubpath();
	painterB.drawPath(pathB & pathD);

	QPainterPath pathC = pathB.subtracted(pathB.intersected(pathD));

	QLinearGradient linearC(QPointF(cornerX, cornerY), QPointF(w, h));
	linearC.setColorAt(0, Qt::black);
	linearC.setColorAt(0.46, QColor(0, 0, 0, 180));
	linearC.setColorAt(0.57, QColor(0, 0, 0, 0));
	painterC.setBrush(linearC);
	painterC.setPen(Qt::NoPen);
	painterC.drawPath(pathC);

	QPainterPath testPath(currentPagePath);

	painterC.setCompositionMode(QPainter::CompositionMode_Source);
	painterC.fillPath(pathA + pathD, Qt::transparent);
	painterC.setPen(QColor(40, 40, 40, 255));
	painterC.drawLine(qRound(bezierVertex1X), qRound(bezierVertex1Y), qRound(bezierVertex2X), qRound(bezierVertex2Y));

#if 1
	int i, j;
	qreal tx, ty, px, py;
	//for(i = 0; i < testPath.elementCount(); i++){
	for(j = 0; j < h; j++){
		for(i = 0; i < w; i++){
			//const QPainterPath::Element &e = testPath.elementAt(i);
			tx = i;
			ty = j;
			if((k1 * tx + b1) >= ty) continue;
			if(tmpImageA.pixel(i, j) != 0xff000000) continue;
			px = (b1 - ty + k * tx) / (k - k1);
			py = k1 * px + b1;
			d = qSqrt((tx - px) * (tx - px) + (ty - py) * (ty - py));
			if(d < PI * r / 2)
				d = d - r * qSin(d / r);
			else
				continue;

			tx -= d / qSqrt(k * k + 1);
			ty -= k * d / qSqrt(k * k + 1);
			if(tx < 0 || tx >= w || ty < 0 || ty >= h) continue;
			if(qRound(tx) == i && qRound(ty) == j) continue;
			tmpImageA.setPixel(qRound(tx), qRound(ty), 0xff000000);
			tmpImageA.setPixel(i, j, QColor(Qt::lightGray).rgb());
		}
		//testPath.setElementPositionAt(i, tx, ty);
	}
	//painterA.setPen(Qt::black);
	//painterA.setBrush(Qt::black);
	//painterA.drawPath(testPath);
	//painterA.setPen(Qt::NoPen);
	//painterA.setBrush(linearA);
	//painterA.drawPath(pathA);
	//painterA.drawLine(qRound(bezierStart1X), qRound(bezierStart1Y), qRound(bezierStart2X), qRound(bezierStart2Y));
	painterA.setCompositionMode(QPainter::CompositionMode_Source);
	painterA.fillPath(pathB, Qt::transparent);

	for(j = 0; j < h; j++){
		for(i = 0; i < w; i++){
			tx = i;
			ty = j;
			if((k1 * tx + cornerB) >= ty) continue;
			if(tmpImageB.pixel(i, j) != 0xff000000) continue;
			px = (b1 - ty + k * tx) / (k - k1);
			py = k1 * px + b1;
			d = qSqrt((tx - px) * (tx - px) + (ty - py) * (ty - py));
			if(d < PI * r){
				if(d <= (PI / 2) * r) continue;
				d = d - r * qSin(d / r);
			}else{
				d = d + d - PI * r;
			}

			tx -= d / qSqrt(k * k + 1);
			ty -= k * d / qSqrt(k * k + 1);
			if(tx < 0 || tx >= w || ty < 0 || ty >= h) continue;
			tmpImageB.setPixel(qRound(tx), qRound(ty), 0xff444444);
		}
	}
#endif
	widgetPainter.drawImage(0, 0, tmpImageB);
	widgetPainter.drawImage(0, 0, tmpImageA);
	widgetPainter.drawImage(0, 0, tmpImageC);
}

void MyPainter::mouseMoveEvent(QMouseEvent *e){
	touchX = e->x();
	touchY = e->y();
	update(0, 0, w, h);
}
void MyPainter::mousePressEvent(QMouseEvent *e){
	touchBeginX = e->x();
	touchBeginY = e->y();
}

void MyPainter::mouseReleaseEvent(QMouseEvent *e){
	QPointF tmp(e->pos());
	if(e->y() >= touchBeginY)  tmp.setY(touchBeginY - 1);

	if(tmp.x() < (w / 2))
		m_direction = QPointF(touchBeginX - 2 * w, touchBeginY) - tmp;
	else
		m_direction = QPointF(touchBeginX, touchBeginY) - tmp;
	touchX = tmp.x();
	touchY = tmp.y();

	timer.start(1, this);
	timeTracker.start();
}

void MyPainter::timerEvent(QTimerEvent *e){
	qreal time = timeTracker.restart() / 500.0;
	touchX += m_direction.x() * time;
	touchY += m_direction.y() * time;
	if(touchX <= touchBeginX - 2 * w || touchX >= touchBeginX || touchY >= touchBeginY){
		if(m_direction.x() < 0){
			nextPagePath = QPainterPath();
			book->pageIndex++;
			book->drawNextPage(nextPagePath);

			imageA = imageB = imageC;

			imageC = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
			imageC.fill(Qt::lightGray);
			QPainter painterC(&imageC);
			painterC.setBrush(QBrush(Qt::black));
			painterC.drawPath(nextPagePath);
		}
		touchX = touchBeginX - 1;
		touchY = touchBeginY - 1;
		update(0, 0, w, h);
		timer.stop();
	}
	update(0, 0, w, h);
}
