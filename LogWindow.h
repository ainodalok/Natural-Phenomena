#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QPlainTextEdit>

class LogWindow : public QPlainTextEdit
{
	Q_OBJECT
public:
	using QPlainTextEdit::QPlainTextEdit;
	static LogWindow* getInstance();
	static void appendMessage(const QString& text);

private:
	static LogWindow* instance;
};

#endif