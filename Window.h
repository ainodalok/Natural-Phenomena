#ifndef WINDOW_H
#define WINDOW_H

#include <QApplication>
#include <QBoxLayout>
#include <QDockWidget>
#include <QMainWindow>
#include <QScreen>
#include <QScrollArea>
#include <QStyle>

#include "Widget.h"

class Window final : public QMainWindow
{
Q_OBJECT
public:
	//Constructor / destructor
	Window();

	void createDockWindows();

	//Main widget
	Widget* widget = nullptr;
	QVBoxLayout* scrollLayout = nullptr;
	QScrollArea* scrollArea = nullptr;
	QWidget* scrollWidget = nullptr;

	//Resets all the interface elements
	void ResetInterface();

private:
	void mouseReleaseEvent(QMouseEvent *event) override;
	void changeEvent(QEvent* event) override;	
};
#endif
