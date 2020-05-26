#ifndef QUADRENDERER_H
#define QUADRENDERER_H

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <QtMath>
#include <QTime>
#include <vector>

#include "Atmosphere.h"
#include "Shader.h"
#include "LogWindow.h"

//90 degrees fovx is typical for 16:9, which is most common aspect ratio
constexpr float FOV_X = 90.0f;

class Renderer final : protected QOpenGLExtraFunctions
{
public:
	Renderer(int width, int height, Atmosphere* const atmosphere);
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator =(Renderer const&) = delete;
	Renderer(const Renderer&&) = delete;
	Renderer& operator =(Renderer const&&) = delete;
	
	enum Texture
	{
		T,
		E,
		S,
		DELTA_J,
		DELTA_E,
		DELTA_SR,
		DELTA_SM,
		PERLIN_WORLEY,
		COUNT
	};

	static constexpr Texture PrecomputedTextures[] = { T, E, S, PERLIN_WORLEY };

	void renderAtmosphere(float angleX, float angleY, QVector3D& cam);
	void rebindPrecomputedTextures();
	void rebuildFramebuffer(int width, int height);
	void precompute();
	void allocateTextures();
	void deletePrecomputedTextures();

	Atmosphere* atmosphere = nullptr;
	
	int width = 0;
	int height = 0;

private:
	[[nodiscard]] float aspectRatio() const;
	void precomputeT();
	void precomputeDeltaJ(int order);
	void precomputeDeltaE(int order);
	void precomputeDeltaS(int order);
	void copyIrradiance();
	void copyScattering(int order);
	void precomputeCloudNoise();
	void setCommonUniforms();
	void setup3DShader(Shader*& shader, const std::string& fsPath);
	void setup2DShader(Shader*& shader, const std::string& fsPath);

	std::vector<GLuint> textureIDs;
	Shader* transmittanceShader;
	Shader* directIrradianceShader;
	Shader* singleScatteringShader;
	Shader* copySingleScatteringShader;
	Shader* firstStepMultipleScatteringShader;
	Shader* indirectIrradianceShader;
	Shader* secondStepMultipleScatteringShader;
	Shader* copyIrradianceShader;
	Shader* copyMultipleScatteringShader;
	Shader* planetShader;
	Shader* screenShader;
	Shader* cloudNoiseShader;

	GLuint FBO = 0;
	GLuint screenTexture = 0;
	GLuint RBO = 0;
};
#endif