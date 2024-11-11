// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;


class ShadowShader : public BaseShader
{
private:
	static const int LIGHT_COUNT = 2;

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[LIGHT_COUNT];
		XMMATRIX lightProjection[LIGHT_COUNT];
	};

	struct LightBufferType
	{
		XMFLOAT4 type[LIGHT_COUNT];
		XMFLOAT4 ambient[LIGHT_COUNT];
		XMFLOAT4 diffuse[LIGHT_COUNT];
		XMFLOAT4 position[LIGHT_COUNT];
		XMFLOAT4 direction[LIGHT_COUNT];
		XMFLOAT4 specular[LIGHT_COUNT];
		XMFLOAT4 specularPower[LIGHT_COUNT];
		XMFLOAT4 attenuationConstant[LIGHT_COUNT];
		XMFLOAT4 attenuationLinear[LIGHT_COUNT];
		XMFLOAT4 attenuationQuadratic[LIGHT_COUNT];
		XMFLOAT4 spotCutoffAngle[LIGHT_COUNT];
		XMFLOAT4 spotOuterCutoffAngle[LIGHT_COUNT];
	};


	struct CamBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection,
		ID3D11ShaderResourceView* texture, ShadowMap* depthMap[], Light* light[], Camera* camera);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* camBuffer;
};

#endif