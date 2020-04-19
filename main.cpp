#include "Window.h"

//RenderDoc Qt Environment Variable
//PATH=H:\Apps\Qt\5.13.0\msvc2017_64\bin
int main(int argc, char *argv[])
{
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
	const Window *window = new Window();

	//start it running
	QApplication::exec();

	//clean up
	//	delete controller;
	delete window;

	//return to caller
	return 0;
}
