#include "Widget.h"

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
	(void)connect(timer, SIGNAL(timeout()), this, SLOT(timerFunction()));

	atmosphere = new Atmosphere();
}

Widget::~Widget()
{
	delete timer;
	delete dtimer;
	delete quadRenderer;
}

//called when OpenGL context is set up
void Widget::initializeGL()
{
	initializeOpenGLFunctions();
	//set the widget background colour
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	//enable depth check
	glEnable(GL_DEPTH_TEST);
	//cull backfaces, we will never see the other side of polygon anyway
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//if normals are >1.0, then rescale to 0.0-1.0 range, useful for dealing with normals like (1.0,1.0,1.0)
	glEnable(GL_RESCALE_NORMAL);

	quadRenderer = new Renderer(width(), height(), atmosphere);
	cam[1] += quadRenderer->atmosphere->Rg;
}

//called every time the widget is resized
void Widget::resizeGL(const int w, const int h)
{
	
}

//called every time the widget needs painting
void Widget::paintGL()
{
	//Rebind all textures 
	if (quadRenderer != nullptr)
	{
		if ((quadRenderer->width != width()) || (quadRenderer->height != height()))
		{
			quadRenderer->rebindPrecomputedTextures();
			quadRenderer->rebuildFramebuffer(width(), height());
		}
	}
	
	//Time calculations
	//Check time delta since last frame
	dtime = dtimer->elapsed() - timeStamp;
	//Time governing
	if (dtime > 0)
	{
		//rotate and move the world according to FPS view
		if (focus)
		{
			QVector3D camTemp;
			const float speedMod = 100000.0f;
			camTemp[0] = cam[0] + velocity.x() * dtime * speedMod;
			camTemp[1] = cam[1] + velocity.y() * dtime * speedMod;
			camTemp[2] = cam[2] + velocity.z() * dtime * speedMod;

			if (camTemp.length() - quadRenderer->atmosphere->Rg > 1.5f)
				cam = camTemp;
			else
				cam = camTemp.normalized() * (quadRenderer->atmosphere->Rg + 1.5f);
		}
		timeStamp = dtimer->elapsed();
	}
	quadRenderer->renderAtmosphere(angleX, angleY, cam);
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
		const QPoint cursorPos(width() / 2, height() / 2);
		QCursor::setPos(mapToGlobal(cursorPos).x(), mapToGlobal(cursorPos).y());
		setFocus();
	}
	else
	{
		//Restores cursor
		QCursor::setPos(oldPoint);
		this->setCursor(Qt::ArrowCursor);
		dynamic_cast<QWidget*>(parent())->setFocus();
		up = false;
		down = false;
		forward = false;
		back = false;
		sideL = false;
		sideR = false;
	}
}

bool Widget::getFocus() const
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
	if (event->key() == Qt::Key_E)
		sunAngleIncrease = true;
	if (event->key() == Qt::Key_Q)
		sunAngleDecrease = true;
	
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
	if (event->key() == Qt::Key_E)
		sunAngleIncrease = false;
	if (event->key() == Qt::Key_Q)
		sunAngleDecrease = false;
}

//Depending on bool flags camera is either turned or moved
void Widget::controlTimerEvent()
{
	velocity = QVector3D(0.0, 0.0, 0.0);
	QVector3D addVelocity;

	if (forward)
	{
		addVelocity = QVector3D(sinf(angleY / 180.0f * M_PIF) * cosf(angleX / 180.0f * M_PIF),
								sinf(angleX / 180.0f * M_PIF),
								-cosf(angleY / 180.0f * M_PIF) * cosf(angleX / 180.0f * M_PIF));
		velocity += addVelocity;
	}

	if (back)
	{
		addVelocity = QVector3D(-sinf(angleY / 180.0f * M_PIF) * cosf(angleX / 180.0f * M_PIF),
								-sinf(angleX / 180.0f * M_PIF),
								cosf(angleY / 180.0f * M_PIF) * cosf(angleX / 180.0f * M_PIF));
		velocity += addVelocity;
	}

	if (sideL)
	{
		addVelocity = QVector3D(-cosf(angleY / 180.0f * M_PIF),
								0.0f,
								-sinf(angleY / 180.0f * M_PIF));
		velocity += addVelocity;
	}

	if (sideR)
	{
		addVelocity = QVector3D(cosf(angleY / 180.0f * M_PIF),
								0.0f,
								sinf(angleY / 180.0f * M_PIF));
		velocity += addVelocity;
	}

	if (up)
	{
		addVelocity = QVector3D(0.0f,
								1.0f,
								0.0f);
		velocity += addVelocity;
	}

	if (down)
	{
		addVelocity = QVector3D(0.0f,
								-1.0f,
								0.0f);
		velocity += addVelocity;
	}

	velocity.normalize();

	velocity *= speedModifier;

	if (sunAngleIncrease)
		atmosphere->updateS(0.25f * dtime);
	if (sunAngleDecrease)
		atmosphere->updateS(-0.25f * dtime);
}

//Sensitivity
void Widget::wheelEvent(QWheelEvent *event)
{
	const QPoint numDegrees = event->angleDelta() / 8;

	if (numDegrees.y() > 0 && sensi > 1)
		sensi -= 1;

	if (numDegrees.y() < 0 && sensi < 30)
		sensi += 1;
}

//Mouse control
void Widget::checkMouse()
{
	const QPoint screenCenter(width() / 2, height() / 2);
	//X increases from left to right, hence vector of horizontal direction is given by cursor pos minus center of screen
	const float dx = static_cast<float>(mapFromGlobal(QCursor::pos()).x() - screenCenter.x());
	//Y increases from top to bottom, hence vector of vertical direction is given by  center of screen minus cursor pos
	const float dy = static_cast<float>(screenCenter.y() - mapFromGlobal(QCursor::pos()).y());
	//Mouse logic
	angleX += dy / sensi;
	angleY += dx / sensi;
	if (angleX > 90)
		angleX = 90;
	if (angleX < -90)
		angleX = -90;
	if (angleY > 360)
		angleY = angleY - 360;
	if (angleY < 0)
		angleY = 360 + angleY;
	QCursor::setPos(mapToGlobal(screenCenter).x(), mapToGlobal(screenCenter).y());
}

void Widget::startTimer() const
{
	timer->start(0);
}

void Widget::stopTimer() const
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