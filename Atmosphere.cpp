#include "Atmosphere.h"

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <QtWidgets>
#include <QtDebug>

Atmosphere::Atmosphere()
{
	//Define sky properties
	//s = QVector3D(0.0f, 0.35f, 1.0f);
	//s.normalize();
	//Rg = 6360e3f;
	//Rt = 6420e3f;
	//Scale heights
	//rH = 8000.0f;
	//mH = 1200.0f;
	//Scattering coefficients
	//Paper
	//rBeta = QVector3D(5.8e-6f, 13.5e-6f, 33.1e-6f);
	//Dice
	//rBeta = new QVector3D(6.55e-6f, 1.73e-5f, 2.3e-5f);
	//As stated under figure 7, not under figure 6. These values look closer to the ones found in examples of other papers
	//mBeta = QVector3D(2.21e-5f, 2.21e-5f, 2.21e-5f);
	//Extinction
	//mBetaExt = mBeta / 0.9;
	//Mean cosine, describes that light is mainly scattered along the light forward direction
	//g = 0.73f;
	//Intensity of the sunlight
	//sunIntensity = 20.0f;
	//Angular radius of the sun
	//sunAngularRadius = 0.0046251225f;
	//sunAngularRadius = 0.017;
}
//
//bool Atmosphere::solveQuadratic(float a, float b, float c, float& x1, float& x2)
//{
//	if (b == 0)
//	{
//		//No roots
//		if ((a == 0) || ((-c / a) < 0)) return false;
//		x1 = -sqrtf(-c / a); x2 = sqrtf(-c / a);
//		return true;
//	} else if (a == 0)
//	{
//		x1 = c / b; x2 = x1;
//		return true;
//	}
//		
//	float discr = b * b - 4 * a * c;
//
//	//No roots.
//	if (discr < 0) return false;
//
//	//More stable version proposed by scratchapixel.com
//	//root of discriminant will never cancel out b or produce big round off error due to close values
//	float q = (b < 0.f) ? -0.5f * (b - sqrtf(discr)) : -0.5f * (b + sqrtf(discr));
//	//Standard quadratic solution
//	x1 = q / a;
//	// 2c / (-b +- sqrt(b^2 - 4ac)) = (-b -+ sqrt(b^2 - 4ac)) / 2a
//	x2 = c / q;
//
//	return true;
//}
//
//bool Atmosphere::raySphereIntersect(QVector3D orig, QVector3D dir, float radius, float& t0, float& t1)
//{
//	//They ray dir is normalized so A = 1 
//	float A = QVector3D::dotProduct(dir, dir);
//	float B = 2 * QVector3D::dotProduct(dir, orig);
//	float C = QVector3D::dotProduct(orig, orig) - radius * radius;
//
//	if ((!solveQuadratic(A, B, C, t0, t1)) || ((t0 < 0) && (t1 < 0)))
//		return false;
//
//	if (t0 > t1)
//		std::swap(t0, t1);
//
//	return true;
//}

void Atmosphere::updateS(float dtime)
{
	//Rotate sun direction by 2 degrees around x axis
	s = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, dtime * 0.05).rotatedVector(s);
	s.normalize();
}

float Atmosphere::getRg()
{
	return Rg;
}

////HDR
//QVector3D Atmosphere::toneMapping(QVector3D skyColour)
//{
//	float r = skyColour.x() < 1.413f ? pow(skyColour.x() * 0.38317f, 1.0f / 2.2f) :
//		1.0f - exp(-skyColour.x());
//	float g = skyColour.y() < 1.413f ? pow(skyColour.y() * 0.38317f, 1.0f / 2.2f) :
//		1.0f - exp(-skyColour.y());
//	float b = skyColour.z() < 1.413f ? pow(skyColour.z() * 0.38317f, 1.0f / 2.2f) :
//		1.0f - exp(-skyColour.z());
//
//	return QVector3D(r, g, b);
//}
//
//QVector3D Atmosphere::computeZeroScatteringLightRealtime(QVector3D orig, QVector3D dir)
//{
//	//First and second intersection distances
//	float dist0 = -1.0f, dist1 = -1.0f;
//	//If ray doesn't intersect with the atmosphere or the only intersection is behind the camera return black light
//	if (!raySphereIntersect(orig, dir, Rt, dist0, dist1) || dist1 < 0)
//		return QVector3D(0.0f, 0.0f, 0.0f);
//	//First intersection can't be behind, should be at least at eye position
//	dist0 = std::max(dist0, 0.0f);
//	//Length of the segment on the view ray to integrate view ray Transmittance
//	float segmentLength = (dist1 - dist0) / TRANSMITTANCE_SAMPLES;
//	//Start from either first intersection if outside of atmosphere or camera position
//	float distCurrent = dist0;
//
//	//Rayleigh light contribution
//	QVector3D rSum(0.0f, 0.0f, 0.0f);
//	//Mie light contribution
//	QVector3D mSum(0.0f, 0.0f, 0.0f);
//	//Accumulated exponent function of negative height by height scale division for transmittance integral
//	//Rayleigh
//	float rOpticalDepthAcc = 0.0f;
//	//Mie
//	float mOpticalDepthAcc = 0.0f;
//
//	//Greek letter mu, cosine between view direction and sun light direction, required for phase function
//	float mu = QVector3D::dotProduct(dir, s) / (dir.length() * s.length());
//	//Describes how much light is scattered towards camera
//	//Rayleigh
//	float rPhase = 3.0f / (16.0f * M_PI) * (1 + mu * mu);
//	//Mie
//	float mPhase = 3.0f / (8.0f * M_PI) * ((1.0f - g * g) * (1.0f + mu * mu)) / ((2.0f + g * g) * pow(1.0f + g * g - 2.0f * g * mu, 1.5f));
//
//	//Integrate Transmittance over view vector
//	for (int i = 0; i < TRANSMITTANCE_SAMPLES; i++)
//	{
//		//Sample positions are taken in the middle between sample points
//		QVector3D samplePos = orig + (distCurrent + segmentLength * 0.5f) * dir;
//		//Altitude of the sample point in relation to the ground
//		float height = samplePos.length() - Rg;
//		if (height < 0) break;
//		//Exponent function of negative height by height scale division for view ray transmittance integral
//		//Rayleigh
//		float rOpticalDepth = exp(-height / rH) * segmentLength;
//		rOpticalDepthAcc += rOpticalDepth;
//		//Mie
//		float mOpticalDepth = exp(-height / mH) * segmentLength;
//		mOpticalDepthAcc += mOpticalDepth;
//
//		//Distance to atmosphere from sample point position by the light vector
//		float dist0Light, dist1Light;
//		//Get intersection of light ray with atmosphere. Since sample position is always inside of the atmosphere
//		//there is only one intersection of the ray and atmosphere, the second one
//		raySphereIntersect(samplePos, s, Rt, dist0Light, dist1Light);
//		//Length of the segment on the light ray to integrate light ray Transmittance
//		float segmentLengthLight = dist1Light / TRANSMITTANCE_SAMPLES;
//		//Start from sample point position
//		float distCurrentLight = 0;
//		//Exponent function of negative height by height scale division for light ray transmittance integral
//		//Rayleigh
//		double rOpticalDepthLightAcc = 0;
//		//Mie
//		double mOpticalDepthLightAcc = 0;
//		int j;
//		for (j = 0; j < TRANSMITTANCE_SAMPLES; ++j)
//		{
//			//Sample positions are taken in the middle between sample points
//			QVector3D samplePosLight = samplePos + (distCurrentLight + segmentLengthLight * 0.5f) * s;
//			//Altitude of the sample point in relation to the ground
//			float heightLight = samplePosLight.length() - Rg;
//			//If altitude of a sample point is less than 0
//			if (heightLight < 0) break;
//			//Exponent function accumulation
//			//Rayleigh
//			rOpticalDepthLightAcc += exp(-heightLight / rH) * segmentLengthLight;
//			//Mie
//			mOpticalDepthLightAcc += exp(-heightLight / mH) * segmentLengthLight;
//			//Advance forward light vector
//			distCurrentLight += segmentLengthLight;
//		}
//		if (j == TRANSMITTANCE_SAMPLES)
//		{
//			//Transmittance of both types of scattering of both light and view rays first without exponential function,
//			//which is calculated later so that there would not be four exponential calls
//			QVector3D transmittanceWithoutExp = rBeta * (rOpticalDepthAcc + rOpticalDepthLightAcc) + mBeta * 1.1f * (mOpticalDepth + mOpticalDepthLightAcc);
//			QVector3D transmittanceWithExp(exp(-transmittanceWithoutExp.x()), exp(-transmittanceWithoutExp.y()), exp(-transmittanceWithoutExp.z()));
//			//Transmittance part by what is left of the integral part of the sky colour equation except of scattering coefficient beta, because it is a constant in this case
//			//Rayleigh
//			rSum += transmittanceWithExp * rOpticalDepth;
//			//Mie
//			mSum += transmittanceWithExp * mOpticalDepth;
//		}
//		//Advance forward view vector
//		distCurrent += segmentLength;
//	}
//
//	//Beta by sum are integral part of sky colour equation
//	return toneMapping((rSum * rBeta * rPhase + mSum * mBeta * mPhase) * sunIntensity);
//	//return (rSum * (*rBeta) * rPhase + mSum * (*mBeta) * mPhase) * sunIntensity;
//}
//
//QVector3D Atmosphere::computeIncidentLight(QVector3D x, QVector3D v)
//{
//	//Black
//	QVector3D L(0.0, 0.0, 0.0);
//	float r = x.length();
//	float mu = QVector3D::dotProduct(x, v) / r;
//	float muS = QVector3D::dotProduct(x, s) / r;
//	float earth0, earth1;
//	float atmos0, atmos1;
//
//	bool intersectsAtmos = false;
//	bool intersectsGround = false;
//
//	if (intersectsAtmos = raySphereIntersect(x, v, Rt, atmos0, atmos1))
//	{
//		intersectsGround = raySphereIntersect(x, v, Rg, earth0, earth1);
//		//Move height of the camera to the atmosphere boundary if it is outside of the atmosphere
//		float rBound = std::min(r, Rt);
//		//Distance of the transmittance ray is limited by earth and atmosphere boundary constraints
//		float dBound = r > Rt ?
//			(intersectsGround ? (earth0 - atmos0) : (atmos1 - atmos0)) :
//			(intersectsGround ? earth0 : atmos1);
//
//		//If doesn't intersect earth, check sun
//		if (!intersectsGround)
//		{
//			//L[0] direct
//			L += directSunlight(v, rBound, dBound, mu);
//
//			//Inscattering
//			//L += inscatteredLight(v);
//		}
//		else
//		{
//			//Ground reflection R[L]
//			L += transmittance(rBound, mu, dBound);
//		}
//	}
//
//	return QVector3D(L);
//}
//
//float Atmosphere::opticalDepth(float H, float r, float mu, float d)
//{
//	float result = 0.0;
//	float segmentLength = d / float(TRANSMITTANCE_SAMPLES);
//	for (int i = 0; i <= TRANSMITTANCE_SAMPLES; i++) 
//	{
//		float travelledDistance = i * segmentLength;
//		//Square root gives length (norm) of segment (vector) from center to point at distance travelDistance along vector r*mu
//		float h = sqrt(r * r + travelledDistance * travelledDistance + 2.0f * travelledDistance * r * mu) - Rg;
//		//exp is always [0,1] since h and H are both positive, hence exp argument is negative
//		float currentOpticalDepthByTravelledDistance = exp(-h / H) * travelledDistance;
//
//		//Trapezoidal rule as in new implementation, old implementation had more variables, saved previous values to process each trapezoid separately
//		if (i == 0 || i == TRANSMITTANCE_SAMPLES)
//			currentOpticalDepthByTravelledDistance = currentOpticalDepthByTravelledDistance * 0.5;
//
//		result += currentOpticalDepthByTravelledDistance;
//	}
//	return result;
//}
//
//QVector3D Atmosphere::transmittance(float r, float mu, float d)
//{
//	QVector3D transmittanceWithoutExp = -rBeta * opticalDepth(rH, r, mu, d) - mBetaExt * opticalDepth(mH, r, mu, d);
//	return QVector3D(exp(transmittanceWithoutExp.x()), exp(transmittanceWithoutExp.y()), exp(transmittanceWithoutExp.z()));
//}
//
//QVector3D Atmosphere::directSunlight(QVector3D v, float rBound, float dBound, float mu)
//{
//	if (QVector3D::dotProduct(v, s) > cos(sunAngularRadius))
//		return QVector3D(1.0, 1.0, 1.0) * transmittance(rBound, mu, dBound) *
//			//Smooth out the edges of the solar disc
//			pow((QVector3D::dotProduct(v, s) - cos(sunAngularRadius)) / (1.0f - cos(sunAngularRadius)), 3.0) *
//			// *1.5 / (M_... constant solar radiance, not physically accurate but does job better than old implementation
//			1.5f / (M_PI * sunAngularRadius * sunAngularRadius);
//	else
//		return QVector3D(0.0, 0.0, 0.0);
//}

