#include "QuadRenderer.h"

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <cstring>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QImage>
#include <QGLWidget>
#include <QtWidgets>
#include <QDebug>
#include <iostream> 
#include <algorithm>
#include <cmath>

QuadRenderer::QuadRenderer(int width, int height)
{
	//Define sky texture
	this->width = width;
	this->height = height;

	skyData = new GLubyte[width * height * 3];
	memset(skyData, 255, width * height * 3);

	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &skyQuadTextureId);
	glBindTexture(GL_TEXTURE_2D, skyQuadTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)skyData);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Define sky mesh
	vertices.push_back(QVector3D(-1, -1, 0.9999999));
	vertices.push_back(QVector3D(1, -1, 0.9999999));
	vertices.push_back(QVector3D(1, 1, 0.9999999));
	vertices.push_back(QVector3D(-1, 1, 0.9999999));

	texVertices.push_back(QVector2D(0.0f, 0.0f));
	texVertices.push_back(QVector2D(1.0f, 0.0f));
	texVertices.push_back(QVector2D(1.0f, 1.0f));
	texVertices.push_back(QVector2D(0.0f, 1.0f));

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	//Define sky properties
	sunDir = new QVector3D(0.0f, -0.05f, -1.0f);
	sunDir->normalize();
	earthRadius = 6360e3f;
	atmosphereRadius = 6420e3f;
	//Scale heights
	rH = 7994.0f;
	mH = 1200.0f;
	//Scattering coefficients
	rBeta = new QVector3D(5.8e-6f, 13.5e-6f, 33.1e-6f);
	mBeta = new QVector3D(21e-6f, 21e-6f, 21e-6f);
	//Transmittance sample amount
	viewTSamples = 16;
	lightTSamples = 2;
	//Mean cosine, describes that light is mainly scattered along the light forward direction
	g = 0.76f;
	//Intensity of the sunlight
	sunIntensity = 20.0f;
}

QuadRenderer::~QuadRenderer()
{
	clearTexture();

	glDeleteTextures(1, &skyQuadTextureId);
}

void QuadRenderer::render()
{
	glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, skyQuadTextureId);

	glTexCoordPointer(2, GL_FLOAT, 0, &(texVertices[0]));
	glVertexPointer(3, GL_FLOAT, 0, &(vertices[0]));
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &(indices[0]));

	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

void QuadRenderer::clearTexture()
{
	delete[] skyData;
	skyData = 0;
}

void QuadRenderer::updateTexture(int width, int height, float angleX, float angleY, float fovX)
{
	bool sizeChanged = false;
	//std::cout << "Here it comes\n";
	glBindTexture(GL_TEXTURE_2D, skyQuadTextureId);
	//If new dimensions, allocate new texture, otherwise update
	if (this->width != width || this->height != height)
	{
		//Deallocate bitmap data
		clearTexture();

		//Allocate new array of colours with correct new width and height
		skyData = new GLubyte[width * height * 3];
		//Set it to white
		memset(skyData, 255, width * height * 3);

		//Save new height and width
		this->height = height;
		this->width = width;

		//Update sizeChanged bool
		sizeChanged = true;
	}


	//Rotate sun direction by 2 degrees around x axis
	*sunDir = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 2).rotatedVector(*sunDir);
	//View Rotation quaternion
	//Camera rotation angle is inverted since quaternion rotation of view vector is ccw, but angle increase turns camera look vector right and up(cw around X and Y axis)
	QQuaternion viewRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -angleY) * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, -angleX);
	
	//Actual update for each pixel
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			//Calculate ray direction
			// x E (tan(-fovX/2); tan(fovX/2))
			//i / width			// turns into [0;1] range
			//2 * i / width		// turns into [0;2] range
			//2 * i / width - 1 // turns into [-1;1] range
			//tanf(fovX / 180.0f * M_PI / 2.0f) = 1 if fovX = 90
			float x = (2.0f * i / static_cast<float>(width) - 1.0f); // * tanf(fovX / 180.0f * M_PI / 2.0f)
			// y E (-tan(fovY/2); tan(fovY/2))
			float y = (2.0f * j / static_cast<float>(height) - 1.0f) / 
			//Divide by aspect ratio to convert x fov resulting values to y fov
					(static_cast<float>(width) / static_cast<float>(height)); // * tanf(fovX / 180.0f * M_PI / 2.0f)
			//By default camera looks in the direction of +z, hence x is inverted (as it increases to the left relative from camera look direction)
			QVector3D rayDir(-x, y, 1.0);
			//Rotate by quaternion
			rayDir = viewRot.rotatedVector(rayDir);
			rayDir.normalize();

			//Compute light one meter from earth
			QVector3D lightColour = computeIncidentLight(QVector3D(0.0f, earthRadius + 1.0f, 0.0f), rayDir);
			//qDebug() << lightColour;
			
			//Tone mapping by scratchpixel.com
			float r = lightColour.x() < 1.413f ? pow(lightColour.x() * 0.38317f, 1.0f / 2.2f) :
												1.0f - exp(-lightColour.x());
			float g = lightColour.y() < 1.413f ? pow(lightColour.y() * 0.38317f, 1.0f / 2.2f) :
												1.0f - exp(-lightColour.y());
			float b = lightColour.z() < 1.413f ? pow(lightColour.z() * 0.38317f, 1.0f / 2.2f) :
												1.0f - exp(-lightColour.z());

			//skyData[i * 3 + j * width * 3] = static_cast<int>(std::clamp(r, 0.0f, 1.0f) * 255);
			//skyData[i * 3 + j * width * 3 + 1] = static_cast<int>(std::clamp(g, 0.0f, 1.0f) * 255);
			//skyData[i * 3 + j * width * 3 + 2] = static_cast<int>(std::clamp(b, 0.0f, 1.0f) * 255);

			skyData[i * 3 + j * width * 3] = static_cast<int>(std::max(0.0f, std::min(1.0f, r)) * 255);
			skyData[i * 3 + j * width * 3 + 1] = static_cast<int>(std::max(0.0f, std::min(1.0f, g)) * 255);
			skyData[i * 3 + j * width * 3 + 2] = static_cast<int>(std::max(0.0f, std::min(1.0f, b)) * 255);

			//std::cout << lightColour.x() << ", "
				//<< lightColour.y() << ", "
				//<< lightColour.z() << std::endl;

			/*
			// Set colour
			skyData[i * 3 + j * width * 3] = static_cast<int>(std::clamp(lightColour.x(), 0.0f, 1.0f) * 255);
			skyData[i * 3 + j * width * 3 + 1] = static_cast<int>(std::clamp(lightColour.y(), 0.0f, 1.0f) * 255);
			skyData[i * 3 + j * width * 3 + 2] = static_cast<int>(std::clamp(lightColour.z(), 0.0f, 1.0f) * 255);
			*/
		}
	}

	//Texture test
	/*float normalizedAngleX = (angleX + 90) / 180.0f;
	for (int i = 0; i < width*height * 3 ; i += 3)
	{
		skyData[i] = normalizedAngleX * 81 + (1.0f - normalizedAngleX) * 109;
		skyData[i + 1] = normalizedAngleX * 198 + (1.0f - normalizedAngleX) * 31;
		skyData[i + 2] = normalizedAngleX * 31 + (1.0f - normalizedAngleX) * 198;
	}*/

	//
	if (sizeChanged)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)skyData);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)skyData);

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool QuadRenderer::solveQuadratic(float a, float b, float c, float& x1, float& x2)
{
	//Handle special case where the two vector ray.dir and V are perpendicular,
	// with V = ray.orig - sphere.centre
	if (b == 0) 
	{
		//No roots
		if (a == 0) return false;
		x1 = 0; x2 = sqrtf(-c / a);
		return true;
	}
	float discr = b * b - 4 * a * c;

	//No roots.
	if (discr < 0) return false;

	//More stable version proposed by scratchapixel.com
	// root of discriminant will never cancel out b or produce big round off error due to close values
	float q = (b < 0.f) ? -0.5f * (b - sqrtf(discr)) : -0.5f * (b + sqrtf(discr));
	//Standard quadratic solution
	x1 = q / a;
	//Muller's method
	x2 = c / q;

	return true;
}

bool QuadRenderer::raySphereIntersect(QVector3D orig, QVector3D dir, float radius, float &t0, float &t1)
{
	//They ray dir is normalized so A = 1 
	float A = QVector3D::dotProduct(dir, dir);
	float B = 2 * QVector3D::dotProduct(dir, orig);
	float C = QVector3D::dotProduct(orig, orig) - radius * radius;

	if (!solveQuadratic(A, B, C, t0, t1)) 
		return false;

	if (t0 > t1) 
		std::swap(t0, t1);

	return true;
}

QVector3D QuadRenderer::computeIncidentLight(QVector3D orig, QVector3D dir)
{
	//First and second intersection distances
	float dist0 = -1.0f, dist1 = -1.0f;
	//If ray doesn't intersect with the atmosphere or the only intersection is behind the camera return black light
	if (!raySphereIntersect(orig, dir, atmosphereRadius, dist0, dist1) || dist1 < 0) 
		return QVector3D(0.0f, 0.0f, 0.0f);
	//First intersection can't be behind, should be at least at eye position
	dist0 = std::max(dist0, 0.0f);
	//Length of the segment on the view ray to integrate view ray Transmittance
	float segmentLength = (dist1 - dist0) / viewTSamples;
	//Start from either first intersection if outside of atmosphere or camera position
	float distCurrent = dist0;

	//Rayleigh light contribution
	QVector3D rSum(0.0f, 0.0f, 0.0f);
	//Mie light contribution
	QVector3D mSum(0.0f, 0.0f, 0.0f);
	//Accumulated exponent function of negative height by height scale division for transmittance integral
	//Rayleigh
	float rOpticalDepthAcc = 0.0f;
	//Mie
	float mOpticalDepthAcc = 0.0f;

	//Greek letter mu, cosine between view direction and sun light direction, required for phase function
	float mu = QVector3D::dotProduct(dir, *sunDir) / (dir.length() * sunDir->length());
	//Describes how much light is scattered towards camera
	//Rayleigh
	float rPhase = 3.0f / (16.0f * M_PI) * (1 + mu * mu);
	//Mie
	float mPhase = 3.0f / (8.0f * M_PI) * ((1.0f - g * g) * (1.0f + mu * mu)) / ((2.0f + g * g) * pow(1.0f + g * g - 2.0f * g * mu, 1.5f));

	//Integrate Transmittance over view vector
	for (int i = 0; i < viewTSamples; i++)
	{
		//Sample positions are taken in the middle between sample points
		QVector3D samplePos = orig + (distCurrent + segmentLength * 0.5f) * dir;
		//Altitude of the sample point in relation to the ground
		float height = samplePos.length() - earthRadius;
		if (height < 0) break;
		//Exponent function of negative height by height scale division for view ray transmittance integral
		//Rayleigh
		float rOpticalDepth = exp(-height / rH) * segmentLength;
		rOpticalDepthAcc += rOpticalDepth;
		//Mie
		float mOpticalDepth = exp(-height / mH) * segmentLength;
		mOpticalDepthAcc += mOpticalDepth;
		
		//Distance to atmosphere from sample point position by the light vector
		float dist0Light, dist1Light;
		//Get intersection of light ray with atmosphere. Since sample position is always inside of the atmosphere
		//there is only one intersection of the ray and atmosphere, the second one
		raySphereIntersect(samplePos, *sunDir, atmosphereRadius, dist0Light, dist1Light);
		//Length of the segment on the light ray to integrate light ray Transmittance
		float segmentLengthLight = dist1Light / lightTSamples;
		//Start from sample point position
		float distCurrentLight = 0;
		//Exponent function of negative height by height scale division for light ray transmittance integral
		//Rayleigh
		double rOpticalDepthLightAcc = 0;
		//Mie
		double mOpticalDepthLightAcc = 0;
		int j;
		for (j = 0; j < lightTSamples; ++j) 
		{
			//Sample positions are taken in the middle between sample points
			QVector3D samplePosLight = samplePos + (distCurrentLight + segmentLengthLight * 0.5f) * (*sunDir);
			//Altitude of the sample point in relation to the ground
			float heightLight = samplePosLight.length() - earthRadius;
			//If altitude of a sample point is less than 0
			if (heightLight < 0) break;
			//Exponent function accumulation
			//Rayleigh
			rOpticalDepthLightAcc += exp(-heightLight / rH) * segmentLengthLight;
			//Mie
			mOpticalDepthLightAcc += exp(-heightLight / mH) * segmentLengthLight;
			//Advance forward light vector
			distCurrentLight += segmentLengthLight;
		}
		if (j == lightTSamples)
		{
			//Transmittance of both types of scattering of both light and view rays first without exponential function,
			//which is calculated later so that there would not be four exponential calls
			QVector3D transmittanceWithoutExp = (*rBeta) * (rOpticalDepthAcc + rOpticalDepthLightAcc) + (*mBeta) * 1.1f * (mOpticalDepth + mOpticalDepthLightAcc);
			QVector3D transmittanceWithExp(exp(-transmittanceWithoutExp.x()), exp(-transmittanceWithoutExp.y()), exp(-transmittanceWithoutExp.z()));
			//Transmittance part by what is left of the integral part of the sky colour equation except of scattering coefficient beta, because it is a constant in this case
			//Rayleigh
			rSum += transmittanceWithExp * rOpticalDepth;
			//Mie
			mSum += transmittanceWithExp * mOpticalDepth;
		}
		//Advance forward view vector
		distCurrent += segmentLength;
	}

	//Beta by sum are integral part of sky colour equation
	return (rSum * (*rBeta) * rPhase + mSum * (*mBeta) * mPhase) * sunIntensity;
}
