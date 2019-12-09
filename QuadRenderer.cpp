#include "QuadRenderer.h"

#ifdef _WIN32 
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#include <QtWidgets>
#include <QDebug>
#include <iostream>
#include "Atmosphere.h"

QuadRenderer::QuadRenderer()
{
	initializeOpenGLFunctions();
	//data = new GLubyte[width * height * 3];
	//memset(data, 255, width * height * 3);

	//glActiveTexture(GL_TEXTURE0);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glGenTextures(1, &quadTextureId);
	//glBindTexture(GL_TEXTURE_2D, quadTextureId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)data);

	//Define quad mesh
	//Vertices' attributes pos/tex
	verticesAttributes.insert(verticesAttributes.end(), { -1.0f, -1.0f });
	//verticesAttributes.insert(verticesAttributes.end(), {  0.0f,  0.0f });

	verticesAttributes.insert(verticesAttributes.end(), {  1.0f, -1.0f });
	//verticesAttributes.insert(verticesAttributes.end(), {  1.0f,  0.0f });

	verticesAttributes.insert(verticesAttributes.end(), {  1.0f,  1.0f });
	//verticesAttributes.insert(verticesAttributes.end(), {  1.0f,  1.0f });

	verticesAttributes.insert(verticesAttributes.end(), { -1.0f,  1.0f });
	//verticesAttributes.insert(verticesAttributes.end(), {  0.0f,  1.0f });

	//Indices
	//Triangle 1
	indices.insert(indices.end(), { 0, 1, 2 });
	//Triangle 2
	indices.insert(indices.end(), { 0, 2, 3 });

	//VAO
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	//VBO
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, verticesAttributes.size() * sizeof(verticesAttributes[0]), verticesAttributes.data(), GL_STATIC_DRAW);
	//Attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(verticesAttributes[0]), 0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(verticesAttributes[0]), (void*)(2 * sizeof(verticesAttributes[0])));
	//glEnableVertexAttribArray(1);
	//IBO
	glGenBuffers(1, &quadIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
	
	atmosphere = new Atmosphere();

	//Generate textures
	textureIDs.insert(textureIDs.begin(), COUNT, 0);
	glActiveTexture(GL_TEXTURE0 + T);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[T]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[T]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, T_W, T_H, 0, GL_RGB, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0 + E);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[E]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[E]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, E_W, E_H, 0, GL_RGB, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0 + S);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[S]);
	glBindTexture(GL_TEXTURE_3D, textureIDs[S]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, MU_R, MU_MU, MU_MU_S * MU_NU, 0, GL_RGB, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0 + DELTA_J);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[DELTA_J]);
	glBindTexture(GL_TEXTURE_3D, textureIDs[DELTA_J]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, MU_R, MU_MU, MU_MU_S * MU_NU, 0, GL_RGB, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0 + DELTA_E);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[DELTA_E]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[DELTA_E]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, E_W, E_H, 0, GL_RGB, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0 + DELTA_SR);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[DELTA_SR]);
	glBindTexture(GL_TEXTURE_3D, textureIDs[DELTA_SR]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, MU_R, MU_MU, MU_MU_S * MU_NU, 0, GL_RGB, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0 + DELTA_SM);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[DELTA_SM]);
	glBindTexture(GL_TEXTURE_3D, textureIDs[DELTA_SM]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, MU_R, MU_MU, MU_MU_S * MU_NU, 0, GL_RGB, GL_FLOAT, NULL);

	//Setup planet render shader
	planetShader = new Shader("./Shaders/planet.glsl");
	planetShader->use();

	//Setup precompute shaders
	transmittanceShader = new Shader("./Shaders/precomputeVS.glsl", "null", "./Shaders/transmittanceFS.glsl");
	
	precompute();
	/*
	glActiveTexture(GL_TEXTURE0 + COUNT);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &debTEX);
	glBindTexture(GL_TEXTURE_2D, debTEX);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1616, 1147, 0, GL_RGB, GL_FLOAT, NULL);
	glGenFramebuffers(1, &debFBO);
	*/
}

QuadRenderer::~QuadRenderer()
{
	//clearTexture();
	//glDeleteTextures(1, &quadTextureId);
	glDeleteTextures(textureIDs.size(), textureIDs.data());
	glDeleteBuffers(1, &quadIBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
	delete planetShader;
}

void QuadRenderer::precompute()
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//Need max 2 bufferes for texture output
	GLuint bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
	glBindVertexArray(quadVAO);
	glDepthFunc(GL_LEQUAL);
	
	precomputeT();

	glDepthFunc(GL_LESS);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
}

void QuadRenderer::precomputeT()
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[T], 0);
	glViewport(0, 0, T_W, T_H);
	transmittanceShader->use();
	glUniform3fv(0, 1, &(atmosphere->rBeta[0]));
	glUniform3fv(1, 1, &(atmosphere->mBetaExt[0]));
	glUniform1f(2, atmosphere->rH);
	glUniform1f(3, atmosphere->mH);
	glUniform1f(4, atmosphere->Rg);
	glUniform1f(5, atmosphere->Rt);
	glUniform1i(6, TRANSMITTANCE_SAMPLES);
	glUniform1i(7, T_W);
	glUniform1i(8, T_H);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void QuadRenderer::renderAtmosphere()
{
	glBindVertexArray(quadVAO);
	glDepthFunc(GL_LEQUAL);
	planetShader->use();
	glUniform1f(0, atmosphere->Rg);
	glUniform1f(1, atmosphere->Rt);
	glUniform1i(2, T_W);
	glUniform1i(3, T_H);
	glUniform1f(4, atmosphere->sunAngularRadius);
	glUniform3fv(5, 1, &(atmosphere->s[0]));
	/*
	glBindFramebuffer(GL_FRAMEBUFFER, debFBO);
	GLuint bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, debTEX, 0);
	glViewport(0, 0, 1616, 1147);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
}

//void QuadRenderer::clearTexture()
//{
//	delete[] data;
//	data = 0;
//}

//void QuadRenderer::updateTexture(int width, int height, float angleX, float angleY, float fovX)
//{
//	bool sizeChanged = false;
//	//If new dimensions, allocate new texture, otherwise update
//	if (this->width != width || this->height != height)
//	{
//		//Deallocate bitmap data
//		clearTexture();
//
//		//Allocate new array of colours with correct new width and height
//		data = new GLubyte[width * height * 3];
//		//Set it to white
//		memset(data, 255, width * height * 3);
//
//		//Save new height and width
//		this->height = height;
//		this->width = width;
//
//		//Update sizeChanged bool
//		sizeChanged = true;
//	}
//
//	atmosphere->updateS();
//	//View Rotation quaternion
//	//Camera rotation angle is inverted since quaternion rotation of view vector is ccw, but angle increase turns camera look vector right and up(cw around X and Y axis)
//	QQuaternion viewRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -angleY) * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, -angleX);
//	
//	//Actual update for each pixel
//	for (int j = 0; j < height; j++)
//	{
//		for (int i = 0; i < width; i++)
//		{
//			//Calculate ray direction
//			// x E (tan(-fovX/2); tan(fovX/2))
//			//i / width			// turns into [0;1] range
//			//2 * i / width		// turns into [0;2] range
//			//2 * i / width - 1 // turns into [-1;1] range
//			//tanf(fovX / 180.0f * M_PI / 2.0f) = 1 if fovX = 90
//			float x = (2.0f * i / static_cast<float>(width) - 1.0f); // * tanf(fovX / 180.0f * M_PI / 2.0f)
//			// y E (-tan(fovY/2); tan(fovY/2))
//			float y = (2.0f * j / static_cast<float>(height) - 1.0f) / 
//			//Divide by aspect ratio to convert x fov resulting values to y fov
//					(static_cast<float>(width) / static_cast<float>(height)); // * tanf(fovX / 180.0f * M_PI / 2.0f)
//			//By default camera looks in the direction of +z, hence x is inverted (as it increases to the left relative from camera look direction)
//			QVector3D rayDir(-x, y, 1.0);
//			//Rotate by quaternion
//			rayDir = viewRot.rotatedVector(rayDir);
//			rayDir.normalize();
//
//			//Compute light one meter from earth
//			QVector3D lightColour = atmosphere->computeIncidentLight(QVector3D(0.0f, atmosphere->getGroundRadius() + 1.0f, 0.0f), rayDir);
//			//QVector3D lightColour = atmosphere->computeZeroScatteringLightRealtime(QVector3D(0.0f, atmosphere->getGroundRadius() + 1.0f, 0.0f), rayDir);
//
//			//qDebug() << lightColour;
//
//			//Clamp and convert to byte format
//			data[i * 3 + j * width * 3] = static_cast<byte>(std::max(0.0f, std::min(1.0f, lightColour.x())) * 255);
//			data[i * 3 + j * width * 3 + 1] = static_cast<byte>(std::max(0.0f, std::min(1.0f, lightColour.y())) * 255);
//			data[i * 3 + j * width * 3 + 2] = static_cast<byte>(std::max(0.0f, std::min(1.0f, lightColour.z())) * 255);
//
//			
//
//			//std::cout << lightColour.x() << ", " << lightColour.y() << ", " << lightColour.z() << std::endl;
//		}
//	}
//
//	//Texture test
//	/*float normalizedAngleX = (angleX + 90) / 180.0f;
//	for (int i = 0; i < width*height * 3 ; i += 3)
//	{
//		data[i] = normalizedAngleX * 81 + (1.0f - normalizedAngleX) * 109;
//		data[i + 1] = normalizedAngleX * 198 + (1.0f - normalizedAngleX) * 31;
//		data[i + 2] = normalizedAngleX * 31 + (1.0f - normalizedAngleX) * 198;
//	}*/
//
//	//
//	if (sizeChanged)
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)data);
//	else
//		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)data);
//}

void QuadRenderer::rebindPrecomputedTextures()
{
	for (int textureType : PrecomputedTextures)
	{
		glActiveTexture(GL_TEXTURE0 + textureType);
		if ((textureType == T) || (textureType == E) || (textureType == DELTA_E))
			glBindTexture(GL_TEXTURE_2D, textureIDs[textureType]);
		else
			glBindTexture(GL_TEXTURE_3D, textureIDs[textureType]);
	}
}

