#ifndef WINDOW_H
#define WINDOW_H

#include <QApplication>
#include <QBoxLayout>
#include <QDockWidget>
#include <QMainWindow>
#include <QStatusBar>
#include <QScreen>
#include <QScrollArea>
#include <QStyle>
#include <QToolButton>
#include <QAction>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>

#include "LogWindow.h"
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
	void recompute();
void apply();

QLineEdit* inputT_W = nullptr;
	QLineEdit* inputT_H = nullptr;
	QLineEdit* inputE_W = nullptr;
	QLineEdit* inputE_H = nullptr;
	QLineEdit* inputMU_R = nullptr;
	QLineEdit* inputMU_MU = nullptr;
	QLineEdit* inputMU_MU_S = nullptr;
	QLineEdit* inputMU_NU = nullptr;
	QLineEdit* inputTRANSMITTANCE_SAMPLES = nullptr;
	QLineEdit* inputSCATTERING_SAMPLES = nullptr;
	QLineEdit* inputSCATTERING_SPHERICAL_SAMPLES = nullptr;
	QLineEdit* inputIRRADIANCE_SPHERICAL_SAMPLES = nullptr;
	QLineEdit* inputORDER_COUNT = nullptr;
	QLineEdit* inputRg = nullptr;
	QLineEdit* inputRt = nullptr;
	QLineEdit* inputsunAngularRadius = nullptr;
	QLineEdit* inputsolarIrradiance = nullptr;
	QLineEdit* inputmuSmin = nullptr;
	QLineEdit* inputrH = nullptr;
	QLineEdit* inputmH = nullptr;
	QLineEdit* inputRBetaR = nullptr;
	QLineEdit* inputRBetaG = nullptr;
	QLineEdit* inputRBetaB = nullptr;
	QLineEdit* inputMBetaR = nullptr;
	QLineEdit* inputMBetaG = nullptr;
	QLineEdit* inputMBetaB = nullptr;
	QLineEdit* inputSurfaceAlbedoR = nullptr;
	QLineEdit* inputSurfaceAlbedoG = nullptr;
	QLineEdit* inputSurfaceAlbedoB = nullptr;
	QLineEdit* inputMieG = nullptr;

	QLineEdit* inputCloudOpacity = nullptr;
	QLineEdit* inputExposure = nullptr;
	QLineEdit* inputWhitePointR = nullptr;
	QLineEdit* inputWhitePointG = nullptr;
	QLineEdit* inputWhitePointB = nullptr;
};
#endif
