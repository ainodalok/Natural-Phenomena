#include "Window.h"

//constructor / destructor
Window::Window()
{
	this->setFocusPolicy(Qt::StrongFocus);
	setWindowTitle("Natural Phenomena");
	setGeometry(QStyle::alignedRect(
		Qt::LeftToRight,
		Qt::AlignCenter,
		QSize(1280, 720),
		QApplication::screens().back()->availableGeometry()
	));
	//show
	showMaximized();

	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	
	//create main widget
	widget = new Widget(this);
	setCentralWidget(widget);

	// Create UI
	createDockWindows();
} 

void Window::createDockWindows()
{
	//LOGGER
	QDockWidget* outputDock = new QDockWidget(tr("Output"), this);
	outputDock->setWidget(LogWindow::getInstance());
	outputDock->setFeatures(QDockWidget::DockWidgetClosable);
	addDockWidget(Qt::BottomDockWidgetArea, outputDock);
	
	QAction* output = outputDock->toggleViewAction();
	output->setText(tr("&Output"));
	QToolButton* outputTool = new QToolButton();
	outputTool->setToolButtonStyle(Qt::ToolButtonTextOnly);
	outputTool->setDefaultAction(output);
	
	//OPTIONS
	//Create dock widget for all sphere widgets
	auto optionsDock = new QDockWidget(tr("Options"), this);
	optionsDock->setFeatures(QDockWidget::DockWidgetClosable);
	addDockWidget(Qt::LeftDockWidgetArea, optionsDock);
	optionsDock->setMinimumSize(400, 200);
	optionsDock->setMaximumWidth(400);

	//With scrollable area
	scrollArea = new QScrollArea(optionsDock);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scrollArea->setWidgetResizable(true);
	optionsDock->setWidget(scrollArea);

	//Add one widget to the scroll area (it is its main widget)
	scrollWidget = new QWidget(scrollArea);

	//Create layout for this widget which places all widgets centered and aligned to the top
	scrollLayout = new QVBoxLayout(scrollWidget);
	scrollLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

	scrollArea->setWidget(scrollWidget);
	scrollWidget->setLayout(scrollLayout);
	
	QAction* options = optionsDock->toggleViewAction();
	options->setText(tr("&Options"));
	QToolButton* optionsTool = new QToolButton();
	optionsTool->setToolButtonStyle(Qt::ToolButtonTextOnly);
	optionsTool->setDefaultAction(options);

	QGroupBox* textureGroup = new QGroupBox(tr("Precomputation Parameters"), scrollWidget);
	QGridLayout* textureLayout = new QGridLayout(textureGroup);
	textureGroup->setLayout(textureLayout);
	scrollLayout->addWidget(textureGroup);

	QLabel* labelT_W = new QLabel("Transmittance texture width (R):", textureGroup);
	textureLayout->addWidget(labelT_W, 0, 0, 1, 2);
	inputT_W = new QLineEdit(QString::number(widget->atmosphere->T_W), textureGroup);
	textureLayout->addWidget(inputT_W, 0, 2, 1, 2);
	
	QLabel* labelT_H = new QLabel("Transmittance texture height (Mu):", textureGroup);
	textureLayout->addWidget(labelT_H, 1, 0, 1, 2);
	inputT_H = new QLineEdit(QString::number(widget->atmosphere->T_H), textureGroup);
	textureLayout->addWidget(inputT_H, 1, 2, 1, 2);
	
	QLabel* labelE_W = new QLabel("Irradiance texture width (R):", textureGroup);
	textureLayout->addWidget(labelE_W, 2, 0, 1, 2);
	inputE_W = new QLineEdit(QString::number(widget->atmosphere->E_W), textureGroup);
	textureLayout->addWidget(inputE_W, 2, 2, 1, 2);
	
	QLabel* labelE_H = new QLabel("Irradiance texture height (MuS):", textureGroup);
	textureLayout->addWidget(labelE_H, 3, 0, 1, 2);
	inputE_H = new QLineEdit(QString::number(widget->atmosphere->E_H), textureGroup);
	textureLayout->addWidget(inputE_H, 3, 2, 1, 2);
	
	QLabel* labelMU_R = new QLabel("Scattering texture layers (R):", textureGroup);
	textureLayout->addWidget(labelMU_R, 4, 0, 1, 2);
	inputMU_R = new QLineEdit(QString::number(widget->atmosphere->MU_R), textureGroup);
	textureLayout->addWidget(inputMU_R, 4, 2, 1, 2);
	
	QLabel* labelMU_MU = new QLabel("Scattering texture height (Mu):", textureGroup);
	textureLayout->addWidget(labelMU_MU, 5, 0, 1, 2);
	inputMU_MU = new QLineEdit(QString::number(widget->atmosphere->MU_MU), textureGroup);
	textureLayout->addWidget(inputMU_MU, 5, 2, 1, 2);
	
	QLabel* labelMU_MU_S = new QLabel("Scattering texture width 1* (MuS):", textureGroup);
	textureLayout->addWidget(labelMU_MU_S, 6, 0, 1, 2);
	inputMU_MU_S = new QLineEdit(QString::number(widget->atmosphere->MU_MU_S), textureGroup);
	textureLayout->addWidget(inputMU_MU_S, 6, 2, 1, 2);
	
	QLabel* labelMU_NU = new QLabel("Scattering texture width 2* (Nu):", textureGroup);
	textureLayout->addWidget(labelMU_NU, 7, 0, 1, 2);
	inputMU_NU = new QLineEdit(QString::number(widget->atmosphere->MU_NU), textureGroup);
	textureLayout->addWidget(inputMU_NU, 7, 2, 1, 2);
	
	QLabel* labelTRANSMITTANCE_SAMPLES = new QLabel("Transmittance integral samples:", textureGroup);
	textureLayout->addWidget(labelTRANSMITTANCE_SAMPLES, 8, 0, 1, 2);
	inputTRANSMITTANCE_SAMPLES = new QLineEdit(QString::number(widget->atmosphere->TRANSMITTANCE_SAMPLES), textureGroup);
	textureLayout->addWidget(inputTRANSMITTANCE_SAMPLES, 8, 2, 1, 2);
	
	QLabel* labelSCATTERING_SAMPLES = new QLabel("Scattering integral samples:", textureGroup);
	textureLayout->addWidget(labelSCATTERING_SAMPLES, 9, 0, 1, 2);
	inputSCATTERING_SAMPLES = new QLineEdit(QString::number(widget->atmosphere->SCATTERING_SAMPLES), textureGroup);
	textureLayout->addWidget(inputSCATTERING_SAMPLES, 9, 2, 1, 2);
	
	QLabel* labelSCATTERING_SPHERICAL_SAMPLES = new QLabel("Scattering integral (x*2x) samples:", textureGroup);
	textureLayout->addWidget(labelSCATTERING_SPHERICAL_SAMPLES, 10, 0, 1, 2);
	inputSCATTERING_SPHERICAL_SAMPLES = new QLineEdit(QString::number(widget->atmosphere->SCATTERING_SPHERICAL_SAMPLES), textureGroup);
	textureLayout->addWidget(inputSCATTERING_SPHERICAL_SAMPLES, 10, 2, 1, 2);
	
	QLabel* labelIRRADIANCE_SPHERICAL_SAMPLES = new QLabel("Irradiance integral (x/2*2x) samples:", textureGroup);
	textureLayout->addWidget(labelIRRADIANCE_SPHERICAL_SAMPLES, 11, 0, 1, 2);
	inputIRRADIANCE_SPHERICAL_SAMPLES = new QLineEdit(QString::number(widget->atmosphere->IRRADIANCE_SPHERICAL_SAMPLES), textureGroup);
	textureLayout->addWidget(inputIRRADIANCE_SPHERICAL_SAMPLES, 11, 2, 1, 2);
	
	QLabel* labelORDER_COUNT = new QLabel("Multiple scattering order count:", textureGroup);
	textureLayout->addWidget(labelORDER_COUNT, 12, 0, 1, 2);
	inputORDER_COUNT = new QLineEdit(QString::number(widget->atmosphere->ORDER_COUNT), textureGroup);
	textureLayout->addWidget(inputORDER_COUNT, 12, 2, 1, 2);

	QLabel* labelRg = new QLabel("Planet radius:", textureGroup);
	textureLayout->addWidget(labelRg, 13, 0, 1, 2);
	inputRg = new QLineEdit(QString::number(widget->atmosphere->Rg), textureGroup);
	textureLayout->addWidget(inputRg, 13, 2, 1, 2);

	QLabel* labelRt = new QLabel("Atmosphere radius:", textureGroup);
	textureLayout->addWidget(labelRt, 14, 0, 1, 2);
	inputRt = new QLineEdit(QString::number(widget->atmosphere->Rt), textureGroup);
	textureLayout->addWidget(inputRt, 14, 2, 1, 2);

	QLabel* labelsunAngularRadius = new QLabel("Sun angular radius:", textureGroup);
	textureLayout->addWidget(labelsunAngularRadius, 15, 0, 1, 2);
	inputsunAngularRadius = new QLineEdit(QString::number(widget->atmosphere->sunAngularRadius), textureGroup);
	textureLayout->addWidget(inputsunAngularRadius, 15, 2, 1, 2);

	QLabel* labelsolarIrradiance = new QLabel("Solar irradiance:", textureGroup);
	textureLayout->addWidget(labelsolarIrradiance, 16, 0, 1, 2);
	inputsolarIrradiance = new QLineEdit(QString::number(widget->atmosphere->solarIrradiance), textureGroup);
	textureLayout->addWidget(inputsolarIrradiance, 16, 2, 1, 2);
	
	QLabel* labelmuSmin = new QLabel("Minimum MuS for scattering:", textureGroup);
	textureLayout->addWidget(labelmuSmin, 17, 0, 1, 2);
	inputmuSmin = new QLineEdit(QString::number(widget->atmosphere->muSmin), textureGroup);
	textureLayout->addWidget(inputmuSmin, 17, 2, 1, 2);

	QLabel* labelrH = new QLabel("Rayleigh scattering height scale:", textureGroup);
	textureLayout->addWidget(labelrH, 18, 0, 1, 2);
	inputrH = new QLineEdit(QString::number(widget->atmosphere->rH), textureGroup);
	textureLayout->addWidget(inputrH, 18, 2, 1, 2);

	QLabel* labelmH = new QLabel("Mie scattering height scale:", textureGroup);
	textureLayout->addWidget(labelmH, 19, 0, 1, 2);
	inputmH = new QLineEdit(QString::number(widget->atmosphere->mH), textureGroup);
	textureLayout->addWidget(inputmH, 19, 2, 1, 2);

	QLabel* labelMieG = new QLabel("Mie asymmetry factor g [-1;1]:", textureGroup);
	textureLayout->addWidget(labelMieG, 20, 0, 1, 2);
	inputMieG = new QLineEdit(QString::number(widget->atmosphere->g), textureGroup);
	textureLayout->addWidget(inputMieG, 20, 2, 1, 2);
	
	QLabel* labelRBeta = new QLabel("Rayleigh scattering beta:", textureGroup);
	QLabel* labelRBetaR = new QLabel("R:", textureGroup);
	inputRBetaR = new QLineEdit(QString::number(widget->atmosphere->rBeta.x()), textureGroup);
	QLabel* labelRBetaG = new QLabel("G:", textureGroup);
	inputRBetaG = new QLineEdit(QString::number(widget->atmosphere->rBeta.y()), textureGroup);
	QLabel* labelRBetaB = new QLabel("B:", textureGroup);
	inputRBetaB = new QLineEdit(QString::number(widget->atmosphere->rBeta.z()), textureGroup);
	textureLayout->addWidget(labelRBeta, 21, 0, 1, 4);
	textureLayout->addWidget(labelRBetaR, 22, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(labelRBetaG, 23, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(labelRBetaB, 24, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(inputRBetaR, 22, 1, 1, 2);
	textureLayout->addWidget(inputRBetaG, 23, 1, 1, 2);
	textureLayout->addWidget(inputRBetaB, 24, 1, 1, 2);

	QLabel* labelMBeta = new QLabel("Mie scattering beta:", textureGroup);
	QLabel* labelMBetaR = new QLabel("R:", textureGroup);
	inputMBetaR = new QLineEdit(QString::number(widget->atmosphere->mBeta.x()), textureGroup);
	QLabel* labelMBetaG = new QLabel("G:", textureGroup);
	inputMBetaG = new QLineEdit(QString::number(widget->atmosphere->mBeta.y()), textureGroup);
	QLabel* labelMBetaB = new QLabel("B:", textureGroup);
	inputMBetaB = new QLineEdit(QString::number(widget->atmosphere->mBeta.z()), textureGroup);
	textureLayout->addWidget(labelMBeta, 25, 0, 1, 4);
	textureLayout->addWidget(labelMBetaR, 26, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(labelMBetaG, 27, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(labelMBetaB, 28, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(inputMBetaR, 26, 1, 1, 2);
	textureLayout->addWidget(inputMBetaG, 27, 1, 1, 2);
	textureLayout->addWidget(inputMBetaB, 28, 1, 1, 2);

	QLabel* labelSurfaceAlbedo = new QLabel("Surface albedo:", textureGroup);
	QLabel* labelSurfaceAlbedoR = new QLabel("R:", textureGroup);
	inputSurfaceAlbedoR = new QLineEdit(QString::number(widget->atmosphere->surfaceAlbedo.x()), textureGroup);
	QLabel* labelSurfaceAlbedoG = new QLabel("G:", textureGroup);
	inputSurfaceAlbedoG = new QLineEdit(QString::number(widget->atmosphere->surfaceAlbedo.y()), textureGroup);
	QLabel* labelSurfaceAlbedoB = new QLabel("B:", textureGroup);
	inputSurfaceAlbedoB = new QLineEdit(QString::number(widget->atmosphere->surfaceAlbedo.z()), textureGroup);
	textureLayout->addWidget(labelSurfaceAlbedo, 29, 0, 1, 4);
	textureLayout->addWidget(labelSurfaceAlbedoR, 30, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(labelSurfaceAlbedoG, 31, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(labelSurfaceAlbedoB, 32, 0, 1, 1, Qt::AlignJustify);
	textureLayout->addWidget(inputSurfaceAlbedoR, 30, 1, 1, 2);
	textureLayout->addWidget(inputSurfaceAlbedoG, 31, 1, 1, 2);
	textureLayout->addWidget(inputSurfaceAlbedoB, 32, 1, 1, 2);
	
	QPushButton* recompute = new QPushButton(tr("Recompute"), scrollWidget);
	recompute->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	recompute->setStyleSheet("QPushButton { font-size: 11pt; }");
	textureLayout->addWidget(recompute, 33, 0, 1, 0, Qt::AlignJustify);
	connect(recompute, &QPushButton::released, this, &Window::recompute);

	QGroupBox* renderGroup = new QGroupBox(tr("Render Parameters"), scrollWidget);
	QGridLayout* renderLayout = new QGridLayout(renderGroup);
	renderGroup->setLayout(renderLayout);
	scrollLayout->addWidget(renderGroup);

	QLabel* labelExposure = new QLabel("Exposure:                                        ", renderGroup);
	renderLayout->addWidget(labelExposure, 0, 0, 1, 2);
	inputExposure = new QLineEdit(QString::number(widget->atmosphere->exposure), renderGroup);
	renderLayout->addWidget(inputExposure, 0, 2, 1, 2);

	QLabel* labelWhitePoint = new QLabel("White point:", renderGroup);
	QLabel* labelWhitePointR = new QLabel("R:", renderGroup);
	inputWhitePointR = new QLineEdit(QString::number(widget->atmosphere->whitePoint.x()), renderGroup);
	QLabel* labelWhitePointG = new QLabel("G:", renderGroup);
	inputWhitePointG = new QLineEdit(QString::number(widget->atmosphere->whitePoint.y()), renderGroup);
	QLabel* labelWhitePointB = new QLabel("B:", renderGroup);
	inputWhitePointB = new QLineEdit(QString::number(widget->atmosphere->whitePoint.z()), renderGroup);
	renderLayout->addWidget(labelWhitePoint, 1, 0, 1, 4);
	renderLayout->addWidget(labelWhitePointR, 2, 0, 1, 1, Qt::AlignJustify);
	renderLayout->addWidget(labelWhitePointG, 3, 0, 1, 1, Qt::AlignJustify);
	renderLayout->addWidget(labelWhitePointB, 4, 0, 1, 1, Qt::AlignJustify);
	renderLayout->addWidget(inputWhitePointR, 2, 1, 1, 2);
	renderLayout->addWidget(inputWhitePointG, 3, 1, 1, 2);
	renderLayout->addWidget(inputWhitePointB, 4, 1, 1, 2);

	QLabel* gammaFormula = new QLabel("Gamma formula:\n1 - exp(-colour * exposure / whitePoint)^(1 / 2.2)", renderGroup);
	renderLayout->addWidget(gammaFormula, 5, 0, 1, 0, Qt::AlignJustify);

	QPushButton* apply = new QPushButton(tr("Apply"), scrollWidget);
	apply->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	apply->setStyleSheet("QPushButton { font-size: 11pt; }");
	renderLayout->addWidget(apply, 6, 0, 1, 0, Qt::AlignJustify);
	connect(apply, &QPushButton::released, this, &Window::apply);
	
	scrollArea->widget()->adjustSize();
	qApp->processEvents();
	
	//STATUS BAR
	statusBar()->addPermanentWidget(optionsTool);
	statusBar()->addPermanentWidget(outputTool);
	QLabel* spacer = new QLabel();
	statusBar()->addPermanentWidget(spacer, 1);
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
		if (widget->getFocus())
			widget->changeFocus();
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

void Window::recompute()
{
	widget->atmosphere->Rg = inputRg->text().toFloat();
	widget->atmosphere->Rt = inputRt->text().toFloat();
	widget->atmosphere->mH = inputmH->text().toFloat();
	widget->atmosphere->rH = inputrH->text().toFloat();
	widget->atmosphere->rBeta.setX(inputRBetaR->text().toFloat());
	widget->atmosphere->rBeta.setY(inputRBetaG->text().toFloat());
	widget->atmosphere->rBeta.setZ(inputRBetaB->text().toFloat());
	widget->atmosphere->mBeta.setX(inputMBetaR->text().toFloat());
	widget->atmosphere->mBeta.setY(inputMBetaG->text().toFloat());
	widget->atmosphere->mBeta.setZ(inputMBetaB->text().toFloat());
	widget->atmosphere->mBetaExt = widget->atmosphere->mBeta / 0.9f;
	widget->atmosphere->g = inputMieG->text().toFloat();
	widget->atmosphere->surfaceAlbedo.setX(inputSurfaceAlbedoR->text().toFloat());
	widget->atmosphere->surfaceAlbedo.setY(inputSurfaceAlbedoG->text().toFloat());
	widget->atmosphere->surfaceAlbedo.setZ(inputSurfaceAlbedoB->text().toFloat());
	widget->atmosphere->sunAngularRadius = inputsunAngularRadius->text().toFloat();
	widget->atmosphere->muSmin = inputmuSmin->text().toFloat();
	widget->atmosphere->solarIrradiance = inputsolarIrradiance->text().toFloat();

	widget->atmosphere->TRANSMITTANCE_SAMPLES = inputTRANSMITTANCE_SAMPLES->text().toInt();
	widget->atmosphere->SCATTERING_SAMPLES = inputSCATTERING_SAMPLES->text().toInt();
	widget->atmosphere->SCATTERING_SPHERICAL_SAMPLES = inputSCATTERING_SPHERICAL_SAMPLES->text().toInt();
	widget->atmosphere->IRRADIANCE_SPHERICAL_SAMPLES = inputIRRADIANCE_SPHERICAL_SAMPLES->text().toInt();

	widget->atmosphere->T_W = inputT_W->text().toInt();
	widget->atmosphere->T_H = inputT_H->text().toInt();
	widget->atmosphere->E_W = inputE_W->text().toInt();
	widget->atmosphere->E_H = inputE_H->text().toInt();
	widget->atmosphere->MU_R = inputMU_R->text().toInt();
	widget->atmosphere->MU_MU = inputMU_MU->text().toInt();
	widget->atmosphere->MU_MU_S = inputMU_MU_S->text().toInt();
	widget->atmosphere->MU_NU = inputMU_NU->text().toInt();
	widget->atmosphere->ORDER_COUNT = inputORDER_COUNT->text().toInt();

	widget->makeCurrent();
	widget->quadRenderer->deletePrecomputedTextures();
	widget->quadRenderer->rebuildFramebuffer(width(), height());
	widget->quadRenderer->allocateTextures();
	widget->quadRenderer->precompute();
	widget->doneCurrent();
}

void Window::apply()
{
	widget->atmosphere->exposure = inputExposure->text().toFloat();
	widget->atmosphere->whitePoint.setX(inputWhitePointR->text().toFloat());
	widget->atmosphere->whitePoint.setY(inputWhitePointG->text().toFloat());
	widget->atmosphere->whitePoint.setZ(inputWhitePointB->text().toFloat());
}