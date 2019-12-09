#include "Window.h"

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <QEvent>
#include <QtWidgets>
#include <GL/glu.h>
#include <GL/gl.h>
#include <QGLWidget>
#include <Qt>
#include <QTimer>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>

//constructor / destructor
Window::Window()
{
	this->setFocusPolicy(Qt::StrongFocus);
	setWindowTitle("Natural Phenomena");
	setGeometry(QStyle::alignedRect(
		Qt::LeftToRight,
		Qt::AlignCenter,
		QSize(1280, 720),
		QApplication::desktop()->availableGeometry(1)
	));
	//show
	showMaximized();
	//Create UI
	createDockWindows();
	//create main widget
	widget = new Widget(this);
	setCentralWidget(widget);
} 

Window::~Window()
{

}

void Window::createDockWindows()
{
	//Create dock widget for all sphere widgets
	QDockWidget* sphereDock = new QDockWidget(tr("Options"), this);
	sphereDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::RightDockWidgetArea, sphereDock);
	sphereDock->setMinimumSize(300, 200);
	sphereDock->setMaximumWidth(500);

	//With scrollable area
	scrollArea = new QScrollArea(sphereDock);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scrollArea->setWidgetResizable(true);
	sphereDock->setWidget(scrollArea);

	//Add one widget to the scroll area (it is its main widget)
	scrollWidget = new QWidget(scrollArea);

	//Create layout for this widget which places all widgets centered and aligned to the top
	scrollLayout = new QVBoxLayout(scrollWidget);
	scrollLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
}

//resets all the interface elements
void Window::ResetInterface()
{
	//now force refresh
	update();
	widget->update();
}

//all key event handlers are here, triggers bool flags
void Window::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		if (widget->getFocus())
		{
			widget->changeFocus();
		}
	}
}

void Window::changeEvent(QEvent *event)
{
	//If tabbed out, return focus
	if (event->type() == QEvent::ActivationChange)
	{
		if (widget->getFocus())
		{
			setFocus();
			widget->changeFocus();
		}
	}
	QWidget::changeEvent(event);
}