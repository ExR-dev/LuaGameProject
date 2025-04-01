#include "Common.hlsli"
#include "BlurParams.hlsli"

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int2 inDim, outDim;
	Input.GetDimensions(inDim.x, inDim.y);
	Output.GetDimensions(outDim.x, outDim.y);
    
	int2 inCoord = int2(float2(DTid.xy) * (float2(inDim) / float2(outDim.xy)));
       
	Output[DTid.xy] = GaussianBlur(inCoord, int2(0, 1), inDim);
}
