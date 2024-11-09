// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void texturePass();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;

	Light* light[2];
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap[2];

	OrthoMesh* orthoMesh;
	RenderTexture* renderTexture;
	CubeMesh* cube;
	SphereMesh* sphere;
	float modelRot = 0;
	float lightPos[2][3];
	float lightDir[2][3];
	float lightDiff[2][4];
};

#endif