#ifndef FIELD_H
#define FIELD_H

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <krandomsequence.h>
#include "defines.h"
#include "dialogs.h"

/* mines field widget */
class Field : public QFrame
{
  Q_OBJECT
	
 public:
    Field(QWidget *parent, const char *name=0);

	QSize sizeHint() const;
	QSizePolicy sizePolicy() const;
	
	void start(const Level &);
	void restart(bool repaint = TRUE);
	void pause();
	void stop() { stopped = TRUE; }
	void showMines();
	
	void up();
	void down();
	void left();
	void right();
	void reveal();
	void mark();
	void autoReveal();

	const Level &level() const { return lev; }
	void setUMark(bool um) { u_mark = um; }
	uint caseSize() const { return _caseSize; }
	void setCaseSize(uint);
	void setCursor(bool show);
	
 public slots:
	void resume();
	
 signals:
	void changeCase(uint, uint);
	void updateStatus(bool);
	void setMood(Smiley::Mood);
	void endGame(int);
	void startTimer();
	void freezeTimer();
	void putMsg(const QString &);
  
 protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	
 private:
	QArray<uint>    _pfield;
	Level           lev;
	KRandomSequence random;
  
	bool paused, stopped;
	bool first_click;
	bool u_mark, cursor;

	int  ic, jc;    // current pos
	bool left_down; // left button pressed
	bool mid_down;  // mid button pressed
  
	uint         _caseSize;
	QPixmap      pm_flag, pm_mine, pm_exploded, pm_error, pm_cursor;
	QPushButton *pb;
  
	uint computeNeighbours(uint, uint) const;
	void uncover(uint, uint);
	void changeCaseState(uint, uint, uint);
	void minePixmap(QPixmap &, bool mask, uint type) const;
	void pressCase(uint, uint, bool, QPainter * = 0);
	void pressClearFunction(uint, uint, bool);
	void uncoverCase(uint, uint);
	bool inside(int, int) const;
	bool placeCursor(int, int, bool check = FALSE);
	void flagPixmap(QPixmap &, bool mask) const;
	void cursorPixmap(QPixmap &, bool mask) const;
	bool locked() const { return stopped || paused; }

	uint &pfield(uint i, uint j) const;
	int xToI(int x) const;
	int yToJ(int y) const;
	int iToX(uint i) const;
	int jToY(uint j) const;

	QPainter *begin(QPainter *);
	void end(QPainter *, const QPainter *);
	void drawCase(uint, uint, QPainter * = 0);
	void drawBox(int, int, bool, QPainter * = 0);
	void eraseField();
	void drawCursor(bool show, QPainter * = 0);
};

#endif // FIELD_H
