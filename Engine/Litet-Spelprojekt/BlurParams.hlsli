static const float SAMPLE_STEP = 0.001f;
static const float WEIGHT_COUNT = 3;
/*static const float GAUSS_WEIGHTS[] = {
    0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162
};*/
static const float GAUSS_WEIGHTS[] = {
    0.7788081181217, 0.2165377067336, 0.0046541751447
};


Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
Texture2D<float> Depth : register(t1);


float4 LinearBlur(int2 uv, int kernelSize, int2 direction, int2 dimensions)
{
	float4 sum = 0;
	
	for (int i = -kernelSize; i <= kernelSize; ++i)
	{
		int2 sampleUV = uv + i * direction;
		sampleUV = clamp(sampleUV, int2(0, 0), dimensions - 1);
		
		sum += Input[sampleUV];
	}
	
	return sum / (2 * kernelSize + 1);
}

float4 GaussianBlur(int2 uv, int2 direction, int2 dimensions)
{
	int2 depthDimensions;
	Depth.GetDimensions(depthDimensions.x, depthDimensions.y);
	
	float2 inputToDepth = float2(depthDimensions) / float2(dimensions);
	float depth = Depth[uv * inputToDepth];
	
	float weightSum = 0;
	float4 sum = 0;
    for (int i = -WEIGHT_COUNT + 1; i < WEIGHT_COUNT; ++i)
	{
		int2 sampleUV = uv + i * direction;
		sampleUV = clamp(sampleUV, int2(0, 0), dimensions - 1);
		
		float sampleDepth = Depth[sampleUV * inputToDepth];
		float offset = abs(sampleDepth - depth);
		
		float weight = GAUSS_WEIGHTS[abs(i)] * lerp(1.0 / max(1, offset), 0.3, 1.0);
		
		weightSum += weight;
		sum += weight * Input[sampleUV];
	}
	
	return sum / weightSum;
}