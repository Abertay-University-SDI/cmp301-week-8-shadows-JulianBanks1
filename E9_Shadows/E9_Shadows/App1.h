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

	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;


	OrthoMesh* orthoMesh;
	RenderTexture* renderTexture;
	CubeMesh* cube;
	float modelRot = 0;

	static const int LIGHT_COUNT = 2;
	Light* light[LIGHT_COUNT];
	float lightPos[LIGHT_COUNT][3];
	float lightDir[LIGHT_COUNT][3];
	float lightAmb[LIGHT_COUNT][4];
	float lightDiff[LIGHT_COUNT][4];
	float lightSpec[LIGHT_COUNT][4];
	float lightSpecPower[LIGHT_COUNT];
	int lightType[LIGHT_COUNT];
	float lightAtten[LIGHT_COUNT][3];
	SphereMesh* lightSphere[LIGHT_COUNT];
	ShadowMap* shadowMap[LIGHT_COUNT];
};

#endif