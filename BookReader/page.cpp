#include "page.h"
#include <QFile>
#include <QFontMetrics>
#include <QTextStream>
#include <iostream>
using namespace std;

StoryBook::StoryBook(QString filename, int pageWidth, 
		int pageHeight, int leading, QFont &font): pageWidth(pageWidth), 
	pageHeight(pageHeight), leading(leading), font(font), filename(filename)
{
	pageIndex = 0;
	QFontMetrics fm(font);
	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	QTextStream stream(&file);
	QString line, tmp;
	int n;
	int textWidth = pageWidth - 80;
	int textHeight = pageHeight - 80;
	lineOfPage = textHeight / (fm.height() + leading);

	while(!stream.atEnd()){
		line = stream.readLine();
		line.replace(QChar('\t'), "    ");
		for(n = 0; n < line.length(); n += tmp.length()){
			tmp = fm.elidedText(line.mid(n), Qt::ElideRight, textWidth);
			stringVector.append(line.mid(n, tmp.length()));
		}
	}
	file.close();
}

void StoryBook::setFont(QFont &font){
	stringVector.clear();
	this->font = font;
	QFontMetrics fm(font);
	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	QTextStream stream(&file);
	QString line, tmp;
	int n;
	int textWidth = pageWidth - 80;
	int textHeight = pageHeight - 80;
	lineOfPage = textHeight / (fm.height() + leading);

	while(!stream.atEnd()){
		line = stream.readLine();
		line.replace(QChar('\t'), "    ");
		for(n = 0; n < line.length(); n += tmp.length()){
			tmp = fm.elidedText(line.mid(n), Qt::ElideNone, textWidth);
			stringVector.append(line.mid(n, tmp.length()));
		}
	}
	file.close();
}

void StoryBook::setLeading(int leading){
	QFontMetrics fm(font);
	lineOfPage = (pageHeight - 100) / (fm.height() + leading);
}

void StoryBook::drawPage(QPainterPath &path){
	QFontMetrics fm(font);
	int i = pageIndex * lineOfPage;
	int n = 0, s = 40 + fm.height();
	int lineHeight = fm.height() + leading;

	while(n++ < lineOfPage){
		if(i >= stringVector.size()) break;
		path.addText(QPointF(40, s), font, stringVector[i++]);
		s += lineHeight;
	}
}

void StoryBook::drawNextPage(QPainterPath &path){
	pageIndex++;
	drawPage(path);
	pageIndex--;
}
