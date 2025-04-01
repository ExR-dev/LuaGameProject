#include "stdafx.h"
#include "CompassBehaviour.h"
#include "Scene.h"
#include "ErrMsg.h"

bool CompassBehaviour::Start()
{
	if (_name == "")
		_name = "CompassBehaviour";

	Content* content = GetScene()->GetContent();

	DirectX::XMFLOAT3A scale = { 0.1f, 0.1f, 0.1f };

	// Compass body
	{
		UINT meshID = content->GetMeshID("Mesh_CompassFlipped");
		Material mat = Material::MakeMat(
			content->GetTextureID("Tex_Compass_SpecularMaterial_Diffuse"),
			content->GetTextureMapID("Compass_SpecularMaterial_Normal"),
			content->GetTextureMapID("Compass_SpecularMaterial_Specular"),
			content->GetTextureMapID("Compass_SpecularMaterial_Glossiness"),
			content->GetTextureMapID("White")
		);
		mat.vsID = content->GetShaderID("VS_Geometry");

		if (!GetScene()->CreateMeshEntity(&_compassBodyMesh, "Compass Body Mesh", meshID, mat, false, false, true))
		{
			ErrMsg("Failed to initialize compass");
			return false;
		}
		_compassBodyMesh->SetSerialization(false);
		_compassBodyMesh->SetParent(GetEntity());
		_compassBodyMesh->GetTransform()->SetScale(scale);
	}

	// Compass needle
	{
		UINT meshID = content->GetMeshID("Mesh_CompassNeedle");
		Material mat = Material::MakeMat(
			content->GetTextureID("Tex_Stalagmites_Large_2"),
			CONTENT_NULL,
			CONTENT_NULL,
			CONTENT_NULL,
			content->GetTextureID("Tex_Red")
		);
		mat.vsID = content->GetShaderID("VS_Geometry");

		if (!GetScene()->CreateMeshEntity(&_compassNeedleMesh, "Compass Needle Mesh", meshID, mat, false, false, true))
		{
			ErrMsg("Failed to initialize compass needle");
			return false;
		}
		_compassNeedleMesh->SetSerialization(false);
		_compassNeedleMesh->SetParent(_compassBodyMesh);
		_compassBodyMesh->GetTransform()->SetScale(scale);

		_compassNeedle = _compassNeedleMesh->GetTransform();
	}

	return true;
}

bool CompassBehaviour::LateUpdate(Time& time, const Input& input)
{
	float playerRotationY = GetScene()->GetPlayer()->GetTransform()->GetEuler(World).y;
	_compassNeedle->SetEuler({ 0.0f, DirectX::XM_PI - playerRotationY, 0.0f });

	return true;
}

void CompassBehaviour::OnEnable()
{
	_compassBodyMesh->Enable();
	_compassNeedleMesh->Enable();
}

void CompassBehaviour::OnDisable()
{
	_compassBodyMesh->Disable();
	_compassNeedleMesh->Disable();
}
