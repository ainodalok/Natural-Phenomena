#ifndef QUADRENDERER_H
#define QUADRENDERER_H

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#include <vector>
#include <QVector2D>
#include <QVector3D>
#include "Atmosphere.h"
#include "Shader.h"

class QuadRenderer : protected QOpenGLExtraFunctions
{
public:
	QuadRenderer();
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

	void renderAtmosphere();
	//void updateTexture(int width, int height, float angleX, float angleY, float fovX);
	//void clearTexture();
	void rebindPrecomputedTextures();

	//int width = 0;
	//int height = 0;

	Atmosphere* atmosphere;
	
private:
	void precompute();
	void precomputeT();

	
	GLuint quadVAO;
	GLuint quadVBO;
	GLuint quadIBO;
	std::vector<float> verticesAttributes;
	std::vector<int> indices;
	std::vector<GLuint> textureIDs;
	//GLuint quadTextureId;
	//GLubyte* data;
	Shader* planetShader;
	Shader* transmittanceShader;
	
	int TRANSMITTANCE_SAMPLES = 500;

	int T_W = 64;
	int T_H = 256;
	int E_W = 16;
	int E_H = 64;
	int MU_R = 32;
	int MU_MU = 128;
	int MU_MU_S = 32;
	int MU_NU = 8;

	GLuint debFBO;
	GLuint debTEX;
};
#endif