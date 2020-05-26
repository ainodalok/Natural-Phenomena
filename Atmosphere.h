#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include <QQuaternion>

class Atmosphere
{
public:
	Atmosphere() = default;
	
	void updateS(float modifier);

	//Precomputation variables initialized to earth values
	QVector3D s = QVector3D(0.0f, 0.0625f, -1.0f).normalized();

	float Rg = 6360e3f;
	float Rt = 6420e3f;
	float mH = 1200.0f;
	float rH = 8000.0f;
	QVector3D rBeta = QVector3D(5.8e-6f, 13.5e-6f, 33.1e-6f);
	QVector3D mBeta = QVector3D(2.21e-5f, 2.21e-5f, 2.21e-5f);
	QVector3D mBetaExt = mBeta / 0.9f;
	float g = 0.73f;
	QVector3D surfaceAlbedo = QVector3D(0.0f, 0.0f, 0.04f);
	//float sunIntensity = 20.0f;
	float sunAngularRadius = 0.0046251225f;
	//The smallest possible angle for which scattering precomputation yields negligible results
	//Necessary so that more memory could be allocated for result precision
	//by default earth value is set
	float muSmin = -0.2f;
	float solarIrradiance = 1.5f;

	int TRANSMITTANCE_SAMPLES = 500;
	int SCATTERING_SAMPLES = 50;
	int SCATTERING_SPHERICAL_SAMPLES = 16;
	int IRRADIANCE_SPHERICAL_SAMPLES = 32;

	int T_W = 64; //R
	int T_H = 256; //Mu
	int E_W = 16; //R
	int E_H = 64; //MuS
	int MU_R = 32;
	int MU_MU = 128;
	int MU_MU_S = 32;
	int MU_NU = 8;
	int ORDER_COUNT = 4;

	int CLOUD_W = 1280;
	int CLOUD_H = 1280;
	int CLOUD_D = 128;

	float exposure = 10.0f;
	float cloudOpacity = 1.0f;
	QVector3D whitePoint = QVector3D(2.8f, 2.175f, 1.875f);
};
#endif
