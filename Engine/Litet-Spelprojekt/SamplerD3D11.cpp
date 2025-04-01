#include "stdafx.h"
#include "SamplerD3D11.h"

#include "ErrMsg.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using Microsoft::WRL::ComPtr;


SamplerD3D11::SamplerD3D11(ID3D11Device *device, D3D11_TEXTURE_ADDRESS_MODE adressMode, D3D11_FILTER filter)
{
	if (!Initialize(device, adressMode, filter))
		ErrMsg("Failed to initialize sampler in constructor!");
}

SamplerD3D11::~SamplerD3D11()
{
}

bool SamplerD3D11::Initialize(ID3D11Device *device, D3D11_TEXTURE_ADDRESS_MODE adressMode, D3D11_FILTER filter)
{
	D3D11_SAMPLER_DESC samplerDesc = { };
	//samplerDesc.Filter = anisotropicFiltering ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = adressMode;
	samplerDesc.AddressV = adressMode;
	samplerDesc.AddressW = adressMode;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 2;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	if (FAILED(device->CreateSamplerState(&samplerDesc, _sampler.ReleaseAndGetAddressOf())))
	{
		ErrMsg("Failed to create sampler state!");
		return false;
	}

	return true;
}


ID3D11SamplerState *SamplerD3D11::GetSamplerState() const
{
	return _sampler.Get();
}
