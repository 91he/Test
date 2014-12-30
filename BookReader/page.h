#ifndef PAGE_H
#define PAGE_H
#include <QVector>
#include <QString>
#include <QPainterPath>
#include <QFont>
class StoryBook{
	int pageWidth;
	int pageHeight;
	int leading;
	int lineOfPage;
	QFont font;
	QString filename;
	QVector<QString> stringVector;
public:
	int pageIndex;
	StoryBook(QString filename, int pageWidth, int pageHeight, int leading, QFont &f);
	void setFont(QFont &f);
	void setLeading(int leading);
	void drawPage(QPainterPath &path);
	void drawNextPage(QPainterPath &path);
};
#endif
