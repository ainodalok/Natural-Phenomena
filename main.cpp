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

//RenderDoc Qt Environment Variable
//PATH=H:\Apps\Qt\5.13.0\msvc2017_64\bin
int main(int argc, char *argv[])
{
	srand(time(NULL));

	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(6);
	//Might be faster on NVIDIA cards + test quad rendering using legacy OpenGL
	format.setProfile(QSurfaceFormat::CompatibilityProfile);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	//Disable VSync
	format.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(format);

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
