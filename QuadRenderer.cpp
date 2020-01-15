#include "QuadRenderer.h"

QuadRenderer::QuadRenderer(int width, int height)
{
	initializeOpenGLFunctions();
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	atmosphere = new Atmosphere();

	this->width = width;
	this->height = height;

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
	planetShader = new Shader("Shaders/planet.glsl");
	//Setup precompute shaders
	transmittanceShader = new Shader("Shaders/precomputeVS.glsl", "null", "Shaders/transmittanceFS.glsl");
	//Setup screen shader
	screenShader = new Shader("Shaders/screen.glsl");
	
	rebuildFramebuffer(width, height);

	precompute();
}

QuadRenderer::~QuadRenderer()
{
	glDeleteTextures(textureIDs.size(), textureIDs.data());
	glDeleteFramebuffers(1, &FBO);
	glDeleteRenderbuffers(1, &RBO);
	glDeleteTextures(1, &FBOtexture);
	delete planetShader;
	delete transmittanceShader;
}

//returns current aspect ratio based on current window
float QuadRenderer::aspectRatio()
{
	return float(width) / float(height);
}

void QuadRenderer::precompute()
{
	precomputeT();
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
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void QuadRenderer::drawTestQuad(float angleX, float angleY, float camX, float camY, float camZ)
{
	//Initial rendering setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//using frustum here calculated with formula for yfov from xfov, xfov is usually 90 degrees in 16:9 aspect ratio windows
	//tan(FOV_X / 180 * M_PI*0.5)  = 1 if FOV_X = 90 degrees
	gluPerspective(qRadiansToDegrees(2.0f * atan(tan(qDegreesToRadians(FOV_X) / 2.0f) / aspectRatio())),
		aspectRatio(),
		0.1f,
		100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//Looking at z- with y+ vector up
	gluLookAt(0., 0., 0.,
		0.0, 0.0, -1.0,
		0.0, 1.0, 0.0);
	//Look into angle defs. for more info
	//Increase in vertical angle turns camera up(while world goes down), since it is a ccw roation around X axis
	glRotatef(-angleX, 1, 0, 0);
	//Increase in horizontal angle turns camera right(while world goes left), since it is a ccw rotation around Y axis
	glRotatef(angleY, 0, 1, 0);
	//move camera to the location of controllable character
	//Actually translates world, not camera position, hence negative values are passed as they represent actual camera location in the world without translations!
	glTranslatef(-camX, -camY, -camZ);
	//Testquad
	glUseProgram(0);
	glColor4f(0.0, 1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1, -1, -2.0f);
	glTexCoord2f(1.0f, 0.0f);   glVertex3f(1, -1, -2.0f);
	glTexCoord2f(1.0f, 1.0f);   glVertex3f(1, 1, -2.0f);
	glTexCoord2f(0.0f, 1.0f);   glVertex3f(-1, 1, -2.0f);
	glEnd();
}

void QuadRenderer::renderAtmosphere(float angleX, float angleY, float camX, float camY, float camZ)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, FBOtexture, 0);
	glViewport(0, 0, width, height);
	//clear the widget
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	planetShader->use();
	glUniform1f(0, atmosphere->Rg);
	glUniform1f(1, atmosphere->Rt);
	glUniform1i(2, T_W);
	glUniform1i(3, T_H);
	glUniform1f(4, atmosphere->sunAngularRadius);
	glUniform3fv(5, 1, &(atmosphere->s[0]));
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glEnable(GL_DEPTH_TEST);

	drawTestQuad(angleX, angleY, camX, camY, camZ);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	screenShader->use();
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

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

void QuadRenderer::rebuildFramebuffer(int width, int height)
{
	this->width = width;
	this->height = height;
	if (FBO != 0)
	{
		glDeleteTextures(1, &FBOtexture);
		glDeleteRenderbuffers(1, &RBO);
	}
	else
		glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Need max 2 bufferes for texture output
	GLuint bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
	glGenTextures(1, &FBOtexture);
	glActiveTexture(GL_TEXTURE0 + COUNT);
	glBindTexture(GL_TEXTURE_2D, FBOtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, FBOtexture, 0);
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}