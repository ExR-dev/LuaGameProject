#include "stdafx.h"
#include "Scene.h"
#include "BreadcrumbBehaviour.h"
#include "FlashlightBehaviour.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

BreadcrumbBehaviour::BreadcrumbBehaviour(BreadcrumbColor color) : _color(color)
{

}

bool BreadcrumbBehaviour::Start()
{
	if (_name == "")
		_name = "BreadcrumbBehaviour"; // For categorization in ImGui.

	Content *content = GetScene()->GetContent();

	std::string crumbColorName = "";
	switch (_color)
	{
	case BreadcrumbColor::Red:
		crumbColorName = "Red";
		break;

	case BreadcrumbColor::Green:
		crumbColorName = "Green";
		break;

	case BreadcrumbColor::Blue:
		crumbColorName = "Blue";
		break;
	}

	// Create a rock mesh behaviour
	UINT meshID = content->GetMeshID("Mesh_Breadcrumb");
	BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();

	Material mat;
	mat.textureID = content->GetTextureID(std::format("Tex_Breadcrumb_{}", crumbColorName));
	mat.normalID = content->GetTextureMapID("TexMap_Breadcrumb_Normal");
	mat.specularID = content->GetTextureMapID(std::format("TexMap_Breadcrumb_{}_Specular", crumbColorName));
	mat.glossinessID = content->GetTextureMapID("TexMap_Breadcrumb_Glossiness");
	mat.occlusionID = content->GetTextureMapID("TexMap_Breadcrumb_Occlusion");
	mat.lightID = content->GetTextureID(std::format("Tex_Breadcrumb_{}_Light", crumbColorName));

	MeshBehaviour *meshBehaviour = new MeshBehaviour(bounds, meshID, &mat);
	if (!meshBehaviour->Initialize(GetEntity()))
	{
		ErrMsg("Failed to create mesh!");
		return false;
	}
	meshBehaviour->SetSerialization(false);

	// Create a billboard mesh behaviour as a child entity
	mat = {};
	mat.textureID = content->GetTextureID(std::format("Tex_Flare_{}", crumbColorName));
	mat.ambientID = content->GetTextureID(std::format("Tex_{}", crumbColorName));

	Entity *entity;
	if (!GetScene()->CreateBillboardMeshEntity(&entity, "Flare Billboard Mesh", mat, 0.0f, 0.25f, 0.01f))
	{
		ErrMsg("Failed to create billboard mesh entity!");
		return false;
	}
	entity->SetParent(GetEntity());
	entity->SetSerialization(false);

	entity->GetBehaviourByType<BillboardMeshBehaviour>(_flare);
	_flare->GetTransform()->Move({ 0.0f, 0.025f, 0.0f }, Local);

#ifdef DEBUG_BUILD
	GetEntity()->SetSerialization(false);
#endif
	return true;
}

bool BreadcrumbBehaviour::Update(Time &time, const Input &input)
{
	Scene *scene = GetScene();

	if (!_flashlight)
	{
		Entity *flashlightEntity = scene->GetSceneHolder()->GetEntityByName("Flashlight");

		if (!flashlightEntity)
			return true;

		if (!flashlightEntity->GetBehaviourByType<FlashlightBehaviour>(_flashlight))
			return true;

		if (!_flashlight)
			return true;
	}

	SpotLightBehaviour *spotlight = _flashlight->GetLight();
	if (!spotlight->IsEnabled())
	{
		_flare->SetSize(0.0f);
		return true;
	}

	XMFLOAT3A breadcrumbPos = GetTransform()->GetPosition(World);

	if (!spotlight->ContainsPoint(breadcrumbPos))
	{
		_flare->SetSize(0.0f);
		return true;
	}

	XMVECTOR crumbPos = Load(breadcrumbPos);
	XMVECTOR lightPos = Load(spotlight->GetTransform()->GetPosition(World));
	XMVECTOR lightDir = Load(spotlight->GetTransform()->GetForward(World));
	XMVECTOR crumbDir = XMVector3Normalize(crumbPos - lightPos);

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(lightDir, crumbDir));
	float lightAngle = spotlight->GetShadowCamera()->GetFOV();

	float flareStrength = 1.0f - std::clamp(angle / lightAngle, 0.0f, 1.0f);

	flareStrength = pow(flareStrength, 3.0f);
	_flare->SetSize(flareStrength * 0.3f);
	return true;
}

bool BreadcrumbBehaviour::Serialize(std::string *code) const
{
	*code += "BreadcrumbBehaviour(";
	*code += std::to_string((UINT)_color) + " ";
	*code += ")";
	return true;
}
bool BreadcrumbBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::istringstream stream(code);
	std::vector<UINT> crumb;

	std::string value;
	while (stream >> value)
	{  // Automatically handles spaces correctly
		UINT attribute = std::stoul(value);
		crumb.push_back(attribute);
	}

	_color = (BreadcrumbColor)crumb[0];

	return true;
}
