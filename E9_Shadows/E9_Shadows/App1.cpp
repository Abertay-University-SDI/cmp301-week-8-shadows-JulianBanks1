// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 2024;
	int shadowmapHeight = 2024;
	int sceneWidth = 200;
	int sceneHeight = 200;

	// This is your shadow map
	// Configure directional light
	for (int i = 0; i < 2; i++)
	{
		shadowMap[i] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

		light[i] = new Light();
		light[i]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
		light[i]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
		light[i]->setDirection(0.0f, -0.7f, 0.7f);
		light[i]->setPosition(0.f, 0.f, -10.f);
		light[i]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 200.f);
		lightPos[i][0] = light[i]->getPosition().x;
		lightPos[i][1] = light[i]->getPosition().y;
		lightPos[i][2] = light[i]->getPosition().z;
		lightDir[i][0] = light[i]->getDirection().x;
		lightDir[i][1] = light[i]->getDirection().y;
		lightDir[i][2] = light[i]->getDirection().z;

	}



	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 2, screenHeight/ 2, -screenWidth * 0.75f, screenHeight * 0.75f);
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	modelRot += 0.01f;
	for (int i = 0; i < 2; i++)
	{
		light[i]->setPosition(lightPos[i][0], lightPos[i][1], lightPos[i][2]);
		light[i]->setDirection(lightDir[i][0], lightDir[i][1], lightDir[i][2]);
		light[i]->setDiffuseColour(lightDiff[i][0], lightDiff[i][1], lightDiff[i][2], lightDiff[i][3]);
	}


	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	//texturePass();
	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::texturePass()
{
	// Set the render target to be the render to texture.
	//shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 0.5f);


	XMMATRIX lightViewMatrix[2];
	XMMATRIX lightProjectionMatrix[2];
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	// get the world, view, and projection matrices from the camera and d3d objects.
	for (int i = 0; i < 2; i++)
	{
		light[i]->generateViewMatrix();
		lightViewMatrix[i] = light[i]->getViewMatrix();
		lightProjectionMatrix[i] = light[i]->getOrthoMatrix();
	}


	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, *lightViewMatrix, *lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	worldMatrix += XMMatrixRotationX(modelRot);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, *lightViewMatrix, *lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(light[0]->getPosition().x, light[0]->getPosition().y, light[0]->getPosition().z);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, *lightViewMatrix, *lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(10.f, 10.f, 10.f);
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, *lightViewMatrix, *lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
}

void App1::depthPass()
{
	for (int i = 0; i < 2; i++)
	{
		// Set the render target to be the render to texture.
		shadowMap[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// get the world, view, and projection matrices from the camera and d3d objects.
		light[i]->generateViewMatrix();
		XMMATRIX lightViewMatrix = light[i]->getViewMatrix();
		XMMATRIX lightProjectionMatrix = light[i]->getOrthoMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		// Render floor
		mesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
		XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
		worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
		worldMatrix += XMMatrixRotationX(modelRot);
		// Render model
		model->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(10.f, 10.f, 10.f);
		cube->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

		// Set back buffer as render target and reset view port.
		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	/*ID3D11ShaderResourceView* shadowMapSRV[2];
	for (int i = 0; i < 2; i++)
	{
		shadowMapSRV[i] = shadowMap[i]->getDepthMapSRV();
	}*/

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap, light);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	worldMatrix += XMMatrixRotationX(modelRot);

	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap, light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(light[0]->getPosition().x, light[0]->getPosition().y, light[0]->getPosition().z);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap, light);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(10.f, 10.f, 10.f);
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap, light);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	for (int i = 0; i < 2; i++)
	{
		orthoMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap[i]->getDepthMapSRV());
		textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		renderer->setZBuffer(true);
	}

	

	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::SliderFloat3("Pos", lightPos[0], -40, 40);
	ImGui::SliderFloat3("Dur", lightDir[0], -4, 4);
	ImGui::ColorEdit4("Diff0", lightDiff[0]);
	ImGui::SliderFloat3("Pos1", lightPos[1], -40, 40);
	ImGui::SliderFloat3("Dur1", lightDir[1], -4, 4);
	ImGui::ColorEdit4("Diff1", lightDiff[1]);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

