#include <QtGui>
#include "square.h"

extern unsigned char changeArr[20];
extern struct shapeArr shapeArr[20];

Square::Square(QWidget *parent)
:QDialog(parent)
{
	stoped = 0;
	score = 0;
	memset(garr, 0, sizeof(garr));
	this->setFixedSize(342,362);

	qw = new QWidget(this);
	qw->setAutoFillBackground(true);
	qw->resize(0, 0);

	QPalette pl;
	pl.setColor(QPalette::Window, QColor(255, 255, 255));
	qw->setPalette(pl);

	pl.setColor(QPalette::Window, QColor(200, 200, 200));
	this->setPalette(pl);

	qpa = new QPropertyAnimation(qw, "geometry");
	qpa->setDuration(100);

	topRight = new QWidget(this);
	topRight->resize(120, 120);
	topRight->move(222, 50);
	topRight->setAutoFillBackground(true);
	pl.setColor(QPalette::Window, QColor(255, 255, 255));
	topRight->setPalette(pl);

	sL = new QLabel("<b>SCORE</b>", this);
	sL->resize(60, 20);
	sL->move(225, 220);
	scoreLabel = new QLabel("0", this);
	scoreLabel->resize(60, 20);
	scoreLabel->move(282, 220);
	scoreLabel->setAlignment(Qt::AlignRight);

	s = new Shape(this);
	s2 = new Shape(this);
	s2->move(250, 70);
	sd = new Phonon::MediaObject(this);
	Phonon::AudioOutput *ao = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	Phonon::createPath(sd, ao);
	s->resize(90, 90);

	connect(&th, SIGNAL(down()), this, SLOT(run()));
	connect(this, SIGNAL(mksd()), sd, SLOT(play()));
	th.start();
}

Square::~Square(){
	delete sd;
	delete s;
}

void Square::paintEvent(QPaintEvent *event){
	QPainter pt(this);
	QColor color(255, 255, 255);
	pt.setPen(QPen(QBrush(color), 2));
	pt.drawRect(0, 0, 222, 362);
	//pt.drawLine(222, 0, 222, 362);
}

bool Square::moveable(char m){
	int x = s->x, y = s->y;
	int x1 = s->x1, y1 = s->y1;
	int x2 = s->x2, y2 = s->y2;
	int x3 = s->x3, y3 = s->y3;
	int x4 = s->x4, y4 = s->y4;
	char shape = s->shape;
	switch(m){
		case 'u':
			shape = changeArr[(unsigned int)shape];
			x1 = shapeArr[(unsigned int)shape].x1;
			x2 = shapeArr[(unsigned int)shape].x2;
			x3 = shapeArr[(unsigned int)shape].x3;
			x4 = shapeArr[(unsigned int)shape].x4;
			y1 = shapeArr[(unsigned int)shape].y1;
			y2 = shapeArr[(unsigned int)shape].y2;
			y3 = shapeArr[(unsigned int)shape].y3;
			y4 = shapeArr[(unsigned int)shape].y4;
			break;
		case 'd':
			y += 20;
			break;
		case 'r':
			x += 20;
			break;
		case 'l':
			x -= 20;
			break;
	}
	bool ret = true;
	if(x+x1 < 0 || x+x1+20 > (this->width()-120)|| 
			x+x2 < 0 || x+x2+20 > (this->width()-120)|| 
			x+x3 < 0 || x+x3+20 > (this->width()-120)||
			x+x4 < 0 || x+x4+20 > (this->width()-120)){
		ret = false;
	}else if(garr[(x+x1)/20][(y+y1)/20] || 
			garr[(x+x2)/20][(y+y2)/20] || 
			garr[(x+x3)/20][(y+y3)/20] || 
			garr[(x+x4)/20][(y+y4)/20]){
		ret = false;
	}else if(y+y1+20 > this->height() ||
			y+y2+20 > this->height() ||
			y+y3+20 > this->height() ||
			y+y4+20 > this->height()){
		ret = false;
	}
	return ret;
}

void Square::keyPressEvent(QKeyEvent *event){
	if(event->key() == ' '){
		if(stoped){
			th.start();
		}else{
			th.stop();
		}
		stoped = ~stoped;
	}else{
		if(!stoped)
			switch(event->key()){
				case Qt::Key_Up:
					if(moveable('u'))
						s->change();
					break;
				case Qt::Key_Down:
					if(moveable('d'))
						s->moveDown();
					break;
				case Qt::Key_Left:
					if(moveable('l')){
						s->moveLeft();
					}
					break;
				case Qt::Key_Right:
					if(moveable('r')){
						s->moveRight();
					}
					break;
			}
	}
}

void Square::run(){
	if(moveable('d')){
		s->moveDown();
	}else{
		sd->setCurrentSource(Phonon::MediaSource("sound/bullet.mp3"));
		emit mksd();
		int x1 = s->x+s->x1, y1 = s->y+s->y1;
		int x2 = s->x+s->x2, y2 = s->y+s->y2;
		int x3 = s->x+s->x3, y3 = s->y+s->y3;
		int x4 = s->x+s->x4, y4 = s->y+s->y4;
		Graph *t = new Graph(s->g1->color, this);
		//t->resize(22, 22);
		t->move(x1, y1);
		t->setVisible(true);
		garr[x1/20][y1/20] = t;
		t = new Graph(s->g2->color, this);
		//t->resize(22, 22);
		t->move(x2, y2);
		t->setVisible(true);
		garr[x2/20][y2/20] = t;
		t = new Graph(s->g3->color, this);
		//t->resize(22, 22);
		t->move(x3, y3);
		t->setVisible(true);
		garr[x3/20][y3/20] = t;
		t = new Graph(s->g4->color, this);
		//t->resize(22, 22);
		t->move(x4, y4);
		t->setVisible(true);
		garr[x4/20][y4/20] = t;

		delete s;
		s2->move(80, 0);
		s = s2;
		s2 = new Shape(this);
		s2->move(250, 70);
		s2->setVisible(true);

		if(garr[(s->x+s->x1)/20][(s->y+s->y1)/20] || 
				garr[(s->x+s->x2)/20][(s->y+s->y2)/20] || 
				garr[(s->x+s->x3)/20][(s->y+s->y3)/20] || 
				garr[(s->x+s->x4)/20][(s->y+s->y4)/20]){
			sd->setCurrentSource(Phonon::MediaSource("sound/game_over.mp3"));
			emit mksd();
			th.stop();
			QMessageBox::information( this, "Information", "Game over!" );
			close();
		}
		s->resize(90, 90);
		s->setVisible(true);

		int arr[4] = {0};
		arr[0] = y1;
		if(y2 != y1){
			arr[1] = y2;
			if(y3 != y2 && y3 != y1){
				arr[2] = y3;
				if(y4 != y3 && y4 != y2 && y4 != y1)
					arr[3] = y4;
			}else{
				if(y4 != y2 && y4 != y1)
					arr[2] = y4;
			}
		}else{
			if(y3 != y1){
				arr[1] = y3;
				if(y4 != y1 && y4 != y3)
					arr[2] = y4;
			}else{
				if(y4 != y1)
					arr[1] = y4;
			}
		}
		int i = 0;
		while(arr[i] && i < 4){
			arr[i] /= 20;
			int w = (this->width()-120)/20;
			int j = arr[i]*w;
			int ret = 0;
			int t = w;
			while(t--){
				if(!garr[t][arr[i]]){
					ret = 1;
					break;
				}
			}
			if(!ret){
				score += 10;
				char tmp[12];
				sprintf(tmp, "%u", score);
				scoreLabel->setText(tmp);
				sd->setCurrentSource(Phonon::MediaSource("sound/achievement.mp3"));
				emit mksd();
				t = w;

				while(t--)
					delete garr[t][arr[i]];

				qpa->setStartValue(QRect(0, arr[i]*20+11, 220, 0));
				qpa->setKeyValueAt(0.2, QRect(0, arr[i]*20+1, 220, 20));
				qpa->setEndValue(QRect(110, arr[i]*20+1, 0, 20));
				qpa->start();
				QTime t;
				t.start();
				while(t.elapsed() < 150)
					    QCoreApplication::processEvents();

				while(--j >= w){
					if(garr[j%w][j/w]){
						garr[j%w][j/w+1] = garr[j%w][j/w];
						garr[j%w][j/w+1]->move((j%w)*20, (j/w+1)*20);
					}else
						garr[j%w][j/w+1] = NULL;
				}
			}
			i++;
		}
	}
}
