
Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture[2] : register(t1);

SamplerState diffuseSampler  : register(s0);
SamplerState shadowSampler : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 ambient[2];
	float4 diffuse[2];
	float4 direction[2];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPos[2] : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
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
    float depthValue = sMap.Sample(shadowSampler, uv).r;
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
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);

	// Calculate the projected texture coordinates.
    float2 pTexCoord[2];
    
    pTexCoord[0] = getProjectiveCoords(input.lightViewPos[0]);
    // Shadow test. Is or isn't in shadow
    if (hasDepthData(pTexCoord[0]))
    {
        // Has depth map data
        if (!isInShadow(depthMapTexture[0], pTexCoord[0], input.lightViewPos[0], shadowMapBias))
        {
            // is NOT in shadow, therefore light
            colour += calculateLighting(-direction[0].xyz, input.normal, diffuse[0]);
        }
    }

    pTexCoord[1] = getProjectiveCoords(input.lightViewPos[1]);
    // Shadow test. Is or isn't in shadow
    if (hasDepthData(pTexCoord[1]))
    {
        // Has depth map data
        if (!isInShadow(depthMapTexture[1], pTexCoord[1], input.lightViewPos[1], shadowMapBias))
        {
            // is NOT in shadow, therefore light
            colour += calculateLighting(-direction[1].xyz, input.normal, diffuse[1]);
        }
    }
    
    //colour = saturate(colour + ambient[1] + ambient[0]);

    
    return saturate(colour) * textureColour;
}