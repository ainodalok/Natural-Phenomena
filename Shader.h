#ifndef SHADER_H
#define SHADER_H

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <QOpenGLExtraFunctions>
#include <string>
#include "Atmosphere.h"

class Shader : protected QOpenGLExtraFunctions
{
public:
	Shader(const std::string& combinedShaderPath);
	Shader(const std::string& vertexShaderPath, const std::string& geometryShaderPath, const std::string& fragmentShaderPath);
	~Shader();

	void use();

private:
	GLuint ID = 0;

	std::string getStringFromFile(const std::string& path);
	GLuint buildProgramFromShaderCode(const std::string& vertexShaderCode, const std::string& geometryShaderCode, const std::string& fragmentShaderCode);
	GLuint compileShaderFromCode(GLenum shaderStage, const std::string& shaderCode);
	void checkCompilerErrors(GLuint id, const char* type);
};
#endif