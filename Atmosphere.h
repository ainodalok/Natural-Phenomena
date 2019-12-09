#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#include <QVector2D>
#include <QVector3D>

class Atmosphere
{
public:
	Atmosphere();

	//Simple scattering model on CPU
	//QVector3D computeZeroScatteringLightRealtime(QVector3D orig, QVector3D dir);
	//Paper scattering model on CPU
	//QVector3D computeIncidentLight(QVector3D x, QVector3D v);
	void updateS(float dtime);
	float getRg();

	//Precomputation variables initialized to earth values
	QVector3D s = QVector3D(0.0f, 0.35f, 1.0f).normalized();
	float Rg = 6360e3f; //6360e3f;
	float Rt = 6360e3f * 6360e3f; //6420e3f;
	float rH = 8000.0f;
	float mH = 1200.0f;
	QVector3D rBeta = QVector3D(5.8e-6f, 13.5e-6f, 33.1e-6f);
	QVector3D mBeta = QVector3D(2.21e-5f, 2.21e-5f, 2.21e-5f);
	QVector3D mBetaExt = mBeta / 0.9;
	float g = 0.73f;
	//float sunIntensity = 20.0f;
	float sunAngularRadius = 0.0046251225f;

private:
	//CPU FUNCTIONS
	//bool solveQuadratic(float a, float b, float c, float& x1, float& x2);
	//Checks if sphere at 0,0,0 is intersected and computes t0/1 values (intersection point = orig + t0/1 * dir)
	//bool raySphereIntersect(QVector3D orig, QVector3D dir, float radius, float& t0, float& t1);
	//float opticalDepth(float H, float r, float mu, float d);
	//QVector3D transmittance(float r, float mu, float d);
	//QVector3D directSunlight(QVector3D v, float rBound, float dBound, float mu);
	//QVector3D toneMapping(QVector3D skyColour);
	
	//GPU FUNCTIONS

};

#endif
