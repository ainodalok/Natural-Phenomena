#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>
#include <GL/gl.h>
#include <time.h>
#include "Window.h"
#include <QtDebug>
#include <QResource>

int main(int argc, char *argv[])
{
	srand(time(NULL));

	QSurfaceFormat fmt;
	fmt.setVersion(4, 6);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	QSurfaceFormat::setDefaultFormat(fmt);

	//create the application
	QApplication app(argc, argv);

	//create a master widget
	Window *window = new Window();

	//start it running
	app.exec();

	//clean up
	//	delete controller;
	delete window;

	//return to caller
	return 0;
}
