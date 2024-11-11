#define LIGHT_COUNT 2

Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture[LIGHT_COUNT] : register(t1);

SamplerState diffuseSampler  : register(s0);
SamplerState shadowSampler[LIGHT_COUNT] : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 lightType[LIGHT_COUNT];
    float4 ambient[LIGHT_COUNT];
    float4 diffuse[LIGHT_COUNT];
    float4 lightPosition[LIGHT_COUNT];
    float4 lightDirection[LIGHT_COUNT];
    float4 specular[LIGHT_COUNT];
    float4 specularPower[LIGHT_COUNT]; // Float
    float4 attenuationConstant[LIGHT_COUNT]; // Float
    float4 attenuationLinear[LIGHT_COUNT]; // Float
    float4 attenuationQuadratic[LIGHT_COUNT]; // Float
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    // For specular
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    // For shadow
    float4 lightViewPos[LIGHT_COUNT] : TEXCOORD3;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateDirectionalLight(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calculatePointLight(float3 lightPosition, float3 normal, float4 diffuse, float3 worldPos, float attenC, float attenL, float attenQ)
{
    float3 lightDirection = normalize(lightPosition - worldPos);
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);

    float dist = length(lightPosition - worldPos);
    float attenuation = 1 / (attenC + (attenL * dist) +
		(attenQ * pow(dist, 2)));
    return colour * attenuation;
}

float4 calculateSpotLight(float3 lightPosition, float3 lightDirection, float3 normal, float4 diffuse, float3 worldPosition,
	float attenC, float attenL, float attenQ)
{
    float3 lightVector = normalize(lightPosition - worldPosition);
    float colour = max(dot(lightVector, lightDirection), 0.0f);

	// Attenuation
    float dist = length(lightPosition - worldPosition);
    float attenuation = 1 / (attenC + (attenL * dist) +
		(attenQ * pow(dist, 2)));
    colour *= attenuation;

    return colour * diffuse;
}

float4 calcSpecular(float3 lightPosition, float3 normal, float3 viewVector, float4
	specularColour, float specularPower, float3 worldPosition)
{
    float3 lightVector = normalize(lightPosition - worldPosition);
    float3 halfway = normalize(lightVector + viewVector);
    float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
    return saturate(specularColour * specularIntensity);
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler[1], uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f;
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
    float4 lightColour = float4(0, 0, 0, 0);
    float4 specColour = float4(0, 0, 0, 0);

    float finalShadow = 1.0;
    for (int i = 0; i < LIGHT_COUNT; i++)
    {
       float2 pTexCoord = getProjectiveCoords(input.lightViewPos[i]);

       if (hasDepthData(pTexCoord))
       {            
            if (!isInShadow(depthMapTexture[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
                if (lightType[i].x == -1)
                {
                    continue;
                }
                if (lightType[i].x == 0)
                {
                    lightColour += calculateDirectionalLight(-lightDirection[i].xyz, input.normal, diffuse[i]);
                }
                else if (lightType[i].x == 1)
                {
                    lightColour += calculatePointLight(lightPosition[i].xyz, input.normal, diffuse[i], input.worldPosition,
                        attenuationConstant[i].x, attenuationLinear[i].x, attenuationQuadratic[i].x);
                }
                else if (lightType[i].x == 2)
                {
                    lightColour += calculateSpotLight(lightPosition[i].xyz, lightDirection[i].xyz, input.normal, diffuse[i], input.worldPosition,
				        attenuationConstant[i].x, attenuationLinear[i].x, attenuationQuadratic[i].x);
                }
              
                
                specColour += calcSpecular(lightPosition[i].xyz, input.normal, input.viewVector, specular[i], specularPower[i].x, input.worldPosition);
            }                 
       }
    }


    for (int a = 0; a < LIGHT_COUNT; a++)
    {
        lightColour += ambient[a];
    }
    return saturate(lightColour) * textureColour + specColour;
}