#ifndef QUADRENDERER_H
#define QUADRENDERER_H

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <QtWidgets>
#include <QVector3D>
#include <QVector2D>
#include <QTransform>
#include <QDebug>
#include <vector>
#include <iostream>
#include "Atmosphere.h"
#include "Shader.h"

//90 degrees fovx is typical for 16:9, which is most common aspect ratio
#define FOV_X 90.0f

class QuadRenderer : protected QOpenGLExtraFunctions
{
public:
	QuadRenderer(int width, int height);
	~QuadRenderer();

	enum Texture
	{
		T,
		E,
		S,
		DELTA_J,
		DELTA_E,
		DELTA_SR,
		DELTA_SM,
		COUNT
	};

	static constexpr Texture PrecomputedTextures[] = { T, E, S };

	void renderAtmosphere(float angleX, float angleY, float camX, float camY, float camZ);
	void rebindPrecomputedTextures();
	void rebuildFramebuffer(int width, int height);

	Atmosphere* atmosphere;
	
private:
	float aspectRatio();
	void precompute();
	void precomputeT();
	void drawTestQuad(float angleX, float angleY, float camX, float camY, float camZ);

	std::vector<GLuint> textureIDs;
	Shader* planetShader;
	Shader* transmittanceShader;
	Shader* screenShader;
	
	int TRANSMITTANCE_SAMPLES = 500;

	int T_W = 64;
	int T_H = 256;
	int E_W = 16;
	int E_H = 64;
	int MU_R = 32;
	int MU_MU = 128;
	int MU_MU_S = 32;
	int MU_NU = 8;

	GLuint FBO = 0;
	GLuint FBOtexture = 0;
	GLuint RBO = 0;

	int width = 0;
	int height = 0;
};
#endif