#include "Widget.h"

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/glu.h>
#include <GL/gl.h>
#include <QOpenGLWidget>
#include <Qt>
#include <QTimer>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>
#include <math.h>
#include <QVector3D>
#include <QtWidgets>
#include "QuadRenderer.h"


//85 degrees fovx is typical for 16:9, which is most common aspect ratio
#define FOV_X 90.0f

//constructor
Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	//grabMouse();
	//setMouseTracking(true);

	timer = new QTimer(this);
	timer->start(0);
	dtimer = new QElapsedTimer();
	dtimer->start();
	timeStamp = dtimer->elapsed();
	//timer for controls
	connect(timer, SIGNAL(timeout()), this, SLOT(timerFunction()));
}

// destructor deletes all glu objects
Widget::~Widget()
{
	delete timer;
	delete dtimer;
	delete quadRenderer;
}

//returns current aspect ratio based on current window
float Widget::aspectRatio()
{
	return float(width()) / float(height());
}

//called when OpenGL context is set up
void Widget::initializeGL()
{
	initializeOpenGLFunctions();
	//set the widget background colour
	glClearColor(1.0, 0.0, 0.0, 0.0);
	//enable depth check
	glEnable(GL_DEPTH_TEST);
	//cull backfaces, we will never see the other side of polygon anyway
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//if normals are >1.0, then rescale to 0.0-1.0 range, useful for dealing with normals like (1.0,1.0,1.0)
	glEnable(GL_RESCALE_NORMAL);

	//glEnable(GL_LIGHTING); // enable lighting in general
	//glEnable(GL_LIGHT0);

	quadRenderer = new QuadRenderer();
}

//called every time the widget is resized
void Widget::resizeGL(int w, int h)
{
	//Rebind all textures 
	if (quadRenderer != NULL)
	{
		quadRenderer->rebindPrecomputedTextures();
	}
}

//called every time the widget needs painting
void Widget::paintGL()
{
	//Time calculations
	//Check time delta since last frame
	dtime = dtimer->elapsed() - timeStamp;

	//Initial rendering setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//using frustum here calculated with formula for yfov from xfov, xfov is usually 90 degrees in 16:9 aspect ratio windows
	//tan(FOV_X / 180 * M_PI*0.5)  = 1 if FOV_X = 90 degrees
	gluPerspective(2.0 * atan(1.0f / aspectRatio()) / M_PI * 180,
		aspectRatio(),
		1.001,
		500.0);
	//clear the widget
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//better to call here, because QT is known for making viewport calls, so it is possible that viewport function
	//will be called between resizeGL() and paintGL() calls
	glViewport(0, 0, width(), height());
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//Time governing
	if (dtime > 0)
	{
		//rotate and move the world according to FPS view
		if (focus)
		{
			//Mouse logic
			angleX += haveToY / sensi;
			angleY += haveToX / sensi;
			if (angleX > 90)
				angleX = 90;
			if (angleX < -90)
				angleX = -90;
			if (angleY > 360)
				angleY = angleY - 360;
			if (angleY < 0)
				angleY = 360 + angleY;
			QPoint cursorPos(width() / 2.0, height() / 2.0);
			QCursor::setPos(mapToGlobal(cursorPos).x(), mapToGlobal(cursorPos).y());
		
			haveToY = 0;
			haveToX = 0;

			//Keyboard movement logic
			camX += velocity.x() * dtime;
			camY += velocity.y() * dtime;
			camZ += velocity.z() * dtime;
		}

		//(quadRenderer->atmosphere)->updateS(dtime);

		timeStamp = dtimer->elapsed();
	}
	

	//Looking at z- with y+ vector up
	gluLookAt(0., 0., 0.,
		0.0, 0.0, -1.0,
		0.0, 1.0, 0.0);

	//Look into angle defs. for more info
	//Increase in vertical angle turns camera up(while world goes down), since it is a ccw roation around X axis
	glRotatef(-angleX, 1, 0, 0);
	//Increase in horizontal angle turns camera right(while world goes left), since it is a ccw rotation around Y axis
	glRotatef(angleY, 0, 1, 0);

	//move camera to the location of controllable character
	//Actually translates world, not camera position, hence negative values are passed as they represent actual camera location in the world without translations!
	glTranslatef(-camX, -camY, -camZ);

	//Call render functions here
	//If window dimensions changed, reconstruct the texture
	//quadRenderer->updateTexture(128, 80, angleX, angleY, FOV_X);
	//quadRenderer->updateTexture(width()/10, height()/10, angleX, angleY, FOV_X);
	
	quadRenderer->renderAtmosphere();

	//Testquad
	glUseProgram(0);
	glColor4f(0.0, 1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1, -1, -2.0f);
	glTexCoord2f(1.0f, 0.0f);   glVertex3f(1, -1, -2.0f);
	glTexCoord2f(1.0f, 1.0f);   glVertex3f(1, 1, -2.0f);
	glTexCoord2f(0.0f, 1.0f);   glVertex3f(-1, 1, -2.0f);
	glEnd();

	//flush to screen
	glFlush();
}

//Enables or disables widget focus and its controls
void Widget::changeFocus()
{
	focus = !focus;
	if (focus)
	{
		//Removes cursor remembering its position
		this->setCursor(Qt::BlankCursor);
		oldPoint = QCursor::pos();
		QPoint cursorPos(width() / 2.0, height() / 2.0);
		QCursor::setPos(mapToGlobal(cursorPos).x(), mapToGlobal(cursorPos).y());
		setFocus();
	}
	else
	{
		//Restores cursor
		QCursor::setPos(oldPoint);
		this->setCursor(Qt::ArrowCursor);
		((QWidget*)parent())->setFocus();
		up = false;
		down = false;
		forward = false;
		back = false;
		sideL = false;
		sideR = false;
	}
}

bool Widget::getFocus()
{
	return focus;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		changeFocus();
}

//All key event handlers are here, triggers bool flags
void Widget::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_R)
		up = true;
	if (event->key() == Qt::Key_F)
		down = true;
	if (event->key() == Qt::Key_W)
		forward = true;
	if (event->key() == Qt::Key_S)
		back = true;
	if (event->key() == Qt::Key_A)
		sideL = true;
	if (event->key() == Qt::Key_D)
		sideR = true;
}

//On release bool flags are removed
void Widget::keyReleaseEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_R)
		up = false;
	if (event->key() == Qt::Key_F)
		down = false;
	if (event->key() == Qt::Key_W)
		forward = false;
	if (event->key() == Qt::Key_S)
		back = false;
	if (event->key() == Qt::Key_A)
		sideL = false;
	if (event->key() == Qt::Key_D)
		sideR = false;
}

//Depending on bool flags camera is either turned or moved
void Widget::controlTimerEvent()
{
	velocity = QVector3D(0.0, 0.0, 0.0);
	QVector3D addVelocity;

	if (forward)
	{
		addVelocity = QVector3D(sin(angleY / float(180) * M_PI) * cos(angleX / float(180) * M_PI), 
								sin(angleX / float(180) * M_PI), 
								-cos(angleY / float(180) * M_PI) * cos(angleX / float(180) * M_PI));
		velocity += addVelocity;
	}

	if (back)
	{
		addVelocity = QVector3D(-sin(angleY / float(180) * M_PI) * cos(angleX / float(180) * M_PI),
								-sin(angleX / float(180) * M_PI),
								cos(angleY / float(180) * M_PI) * cos(angleX / float(180) * M_PI));
		velocity += addVelocity;
	}

	if (sideL)
	{
		addVelocity = QVector3D(-cos(angleY / float(180) * M_PI),
								0.0,
								-sin(angleY / float(180) * M_PI));
		velocity += addVelocity;
	}

	if (sideR)
	{
		addVelocity = QVector3D(cos(angleY / float(180) * M_PI),
								0.0,
								sin(angleY / float(180) * M_PI));
		velocity += addVelocity;
	}

	if (up)
	{
		addVelocity = QVector3D(0.0,
								1.0,
								0.0);
		velocity += addVelocity;
	}

	if (down)
	{
		addVelocity = QVector3D(0.0,
								-1.0,
								0.0);
		velocity += addVelocity;
	}

	velocity.normalize();

	velocity *= speedModifier;
}

//Sensitivity
void Widget::wheelEvent(QWheelEvent *event)
{
	QPoint numDegrees = event->angleDelta() / 8;

	if ((numDegrees.y() > 0) && (sensi > 1))
	{
		sensi -= 1;
	}

	if ((numDegrees.y() < 0) && (sensi < 30))
	{
		sensi += 1;
	}
}

//Mouse control
void Widget::checkMouse()
{
	QPoint screenCenter(width() / 2.0, height() / 2.0);
	//X increases from left to right, hence vector of horizontal direction is given by cursor pos minus center of screen
	float dx = mapFromGlobal(QCursor::pos()).x() - screenCenter.x();
	//Y increases from top to bottom, hence vector of vertical direction is given by  center of screen minus cursor pos
	float dy = screenCenter.y() - mapFromGlobal(QCursor::pos()).y();
	haveToX += dx;
	haveToY += dy;
}

void Widget::startTimer()
{
	timer->start(0);
}

void Widget::stopTimer()
{
	timer->stop();
}

void Widget::timerFunction()
{
	//If widget is focused, enable controls
	if (focus)
	{
		checkMouse();
		controlTimerEvent();
	}
	repaint();
}