#ifndef WIDGET_H
#define WIDGET_H

#include <QElapsedTimer>
#include <QKeyEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLWidget>
#include <QTimer>

#include "QuadRenderer.h"

static const float M_PIF = static_cast<float>(M_PI);

class Widget final : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	explicit Widget(QWidget* parent = nullptr);
	~Widget();
	Widget(const Widget&) = delete;
	Widget& operator=(const Widget&) = delete;
	Widget(const Widget&&) = delete;
	Widget& operator=(const Widget&&) = delete;
	void stopTimer() const;
	void startTimer() const;
	void changeFocus();
	[[nodiscard]] bool getFocus() const;

protected:
	//called when OpenGL context is set up
	void initializeGL() override;
	//called every time the widget is resized
	void resizeGL(int w, int h) override;
	//called every time the widget needs painting
	void paintGL() override;

private:
	void keyPressEvent(QKeyEvent * event) override;
	void keyReleaseEvent(QKeyEvent * event) override;

	void mousePressEvent(QMouseEvent *event) override;

	void wheelEvent(QWheelEvent *event) override;

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
	float angleY = 0.0f;
	//Vertical camera angle increases as camera turns up(cw rotation)
	float angleX = 0.0f;
	float camX = 0.0f;
	float camY = 1.5f;
	float camZ = 5.0f;

	//Velocity vector
	QVector3D velocity = QVector3D(0.0, 0.0, 0.0);
	float speedModifier = 0.01f;

	//Mouse properties
	float mouseX = 0.0f;
	float mouseY = 0.0f;
	int haveToY = 0;
	int haveToX = 0;
	float sensi = 5.0f;
	
	//Time governing
	float dtime = 0.0f;
	float timeStamp = 0.0f;

	//Timers
	QTimer *timer = nullptr;
	QElapsedTimer *dtimer = nullptr;

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
