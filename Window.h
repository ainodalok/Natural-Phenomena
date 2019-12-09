#ifndef WINDOW_H
#define WINDOW_H

#include <QGLWidget>
#include <QMenuBar>
#include <QSlider>
#include <QBoxLayout>
#include <QObject>
#include "Widget.h"
#include <QMainWindow>
#include <QScrollArea>
#include <QLineEdit>

class Window : public QMainWindow
{
Q_OBJECT
public:
	//Constructor / destructor
	Window();
	~Window();

	void createDockWindows();

	//Main widget
	Widget* widget = 0;
	QVBoxLayout* scrollLayout;
	QScrollArea* scrollArea;
	QWidget* scrollWidget;

	//Resets all the interface elements
	void ResetInterface();

protected:

private:

	virtual void mouseReleaseEvent(QMouseEvent *event);

	virtual void changeEvent(QEvent* event);	
};
#endif
