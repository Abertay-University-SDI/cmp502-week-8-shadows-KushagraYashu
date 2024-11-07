// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

int sceneWidth = 100;
int sceneHeight = 100;

int screenWidthVar, screenHeightVar;

float lightDirection[2][3];
float lightPlanePosition[2][3];

float teaPot1Pos[3] = { 0.f, 4.f, 5.f };
float teaPot2Pos[3] = { 0.f, 4.f, 15.f };

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	screenWidthVar = screenWidth;
	screenHeightVar = screenHeight;

	sceneWidth = 200;
	sceneHeight = 200;

	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	//creating orthomesh
	debugOrthoMesh[0] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, -screenWidth / 2.7, screenHeight / 2.7);
	debugOrthoMesh[1] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 2.7, screenHeight / 2.7);
	debugPlaneMesh[0] = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 100);
	debugPlaneMesh[1] = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 100);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 4096*4;
	int shadowmapHeight = 4096*4;


	// This is your shadow map
	shadowMap[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMap[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Configure directional light
	light[0] = new Light();
	light[0]->setAmbientColour(0.1f, 0.f, 0.f, 1.0f);
	light[0]->setDiffuseColour(1.0f, 0.f, 0.f, 1.0f);
	light[0]->setDirection(1.0f, -1.f, 0.f);
	lightDirection[0][0] = 1.f;
	lightDirection[0][1] = -1.f;
	lightDirection[0][2] = 0.f;
	light[0]->setPosition(-70.f, 10.f, -10.f);
	lightPlanePosition[0][0] = -70.f;
	lightPlanePosition[0][1] = 10.f;
	lightPlanePosition[0][2] = -10.f;
	//light[0]->generateViewMatrix();
	//light[0]->generateProjectionMatrix(0.1f, 10.f);
	light[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, SCREEN_NEAR, SCREEN_DEPTH);

	light[1] = new Light();
	light[1]->setAmbientColour(0.0f, 0.f, 0.1f, 1.0f);
	light[1]->setDiffuseColour(0.f, 0.f, 1.f, 1.0f);
	light[1]->setDirection(-1.0f, -1.f, 0.f);
	lightDirection[1][0] = -1.f;
	lightDirection[1][1] = -1.f;
	lightDirection[1][2] = 0.f;
	light[1]->setPosition(70.f, 10.f, -10.f);
	lightPlanePosition[1][0] = 70.f;
	lightPlanePosition[1][1] = 10.f;
	lightPlanePosition[1][2] = -10.f;
	//light[1]->generateViewMatrix();
	//light[1]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	light[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, SCREEN_NEAR, SCREEN_DEPTH);
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

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	//doing light[0]
	// Set the render target to be the render to texture.
	shadowMap[0]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	light[0]->generateViewMatrix();
	XMMATRIX lightViewMatrix = light[0]->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light[0]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(teaPot1Pos[0], teaPot1Pos[1], teaPot1Pos[2]), lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	/*model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(teaPot2Pos[0], teaPot2Pos[1], teaPot2Pos[2]), lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());*/

	//doing light[1]
	// Set the render target to be the render to texture.
	shadowMap[1]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	light[1]->generateViewMatrix();
	lightViewMatrix = light[1]->getViewMatrix();
	lightProjectionMatrix = light[1]->getOrthoMatrix();
	worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(teaPot1Pos[0], teaPot1Pos[1], teaPot1Pos[2]), lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	/*model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(teaPot2Pos[0], teaPot2Pos[1], teaPot2Pos[2]), lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());*/

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	ID3D11ShaderResourceView* shadowMaps[2];
	shadowMaps[0] = shadowMap[0]->getDepthMapSRV();
	shadowMaps[1] = shadowMap[1]->getDepthMapSRV();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMaps, light);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(teaPot1Pos[0], teaPot1Pos[1], teaPot1Pos[2]), viewMatrix, projectionMatrix, textureMgr->getTexture(L""), shadowMaps, light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	/*model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(teaPot2Pos[0], teaPot2Pos[1], teaPot2Pos[2]), viewMatrix, projectionMatrix, textureMgr->getTexture(L""), shadowMaps, light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());*/

	//rendering a plane to show the light frustum
	worldMatrix = renderer->getWorldMatrix();
	XMVECTOR upVector = XMVectorSet(0.0f ,1.f, 0.f, 0.f);
	XMFLOAT3* lightDir = new XMFLOAT3(-lightDirection[0][0], -lightDirection[0][1], -lightDirection[0][2]);
	XMVECTOR normalisedLightDir = XMVector3Normalize(XMLoadFloat3(lightDir));
	XMVECTOR rotationAxis = XMVector3Cross(upVector, normalisedLightDir);
	float angleToRotate = acosf(XMVectorGetX(XMVector3Dot(upVector, normalisedLightDir)));
	XMMATRIX rotMatrix = XMMatrixRotationAxis(rotationAxis, angleToRotate);
	//XMMATRIX rotMatrix = XMMatrixLookAtLH(XMLoadFloat3(&light[0]->getPosition()), XMLoadFloat3(lightDir), upVector);
	//XMMATRIX flip = XMMatrixRotationY(XM_PI);

	XMMATRIX scaling = XMMatrixScaling(sceneWidth, sceneHeight, 1.f);

	XMMATRIX offsetMatrix = XMMatrixTranslation(sceneWidth*0.5, sceneHeight*0.5, 0.f);
	XMMATRIX translationToLightPos = XMMatrixTranslation(lightPlanePosition[0][0], lightPlanePosition[0][1], lightPlanePosition[0][2]);

	XMMATRIX planeWorldMatrix = worldMatrix * scaleMatrix * rotMatrix * /*offsetMatrix **/ translationToLightPos;
	debugPlaneMesh[0]->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), planeWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L""));
	textureShader->render(renderer->getDeviceContext(), debugPlaneMesh[0]->getIndexCount());

	//rendering the shadowmap to an orthomesh
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);
	debugOrthoMesh[0]->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap[0]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), debugOrthoMesh[0]->getIndexCount());
	debugOrthoMesh[1]->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap[1]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), debugOrthoMesh[1]->getIndexCount());
	renderer->setZBuffer(true);

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

	ImGui::Text("Change the direction for the directional light 1");
	ImGui::SliderFloat3("direction 1", lightDirection[0], -1, 1, "%.0f");
	light[0]->setDirection(lightDirection[0][0], lightDirection[0][1], lightDirection[0][2]);
	ImGui::Text("Change the position for the directional light plane 1");
	ImGui::SliderFloat3("position 1", lightPlanePosition[0], -100, 100, "%.0f");
	light[0]->setPosition(lightPlanePosition[0][0], lightPlanePosition[0][1], lightPlanePosition[0][2]);

	ImGui::Text("Change the direction for the directional light 2");
	ImGui::SliderFloat3("direction 2", lightDirection[1], -1, 1, "%.0f");
	light[1]->setDirection(lightDirection[1][0], lightDirection[1][1], lightDirection[1][2]);
	ImGui::Text("Change the position for the directional light plane 2");
	ImGui::SliderFloat3("position 2", lightPlanePosition[1], -100, 100, "%.0f");
	light[1]->setPosition(lightPlanePosition[1][0], lightPlanePosition[1][1], lightPlanePosition[1][2]);

	ImGui::Text("Change the position for Tea pot 1");
	ImGui::SliderFloat3("teapot 1", teaPot1Pos, -100, 100, "%.2f");
	ImGui::Text("Change the position for Tea pot 2");
	ImGui::SliderFloat3("teapot 2", teaPot2Pos, -100, 100, "%.2f");


	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

