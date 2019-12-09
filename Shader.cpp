#include "Shader.h"
#include <fstream>
#include <sstream>
#include <QDebug>
#include <string>

Shader::Shader(const std::string& combinedShaderPath)
{
	initializeOpenGLFunctions();
	std::string headerCode = getStringFromFile("./Shaders/common.glsl");

	std::string shaderCode = getStringFromFile(combinedShaderPath);
	std::string vertexShaderCode = headerCode + "\n#define VERTEX\n" + shaderCode;
	std::string geometryShaderCode = "null";
	if (shaderCode.find("#ifdef GEOMETRY") != std::string::npos)
		std::string geometryShaderCode = headerCode + "\n#define GEOMETRY\n" + shaderCode;
	std::string fragmentShaderCode = headerCode + "\n#define FRAGMENT\n" + shaderCode;

	ID = buildProgramFromShaderCode(vertexShaderCode, geometryShaderCode, fragmentShaderCode);
}

Shader::Shader(const std::string& vertexShaderPath, const std::string& geometryShaderPath, const std::string& fragmentShaderPath)
{
	initializeOpenGLFunctions();
	std::string headerCode = getStringFromFile("./Shaders/common.glsl") + "\n";

	std::string vertexShaderCode = headerCode + getStringFromFile(vertexShaderPath);
	std::string geometryShaderCode = "null";
	if (geometryShaderPath != "null")
		std::string geometryShaderCode = headerCode + getStringFromFile(geometryShaderPath);
	std::string fragmentShaderCode = headerCode + getStringFromFile(fragmentShaderPath);

	ID = buildProgramFromShaderCode(vertexShaderCode, geometryShaderCode, fragmentShaderCode);
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}

std::string Shader::getStringFromFile(const std::string& path)
{
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		std::stringstream stream;
		file.open(path);
		stream << file.rdbuf();
		file.close();
		return stream.str();
	}
	catch (std::ifstream::failure e)
	{
		qDebug() << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ";
	}
}

GLuint Shader::buildProgramFromShaderCode(const std::string& vertexShaderCode, const std::string& geometryShaderCode, const std::string& fragmentShaderCode)
{
	GLuint vertexShaderID = compileShaderFromCode(GL_VERTEX_SHADER, vertexShaderCode);
	GLuint geometryShaderID = 0;
	if (geometryShaderCode != "null")
		geometryShaderID = compileShaderFromCode(GL_GEOMETRY_SHADER, geometryShaderCode);
	GLuint fragmentShaderID = compileShaderFromCode(GL_FRAGMENT_SHADER, fragmentShaderCode);

	GLuint programID = glCreateProgram();

	glAttachShader(programID, vertexShaderID);
	if (geometryShaderID)
		glAttachShader(programID, geometryShaderID);
	glAttachShader(programID, fragmentShaderID);

	glLinkProgram(programID);
	checkCompilerErrors(programID, "PROGRAM");

	glDetachShader(programID, vertexShaderID);
	glDeleteShader(vertexShaderID);
	if (geometryShaderID)
	{
		glDetachShader(programID, geometryShaderID);
		glDeleteShader(geometryShaderID);
	}
	glDetachShader(programID, fragmentShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}

GLuint Shader::compileShaderFromCode(GLenum shaderStage, const std::string& shaderCode)
{
	GLuint shader = glCreateShader(shaderStage);
	const char* shaderCodeC_STR = shaderCode.c_str();
	glShaderSource(shader, 1, &shaderCodeC_STR, NULL);
	glCompileShader(shader);
	switch (shaderStage)
	{
		case GL_VERTEX_SHADER:
			checkCompilerErrors(shader, "VERTEX");
			break;
		case GL_GEOMETRY_SHADER:
			checkCompilerErrors(shader, "GEOMETRY");
			break;
		case GL_FRAGMENT_SHADER:
			checkCompilerErrors(shader, "FRAGMENT");
			break;
	}

	return shader;
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::checkCompilerErrors(GLuint id, const char* type)
{
	int success;
	char infoLog[1024];
	if (std::strcmp(type, "PROGRAM"))
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(id, 1024, NULL, infoLog);
			qDebug() << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- ";
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(id, 1024, NULL, infoLog);
			qDebug() << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- ";
		}
	}
}

