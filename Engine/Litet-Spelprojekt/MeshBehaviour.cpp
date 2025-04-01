#include "stdafx.h"
#include "MeshBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "RenderQueuer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

MeshBehaviour::MeshBehaviour(const BoundingOrientedBox &bounds, bool isTransparent, bool castShadows, bool isOverlay)
{
	_bounds = bounds;
	_isTransparent = isTransparent;
	_castShadows = castShadows;
	_isOverlay = isOverlay;
}
MeshBehaviour::MeshBehaviour(const BoundingOrientedBox &bounds, UINT meshID, const Material *material, bool isTransparent, bool castShadows, bool isOverlay)
{
	_bounds = bounds;
	_meshID = meshID;
	_material = material;
	_isTransparent = isTransparent;
	_castShadows = castShadows;
	_isOverlay = isOverlay;
}

bool MeshBehaviour::Start()
{
	if (_name == "")
		_name = "MeshBehaviour"; // For categorization in ImGui.

	bool invalidMesh = false;
	bool invalidMaterial = false;

	if (_meshID == CONTENT_NULL)
		invalidMesh = true;

	if (_material)
	{
		if (!ValidateMaterial(&_material))
			invalidMaterial = true;
	}
	else
	{
		invalidMaterial = true;
	}

	auto device = GetScene()->GetDevice();
	auto content = GetScene()->GetContent();

	if (invalidMesh && invalidMaterial)
	{
		_meshID = content->GetMeshID("Mesh_Error");
		_material = content->GetErrorMaterial();
	}
	else if (invalidMesh)
	{
		_meshID = content->GetMeshID("Mesh_Fallback");
	}
	else if (invalidMaterial)
	{
		_material = content->GetDefaultMaterial();
	}

	MaterialProperties materialProperties = { };
	materialProperties.sampleNormal = _material->normalID != CONTENT_NULL;
	materialProperties.sampleSpecular = _material->specularID != CONTENT_NULL;
	materialProperties.sampleGlossiness = _material->glossinessID != CONTENT_NULL;
	materialProperties.sampleLight = _material->lightID != CONTENT_NULL;
	materialProperties.sampleAmbient = _material->ambientID != CONTENT_NULL;
	materialProperties.sampleOcclusion = _material->occlusionID != CONTENT_NULL;
	materialProperties.alphaCutoff = _alphaCutoff;

	if (!_materialBuffer.Initialize(device, sizeof(MaterialProperties), &materialProperties))
	{
		ErrMsg("Failed to initialize material buffer!");
		return false;
	}

	XMFLOAT4A pos = To4(GetTransform()->GetPosition());
	if (!_posBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4A), &pos))
	{
		ErrMsg("Failed to initialize position buffer!");
		return false;
	}

	return true;
}

bool MeshBehaviour::Update(Time &time, const Input &input)
{
	if (_updatePosBuffer)
	{
		BoundingOrientedBox worldSpaceBounds;
		StoreBounds(worldSpaceBounds);
		const XMFLOAT4A center = { worldSpaceBounds.Center.x, worldSpaceBounds.Center.y, worldSpaceBounds.Center.z, 0.0f };

		if (!_posBuffer.UpdateBuffer(GetScene()->GetContext(), &center))
		{
			ErrMsg("Failed to update position buffer!");
			return false;
		}

		_updatePosBuffer = false;
	}

	if (_updateMatBuffer)
	{
		MaterialProperties materialProperties = { };
		materialProperties.sampleNormal = _material->normalID != CONTENT_NULL;
		materialProperties.sampleSpecular = _material->specularID != CONTENT_NULL;
		materialProperties.sampleGlossiness = _material->glossinessID != CONTENT_NULL;
		materialProperties.sampleLight = _material->lightID != CONTENT_NULL;
		materialProperties.sampleAmbient = _material->ambientID != CONTENT_NULL;
		materialProperties.sampleOcclusion = _material->occlusionID != CONTENT_NULL;

		if (!_materialBuffer.UpdateBuffer(GetScene()->GetContext(), &materialProperties))
		{
			ErrMsg("Failed to update material buffer!");
			return false;
		}

		_updateMatBuffer = false;
	}

	return true;
}

bool MeshBehaviour::Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo)
{
	if (rendererInfo.shadowCamera && !_castShadows)
		return true;

	const ResourceGroup resources = { _meshID, _material, _castShadows, _isOverlay };
	const RenderInstance instance = {
		dynamic_cast<Behaviour*>(this),
		sizeof(MeshBehaviour)
	};

	if (_isTransparent)
		queuer.QueueTransparent(resources, instance);
	else
		queuer.QueueGeometry(resources, instance);

	return true;
}

#ifdef USE_IMGUI
bool MeshBehaviour::RenderUI()
{
	Content *content = GetScene()->GetContent();

	if (ImGui::RadioButton("Overlay", _isOverlay))
		_isOverlay = !_isOverlay;

	if (ImGui::RadioButton("Transparent", _isTransparent))
		_isTransparent = !_isTransparent;

	if (!_isTransparent)
		if (ImGui::RadioButton("Cast Shadows", _castShadows))
			_castShadows = !_castShadows;
	
	if (ImGui::SliderFloat("Alpha Cutoff", &_alphaCutoff, 0, 1))
		_updateMatBuffer = true;

	UINT startVS, endVS;
	UINT startPS, endPS;

	content->GetShaderTypeRange("VS_", startVS, endVS);
	content->GetShaderTypeRange("PS_", startPS, endPS);

	if (ImGui::Button("Print texture maps"))
		for (int i = 0; i < content->GetTextureMapCount(); i++)
			std::cout << "(" << i << "): '" << content->GetTextureMapName(i) << "'" << std::endl;

	int
		inputMeshID = (int)_meshID,
		inputTexID = (int)_material->textureID,
		inputNormID = (int)_material->normalID,
		inputSpecID = (int)_material->specularID,
		inputGlossID = (int)_material->glossinessID,
		inputAmbID = (int)_material->ambientID,
		inputLightID = (int)_material->lightID,
		inputOcclusionID = (int)_material->occlusionID,
		inputHeightID = (int)_material->heightID,
		inputSampID = (int)_material->samplerID,
		inputVSID = (int)_material->vsID,
		inputPSID = (int)_material->psID;

	static int previewSize = 128;
	ImGui::Text("Preview Size: ");
	ImGui::SameLine();
	ImGui::InputInt("##PreviewSize", &previewSize);
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	ImVec2 previewVec = ImVec2(abs(static_cast<float>(previewSize)), abs(static_cast<float>(previewSize)));

	std::vector<std::string> meshNames, textureNames, textureMapNames, shaderNames;
	content->GetMeshNames(&meshNames);
	content->GetTextureNames(&textureNames);
	content->GetTextureMapNames(&textureMapNames);
	content->GetShaderNames(&shaderNames);

	ImGui::PushID("Mats");
	bool isChanged = false;
	int id = 1;

	// Mesh
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Mesh: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetMeshName((UINT)inputMeshID).c_str()))
		{
			for (UINT i = 0; i < meshNames.size(); i++)
			{
				bool isSelected = (inputMeshID == i);
				if (ImGui::Selectable(meshNames[i].c_str(), isSelected))
				{
					inputMeshID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Texture map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Texture: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureName((UINT)inputTexID).c_str()))
		{
			for (UINT i = 0; i < textureNames.size(); i++)
			{
				bool isSelected = (inputTexID == i);
				if (ImGui::Selectable(textureNames[i].c_str(), isSelected))
				{
					inputTexID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Image((ImTextureID)content->GetTexture((UINT)inputTexID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Normal map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Normal: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureMapName((UINT)inputNormID).c_str()))
		{
			{
				bool isSelected = (inputNormID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputNormID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureMapNames.size(); i++)
			{
				if (textureMapNames[i].find("_Normal") == std::string::npos)
					continue;

				bool isSelected = (inputNormID == i);
				if (ImGui::Selectable(textureMapNames[i].c_str(), isSelected))
				{
					inputNormID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		std::string name = content->GetTextureMapName((UINT)inputNormID);
		if (content->GetTextureMap((UINT)inputNormID) && name != "Uninitialized")
			ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputNormID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Specular map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Specular: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureMapName((UINT)inputSpecID).c_str()))
		{
			{
				bool isSelected = (inputSpecID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputSpecID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureMapNames.size(); i++)
			{
				if (textureMapNames[i].find("_Specular") == std::string::npos)
					continue;

				bool isSelected = (inputSpecID == i);
				if (ImGui::Selectable(textureMapNames[i].c_str(), isSelected))
				{
					inputSpecID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		std::string name = content->GetTextureMapName((UINT)inputSpecID);
		if (content->GetTextureMap((UINT)inputSpecID) && name != "Uninitialized")
			ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputSpecID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Glossiness map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Glossiness: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureMapName((UINT)inputGlossID).c_str()))
		{
			{
				bool isSelected = (inputGlossID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputGlossID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureMapNames.size(); i++)
			{
				if (textureMapNames[i].find("_Glossiness") == std::string::npos)
					continue;

				bool isSelected = (inputGlossID == i);
				if (ImGui::Selectable(textureMapNames[i].c_str(), isSelected))
				{
					inputGlossID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		std::string name = content->GetTextureMapName((UINT)inputGlossID);
		if (content->GetTextureMap((UINT)inputGlossID) && name != "Uninitialized")
			ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputGlossID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Ambient map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Ambient: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureName((UINT)inputAmbID).c_str()))
		{
			{
				bool isSelected = (inputAmbID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputAmbID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureNames.size(); i++)
			{
				bool isSelected = (inputAmbID == i);
				if (ImGui::Selectable(textureNames[i].c_str(), isSelected))
				{
					inputAmbID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Image((ImTextureID)content->GetTexture((UINT)inputAmbID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Light map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Baked Light: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureName((UINT)inputLightID).c_str()))
		{
			{
				bool isSelected = (inputLightID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputLightID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureNames.size(); i++)
			{
				bool isSelected = (inputLightID == i);
				if (ImGui::Selectable(textureNames[i].c_str(), isSelected))
				{
					inputLightID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Image((ImTextureID)content->GetTexture((UINT)inputLightID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Occlusion map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Ambient Occlusion: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureMapName((UINT)inputOcclusionID).c_str()))
		{
			{
				bool isSelected = (inputOcclusionID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputOcclusionID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureMapNames.size(); i++)
			{
				if (textureMapNames[i].find("_Glossiness") == std::string::npos)
					continue;

				bool isSelected = (inputOcclusionID == i);
				if (ImGui::Selectable(textureMapNames[i].c_str(), isSelected))
				{
					inputOcclusionID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		std::string name = content->GetTextureMapName((UINT)inputOcclusionID);
		if (content->GetTextureMap((UINT)inputOcclusionID) && name != "Uninitialized")
			ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputOcclusionID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Height map
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Height: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetTextureMapName((UINT)inputHeightID).c_str()))
		{
			{
				bool isSelected = (inputHeightID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputHeightID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < textureMapNames.size(); i++)
			{
				if (textureMapNames[i].find("_Glossiness") == std::string::npos)
					continue;

				bool isSelected = (inputHeightID == i);
				if (ImGui::Selectable(textureMapNames[i].c_str(), isSelected))
				{
					inputHeightID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		std::string name = content->GetTextureMapName((UINT)inputHeightID);
		if (content->GetTextureMap((UINT)inputHeightID) && name != "Uninitialized")
			ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputHeightID)->GetSRV(), previewVec);
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Sampler
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());
		ImGui::Text(("Sampler: " + content->GetSamplerName((UINT)inputSampID)).c_str());
		if (ImGui::InputInt("", &inputSampID))
			isChanged = true;
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Vertex Shader
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Vertex Shader: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetShaderName((UINT)inputVSID).c_str()))
		{
			{
				bool isSelected = (inputVSID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputVSID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < shaderNames.size(); i++)
			{
				if (shaderNames[i].find("VS_") == std::string::npos)
					continue;

				bool isSelected = (inputVSID == i);
				if (ImGui::Selectable(shaderNames[i].c_str(), isSelected))
				{
					inputVSID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// Pixel Shader
	{
		ImGui::PushID(("Param " + std::to_string(id++)).c_str());

		ImGui::Text("Pixel Shader: "); ImGui::SameLine();
		if (ImGui::BeginCombo("", content->GetShaderName((UINT)inputPSID).c_str()))
		{
			{
				bool isSelected = (inputPSID == -1);
				if (ImGui::Selectable("None", isSelected))
				{
					inputPSID = -1;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			for (UINT i = 0; i < shaderNames.size(); i++)
			{
				if (shaderNames[i].find("PS_") == std::string::npos)
					continue;

				bool isSelected = (inputPSID == i);
				if (ImGui::Selectable(shaderNames[i].c_str(), isSelected))
				{
					inputPSID = i;
					isChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
	ImGui::Separator();

	ImGui::PopID();

	if (isChanged)
	{
		inputMeshID += (int)content->GetMeshCount();
		inputMeshID %= (int)content->GetMeshCount();

		inputTexID += (int)content->GetTextureCount();
		inputTexID %= (int)content->GetTextureCount();

		if (inputNormID != -1)
		{
			if (inputNormID < 0)
				inputNormID++;

			inputNormID += (int)content->GetTextureMapCount();
			inputNormID %= (int)content->GetTextureMapCount();
		}

		if (inputSpecID != -1)
		{
			if (inputSpecID < 0)
				inputSpecID++;

			inputSpecID += (int)content->GetTextureMapCount();
			inputSpecID %= (int)content->GetTextureMapCount();
		}

		if (inputGlossID != -1)
		{
			if (inputGlossID < 0)
				inputGlossID++;

			inputGlossID += (int)content->GetTextureMapCount();
			inputGlossID %= (int)content->GetTextureMapCount();
		}

		if (inputAmbID != -1)
		{
			if (inputAmbID < 0)
				inputAmbID++;

			inputAmbID += (int)content->GetTextureCount();
			inputAmbID %= (int)content->GetTextureCount();
		}

		if (inputLightID != -1)
		{
			if (inputLightID < 0)
				inputLightID++;

			inputLightID += (int)content->GetTextureCount();
			inputLightID %= (int)content->GetTextureCount();
		}

		if (inputOcclusionID != -1)
		{
			if (inputOcclusionID < 0)
				inputOcclusionID++;

			inputOcclusionID += (int)content->GetTextureMapCount();
			inputOcclusionID %= (int)content->GetTextureMapCount();
		}

		if (inputHeightID != -1)
		{
			if (inputHeightID < 0)
				inputHeightID++;

			inputHeightID += (int)content->GetTextureMapCount();
			inputHeightID %= (int)content->GetTextureMapCount();
		}

		if (inputSampID != -1)
		{
			if (inputSampID < 0)
				inputSampID++;

			inputSampID += (int)content->GetSamplerCount();
			inputSampID %= (int)content->GetSamplerCount();
		}
		
		if (inputVSID != -1)
		{
			if (_material->vsID == -1)
			{
				if (inputVSID < -1)
					inputVSID = endVS;
				else
					inputVSID = startVS;
			}
			else
			{
				if (static_cast<UINT>(inputVSID) < startVS)
					inputVSID = endVS;
				else if (static_cast<UINT>(inputVSID) > endVS)
					inputVSID = startVS;
			}
		}
		
		if (inputPSID != -1)
		{
			if (_material->psID == -1)
			{
				if (inputPSID < -1)
					inputPSID = endPS;
				else
					inputPSID = startPS;
			}
			else
			{
				if (static_cast<UINT>(inputPSID) < startPS)
					inputPSID = endPS;
				else if (static_cast<UINT>(inputPSID) > endPS)
					inputPSID = startPS;
			}
		}

		if (inputMeshID != _meshID)
			SetMesh((UINT)inputMeshID, true);

		Material newMat;
		newMat.textureID	= (UINT)inputTexID;
		newMat.normalID		= (UINT)inputNormID;
		newMat.specularID	= (UINT)inputSpecID;
		newMat.glossinessID	= (UINT)inputGlossID;
		newMat.ambientID	= (UINT)inputAmbID;
		newMat.lightID		= (UINT)inputLightID;
		newMat.occlusionID	= (UINT)inputOcclusionID;
		newMat.heightID		= (UINT)inputHeightID;
		newMat.samplerID	= (UINT)inputSampID;
		newMat.vsID			= (UINT)inputVSID;
		newMat.psID			= (UINT)inputPSID;

		if (!SetMaterial(&newMat))
		{
			ErrMsg("Failed to set material!");
			return false;
		}

		_updateMatBuffer = true;
	}

	return true;
}
#endif

bool MeshBehaviour::BindBuffers()
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const wmBuffer = GetTransform()->GetConstantBuffer();
	context->VSSetConstantBuffers(0, 1, &wmBuffer);

	ID3D11Buffer *const materialBuffer = _materialBuffer.GetBuffer();
	context->PSSetConstantBuffers(2, 1, &materialBuffer);

	ID3D11Buffer *const posBuffer = _posBuffer.GetBuffer();
	context->HSSetConstantBuffers(0, 1, &posBuffer);

	return true;
}

void MeshBehaviour::OnDirty()
{
	_updatePosBuffer = true;
	_recalculateBounds = true;
}

bool MeshBehaviour::Serialize(std::string *code) const 
{
	Content *content = GetScene()->GetContent();
	std::string meshName = GetScene()->GetContent()->GetMeshName(_meshID);
	std::replace(meshName.begin(), meshName.end(), ' ', '_');

	*code += "MeshBehaviour(" +
		meshName + " " +
		std::to_string(_isTransparent) + " ";

	std::string textureName;

	textureName = content->GetTextureName(((UINT *)_material)[0]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureMapName(((UINT *)_material)[1]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureMapName(((UINT *)_material)[2]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureMapName(((UINT *)_material)[3]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureName(((UINT *)_material)[4]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureMapName(((UINT *)_material)[5]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureName(((UINT *)_material)[6]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " ";

	textureName = content->GetTextureMapName(((UINT *)_material)[7]);
	std::replace(textureName.begin(), textureName.end(), ' ', '_');
	*code += textureName + " " +
		std::to_string(((UINT *)_material)[8]) + " " +
		std::to_string(((UINT *)_material)[9]) + " " +
		std::to_string(((UINT *)_material)[10]) + " )";

	return true;
}
bool MeshBehaviour::Deserialize(const std::string &code)
{
	Content *content = GetScene()->GetContent();

	// Standard code for all behaviours deserialize
	std::istringstream stream(code);
	std::vector<std::string> textureNames;

	std::string value;
	stream >> value;
	_meshID = GetScene()->GetContent()->GetMeshID(value);

	stream >> value;
	_isTransparent = std::stoul(value);

	while (stream >> value) 
	{
		textureNames.push_back(value);
	}

	// This section is for the initialization of the behaviour
	Material mat = Material();
	if (textureNames.at(0) != "Uninitialized")
		((UINT *)&mat)[0] = content->GetTextureID(textureNames.at(0));
	if (textureNames.at(1) != "Uninitialized")
		((UINT *)&mat)[1] = content->GetTextureMapID(textureNames.at(1));
	if (textureNames.at(2) != "Uninitialized")
		((UINT *)&mat)[2] = content->GetTextureMapID(textureNames.at(2));
	if (textureNames.at(3) != "Uninitialized")
		((UINT *)&mat)[3] = content->GetTextureMapID(textureNames.at(3));
	if (textureNames.at(4) != "Uninitialized")
		((UINT *)&mat)[4] = content->GetTextureID(textureNames.at(4));
	if (textureNames.at(5) != "Uninitialized")
		((UINT *)&mat)[5] = content->GetTextureMapID(textureNames.at(5));
	if (textureNames.at(6) != "Uninitialized")
		((UINT *)&mat)[6] = content->GetTextureID(textureNames.at(6));
	if (textureNames.at(7) != "Uninitialized")
		((UINT *)&mat)[7] = content->GetTextureMapID(textureNames.at(7));
	((UINT *)&mat)[8] = std::stoul(textureNames.at(8));
	((UINT *)&mat)[9] = std::stoul(textureNames.at(9));
	((UINT *)&mat)[10] = std::stoul(textureNames.at(10));

	SetMesh(_meshID, true);
	if (!SetMaterial(&mat))
	{
		ErrMsg("Failed to set material!");
		return false;
	}

	return true;
}


bool MeshBehaviour::ValidateMaterial(const Material **material)
{
	if (!material)
	{
		ErrMsg("**Material is nullptr!");
		return false;
	}

	if (!*material)
	{
		ErrMsg("*Material is nullptr!");
		return false;
	}

	(*material) = GetScene()->GetContent()->GetOrAddMaterial(**material);
	return true;
}

void MeshBehaviour::SetMesh(UINT meshID, bool updateBounds)
{
	_meshID = meshID;
	_updateMatBuffer = true;

	if (updateBounds)
	{
		BoundingOrientedBox newBounds = GetScene()->GetContent()->GetMesh(meshID)->GetBoundingOrientedBox();
		SetBounds(newBounds);
		GetEntity()->SetEntityBounds(newBounds);
	}

}
bool MeshBehaviour::SetMaterial(const Material *material)
{
	if (IsInitialized())
	{
		if (!ValidateMaterial(&material))
		{
			ErrMsg("Failed to validate material!");
			return false;
		}
	}

	_material = material;
	_updateMatBuffer = true;
	return true;
}
void MeshBehaviour::SetTransparent(bool state)
{
	_isTransparent = state;
}
void MeshBehaviour::SetOverlay(bool state)
{
	_isOverlay = state;
}
void MeshBehaviour::SetCastShadows(bool state)
{
	_castShadows = state;
}
void MeshBehaviour::SetAlphaCutoff(float value)
{
	_alphaCutoff = value;
	_updateMatBuffer = true;
}
void MeshBehaviour::SetBounds(BoundingOrientedBox &newBounds)
{
	_bounds = newBounds;
	_recalculateBounds = true;
}

UINT MeshBehaviour::GetMesh() const
{
	return _meshID;
}
const Material *MeshBehaviour::GetMaterial() const
{
	return _material;
}

void MeshBehaviour::StoreBounds(BoundingOrientedBox &meshBounds)
{
	if (_recalculateBounds)
	{
		XMFLOAT4X4A worldMatrix = GetTransform()->GetWorldMatrix();
		
		_bounds.Transform(_transformedBounds, Load(&worldMatrix));
		_recalculateBounds = false;
	}

	meshBounds = _transformedBounds;
}
