#ifndef WIDGET_H
#define WIDGET_H

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#include <Qt>
#include <QApplication>
#include <QtWidgets>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QVector3D>
#include <QDebug>
#include <math.h>
#include "QuadRenderer.h"


class Widget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	Widget(QWidget* parent = 0);
	~Widget();
	void stopTimer();
	void startTimer();
	void changeFocus();
	bool getFocus();

protected:
	//called when OpenGL context is set up
	void initializeGL() override;
	//called every time the widget is resized
	void resizeGL(int w, int h) override;
	//called every time the widget needs painting
	void paintGL() override;

private:
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void keyReleaseEvent(QKeyEvent * event);

	virtual void mousePressEvent(QMouseEvent *event);

	virtual void wheelEvent(QWheelEvent *event);

	//Manages controls
	void controlTimerEvent();
	//Checks mouse movement
	void checkMouse();

	//Widget focus
	bool focus = false;

	//Movement button press bools
	bool forward = false;
	bool back = false;
	bool sideR = false;
	bool sideL = false;
	bool up = false;
	bool down = false;

	//Camera properties
	//Horizontal camera angle increases as camera turns right(cw rotation)
	float angleY = 0;
	//Vertical camera angle increases as camera turns up(cw rotation)
	float angleX = 0;
	float camX = 0;
	float camY = 1.5;
	float camZ = 5;

	//Velocity vector
	QVector3D velocity = QVector3D(0.0, 0.0, 0.0);
	float speedModifier = 0.01f;

	//Mouse properties
	float mouseX = 0;
	float mouseY = 0;
	int haveToY = 0;
	int haveToX = 0;
	float sensi = 5;
	
	//Time governing
	float dtime = 0;
	float timeStamp = 0;

	//Timers
	QTimer *timer;
	QElapsedTimer *dtimer;

	//Effects to render
	QuadRenderer* quadRenderer = nullptr;

	//Data for old mouse position
	QPoint oldPoint;

	QOpenGLDebugLogger* logger = nullptr;

private slots:
	//Timer trigger
	void timerFunction();
};
#endif
