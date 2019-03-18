
#ifndef _GRADIENT_FRAME_H_
#define _GRADIENT_FRAME_H_

#include <QFrame>
#include <QGradient>
#include <QPainter>
#include <QLabel>
#include <QStyle>
#include <QRect>
#include <QPalette>

class GradientFrame : public QFrame
{

    Q_OBJECT

public:

    GradientFrame(QWidget *parent = 0) : QFrame(parent) { }
    void paintEvent( QPaintEvent * event );
};

class ShadowLabel : public QLabel
{
    Q_OBJECT

public:
        ShadowLabel(QWidget *parent=0) : QLabel(parent) {}
        void paintEvent( QPaintEvent * event );
};


#endif
