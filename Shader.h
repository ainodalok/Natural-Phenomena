#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLExtraFunctions>
#include <string>
#include <fstream>
#include <sstream>
#include <QDebug>

class Shader : protected QOpenGLExtraFunctions
{
public:
	explicit Shader(const std::string& combinedShaderPath);
	Shader(const std::string& vertexShaderPath, const std::string& geometryShaderPath, const std::string& fragmentShaderPath);
	Shader(const Shader&) = default;
	Shader& operator=(const Shader&) = default;
	Shader(const Shader&&) = delete;
	Shader& operator=(const Shader&&) = delete;
	~Shader();

	void use();

private:
	GLuint ID = 0;

	static std::string getStringFromFile(const std::string& path);
	GLuint buildProgramFromShaderCode(const std::string& vertexShaderCode, const std::string& geometryShaderCode, const std::string& fragmentShaderCode);
	GLuint compileShaderFromCode(GLenum shaderStage, const std::string& shaderCode);
	void checkCompilerErrors(GLuint id, const char* type);
};
#endif