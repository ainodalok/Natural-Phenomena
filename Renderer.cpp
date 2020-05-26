#include "Renderer.h"

Renderer::Renderer(const int width, const int height, Atmosphere* const atmosphere)
{
	initializeOpenGLFunctions();
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	this->atmosphere = atmosphere;

	this->width = width;
	this->height = height;

	//Setup precompute shaders
	setup2DShader(transmittanceShader, "Shaders/transmittanceFS.glsl");
	setup2DShader(directIrradianceShader, "Shaders/directIrradianceFS.glsl");
	setup3DShader(singleScatteringShader, "Shaders/singleScatteringFS.glsl");
	setup3DShader(copySingleScatteringShader, "Shaders/copySingleScatteringFS.glsl");
	setup3DShader(firstStepMultipleScatteringShader, "Shaders/firstStepMultipleScatteringFS.glsl");
	setup2DShader(indirectIrradianceShader, "Shaders/indirectIrradianceFS.glsl");
	setup3DShader(secondStepMultipleScatteringShader, "Shaders/secondStepMultipleScatteringFS.glsl");
	setup2DShader(copyIrradianceShader, "Shaders/copyIrradianceFS.glsl");
	setup3DShader(copyMultipleScatteringShader, "Shaders/copyMultipleScatteringFS.glsl");

	setup3DShader(cloudNoiseShader, "Shaders/cloudNoiseFS.glsl");
	
	//Setup planet render shader
	planetShader = new Shader("Shaders/planet.glsl");
	//Setup screen shader
	screenShader = new Shader("Shaders/screen.glsl");
	
	rebuildFramebuffer(width, height);

	//Generate textures
	textureIDs.insert(textureIDs.begin(), COUNT, 0);
	allocateTextures();
	precompute();
}

Renderer::~Renderer()
{
	deletePrecomputedTextures();
	glDeleteFramebuffers(1, &FBO);
	glDeleteRenderbuffers(1, &RBO);
	glDeleteTextures(1, &screenTexture);
	delete planetShader;
	delete transmittanceShader;
	delete directIrradianceShader;
	delete singleScatteringShader;
	delete copySingleScatteringShader;
	delete firstStepMultipleScatteringShader;
	delete indirectIrradianceShader;
	delete secondStepMultipleScatteringShader;
	delete copyIrradianceShader;
	delete copyMultipleScatteringShader;
	delete cloudNoiseShader;
}

//returns current aspect ratio based on current window
float Renderer::aspectRatio() const
{
	return float(width) / float(height);
}

void Renderer::precompute()
{
	//Depth buffer is not needed when doing precomputations
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

	precomputeT();
	precomputeDeltaE(0);
	precomputeDeltaS(0);
	//0th order irradiance delta is then not copied into irradiance accumulation texture, since it is calculated real-time
	copyScattering(0);
	glFinish();
	LogWindow::getInstance()->appendMessage(QString::fromStdString("Scattering order 1/" + std::to_string(atmosphere->ORDER_COUNT) + " precomputed!"));
	for (int order = 1; order < atmosphere->ORDER_COUNT; order++)
	{
		precomputeDeltaJ(order);
		precomputeDeltaE(order);
		precomputeDeltaS(order);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		copyIrradiance();
		copyScattering(order);
		glDisable(GL_BLEND);
		glFinish();
		LogWindow::getInstance()->appendMessage(QString::fromStdString("Scattering order " + std::to_string(order + 1) + "/" + std::to_string(atmosphere->ORDER_COUNT) + " precomputed!"));
	}
	precomputeCloudNoise();
	glFinish();
	GLenum err;
	LogWindow::getInstance()->appendMessage(QString::fromStdString("Noise precomputed!"));
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void Renderer::precomputeT()
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[T], 0);
	glViewport(0, 0, atmosphere->T_W, atmosphere->T_H);
	transmittanceShader->use();
	setCommonUniforms();
	glUniform3fv(14, 1, &atmosphere->rBeta[0]);
	glUniform3fv(15, 1, &atmosphere->mBetaExt[0]);
	glUniform1f(16, atmosphere->rH);
	glUniform1f(17, atmosphere->mH);
	glUniform1i(18, atmosphere->TRANSMITTANCE_SAMPLES);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::precomputeDeltaE(int order)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[DELTA_E], 0);
	glViewport(0, 0, atmosphere->E_W, atmosphere->E_H);
	//Direct (L[0]) irradiance
	if (order == 0)
		directIrradianceShader->use();
	else
		indirectIrradianceShader->use();
	setCommonUniforms();
	if (order != 0)
	{
		glUniform1i(14, order);
		glUniform1i(15, atmosphere->IRRADIANCE_SPHERICAL_SAMPLES);
	}
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::precomputeDeltaJ(int order)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[DELTA_J], 0);
	glViewport(0, 0, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU);
	firstStepMultipleScatteringShader->use();
	setCommonUniforms();
	glUniform1i(14, atmosphere->SCATTERING_SPHERICAL_SAMPLES);
	glUniform3fv(15, 1, &atmosphere->surfaceAlbedo[0]);
	glUniform1i(16,order);
	glUniform3fv(17, 1, &atmosphere->rBeta[0]);
	glUniform3fv(18, 1, &atmosphere->mBeta[0]);
	glUniform1f(19, atmosphere->rH);
	glUniform1f(20, atmosphere->mH);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, atmosphere->MU_R);
}

void Renderer::precomputeDeltaS(int order)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[DELTA_SR], 0);
	if(order == 0)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textureIDs[DELTA_SM], 0);
	glViewport(0, 0, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU);
	
	//Single (L[0]) scattering
	if (order == 0)
		singleScatteringShader->use();
	else
		secondStepMultipleScatteringShader->use();
	
	setCommonUniforms();
	glUniform1i(14, atmosphere->SCATTERING_SAMPLES);
	if (order == 0)
	{
		glUniform3fv(15, 1, &atmosphere->rBeta[0]);
		glUniform3fv(16, 1, &atmosphere->mBeta[0]);
		glUniform1f(17, atmosphere->rH);
		glUniform1f(18, atmosphere->mH);
	}
	
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, atmosphere->MU_R);

	if(order == 0)
		//Remove texture attached to second color attachment
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
}

void Renderer::copyIrradiance()
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[E], 0);
	glViewport(0, 0, atmosphere->E_W, atmosphere->E_H);
	copyIrradianceShader->use();
	setCommonUniforms();

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::copyScattering(int order)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[S], 0);
	glViewport(0, 0, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU);
	//Single (L[0]) scattering
	if (order == 0)
		copySingleScatteringShader->use();
	else
		copyMultipleScatteringShader->use();
	setCommonUniforms();

	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, atmosphere->MU_R);
}

void Renderer::precomputeCloudNoise()
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[PERLIN_WORLEY], 0);
	glViewport(0, 0, atmosphere->CLOUD_W, atmosphere->CLOUD_H);
	cloudNoiseShader->use();
	setCommonUniforms();
	glUniform1i(14, atmosphere->CLOUD_W);
	glUniform1i(15, atmosphere->CLOUD_H);
	glUniform1i(16, atmosphere->CLOUD_D);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, atmosphere->CLOUD_D);
}

void Renderer::renderAtmosphere(const float angleX, const float angleY, QVector3D& cam)
{	
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenTexture, 0);
	glViewport(0, 0, width, height);
	//clear the widget
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	planetShader->use();
	setCommonUniforms();
	glUniform3fv(14, 1, &atmosphere->s[0]);
	glUniform1f(15, width);
	glUniform1f(16, height);
	glUniform1f(17, FOV_X);
	//Inverse rotation matrix for rays => viewmatrix
	glUniformMatrix3fv(18, 1, GL_FALSE, QQuaternion::fromEulerAngles(angleX, -angleY, 0.0f).toRotationMatrix().constData());
	glUniform3fv(19, 1, &cam[0]);
	glUniform3fv(20, 1, &atmosphere->surfaceAlbedo[0]);
	glUniform3fv(21, 1, &atmosphere->rBeta[0]);
	glUniform3fv(22, 1, &atmosphere->mBeta[0]);
	glUniform1f(23, atmosphere->exposure);
	glUniform3fv(24, 1, &atmosphere->whitePoint[0]);
	glUniform1f(25, atmosphere->cloudOpacity * 10.0f);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glEnable(GL_DEPTH_TEST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	screenShader->use();
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::rebindPrecomputedTextures()
{
	for (int textureType : PrecomputedTextures)
	{
		glActiveTexture(GL_TEXTURE0 + textureType);
		if (textureType != S && textureType != PERLIN_WORLEY)
			glBindTexture(GL_TEXTURE_2D, textureIDs[textureType]);
		else
			glBindTexture(GL_TEXTURE_3D, textureIDs[textureType]);
	}
}

void Renderer::rebuildFramebuffer(const int width, const int height)
{
	this->width = width;
	this->height = height;
	if (FBO != 0)
	{
		glDeleteTextures(1, &screenTexture);
		glDeleteRenderbuffers(1, &RBO);
	}
	else
		glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Need max 2 bufferes for texture output
	GLuint bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
	glGenTextures(1, &screenTexture);
	glActiveTexture(GL_TEXTURE0 + COUNT);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenTexture, 0);
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setCommonUniforms()
{
	glUniform1f(0, atmosphere->Rg);
	glUniform1f(1, atmosphere->Rt);
	glUniform1i(2, atmosphere->T_W);
	glUniform1i(3, atmosphere->T_H);
	glUniform1i(4, atmosphere->E_W);
	glUniform1i(5, atmosphere->E_H);
	glUniform1i(6, atmosphere->MU_R);
	glUniform1i(7, atmosphere->MU_MU);
	glUniform1i(8, atmosphere->MU_MU_S);
	glUniform1i(9, atmosphere->MU_NU);
	glUniform1f(10, atmosphere->sunAngularRadius);
	glUniform1f(11, atmosphere->muSmin);
	glUniform1f(12, atmosphere->g);
	glUniform1f(13, atmosphere->solarIrradiance);
}

void Renderer::setup3DShader(Shader*& shader ,const std::string& fsPath)
{
	shader = new Shader("Shaders/precomputeVS.glsl", "Shaders/instanced3DLayerSetterGS.glsl", fsPath);
}

void Renderer::setup2DShader(Shader*& shader, const std::string& fsPath)
{
	shader = new Shader("Shaders/precomputeVS.glsl", "null", fsPath);
}

void Renderer::allocateTextures()
{
	glActiveTexture(GL_TEXTURE0 + T);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[T]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[T]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, atmosphere->T_W, atmosphere->T_H, 0, GL_RGB, GL_FLOAT, nullptr);

	glActiveTexture(GL_TEXTURE0 + E);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[E]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[E]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, atmosphere->E_W, atmosphere->E_H, 0, GL_RGB, GL_FLOAT, nullptr);

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU, atmosphere->MU_R, 0, GL_RGBA, GL_FLOAT, nullptr);

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU, atmosphere->MU_R, 0, GL_RGB, GL_FLOAT, nullptr);

	glActiveTexture(GL_TEXTURE0 + DELTA_E);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[DELTA_E]);
	glBindTexture(GL_TEXTURE_2D, textureIDs[DELTA_E]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, atmosphere->E_W, atmosphere->E_H, 0, GL_RGB, GL_FLOAT, nullptr);

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU, atmosphere->MU_R, 0, GL_RGB, GL_FLOAT, nullptr);

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, atmosphere->MU_MU_S * atmosphere->MU_NU, atmosphere->MU_MU, atmosphere->MU_R, 0, GL_RGB, GL_FLOAT, nullptr);

	glActiveTexture(GL_TEXTURE0 + PERLIN_WORLEY);
	//Each row must be aligned to 1-4 bytes, 1 means no alignment!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureIDs[PERLIN_WORLEY]);
	glBindTexture(GL_TEXTURE_3D, textureIDs[PERLIN_WORLEY]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, atmosphere->CLOUD_W, atmosphere->CLOUD_H, atmosphere->CLOUD_D, 0, GL_RED, GL_FLOAT, nullptr);
}

void Renderer::deletePrecomputedTextures()
{
	glDeleteTextures(textureIDs.size(), textureIDs.data());
}