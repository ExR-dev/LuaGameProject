#include "Common.hlsli"


Texture2D<float4> SceneColor : register(t0);
Texture2D<float4> Emission : register(t1);
Texture2D<float4> Fog : register(t2);

RWTexture2D<unorm float4> BackBufferUAV : register(u0);


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 outDim;
    BackBufferUAV.GetDimensions(outDim.x, outDim.y);
    float2 uv = float2(DTid.xy) / float2(outDim);
    
    const float4 sceneCol = SceneColor.SampleLevel(Sampler, uv, 0);
    const float4 emission = Emission.SampleLevel(Sampler, uv, 0);
    const float4 fog = Fog.SampleLevel(Sampler, uv, 0);
	
	float3 result = sceneCol.xyz + (emission.xyz * emission.w);
    result = ACESFilm(lerp(result, fog.xyz, fog.w));
	result = lerp(result, float3(0.0, 0.0, 0.0), ambient_light.w);
    
    BackBufferUAV[DTid.xy] = float4(result, 1.0);
}