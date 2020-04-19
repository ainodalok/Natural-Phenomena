#include "LogWindow.h"
#include <QtWidgets>

LogWindow* LogWindow::instance;

LogWindow* LogWindow::getInstance()
{
	if (!instance)
	{
		instance = new LogWindow();
		instance->setReadOnly(true);
	}
	return instance;
}

void LogWindow::appendMessage(const QString& text)
{
	QString time = QTime::currentTime().toString("[hh:mm:ss]");
	getInstance()->appendPlainText(time + " " + text); // Adds the message to the widget
	getInstance()->verticalScrollBar()->setValue(getInstance()->verticalScrollBar()->maximum());
}