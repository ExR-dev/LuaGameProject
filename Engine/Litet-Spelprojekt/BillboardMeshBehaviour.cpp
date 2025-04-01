#include "stdafx.h"
#include "BillboardMeshBehaviour.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

BillboardMeshBehaviour::BillboardMeshBehaviour(const Material &material, 
	float rotation, float normalOffset, float size,
	bool keepUpright, bool isTransparent, bool castShadows, bool isOverlay)
{
	_transparent = isTransparent;
	_castShadows = castShadows;
	_overlay = isOverlay;
	_material = material;
	_rotation = rotation;
	_normalOffset = normalOffset;
	_scale = size;
	_keepUpright = keepUpright;
}

void BillboardMeshBehaviour::SetSize(float size)
{
	_scale = size;
}
void BillboardMeshBehaviour::SetRotation(float rotation)
{
	_rotation = rotation;
}

MeshBehaviour* BillboardMeshBehaviour::GetMeshBehaviour() const
{
	return _meshBehaviour;
}

bool BillboardMeshBehaviour::Start()
{
	if (_name == "")
		_name = "BillboardMeshBehaviour"; // For categorization in ImGui.

	// Create a mesh behaviour as a child entity
	static UINT meshID = GetScene()->GetContent()->GetMeshID("Mesh_Plane");

	Entity *entity;
	if (!GetScene()->CreateMeshEntity(&entity, "Billboard Mesh", meshID, _material, _transparent, _castShadows, _overlay))
	{
		ErrMsg("Failed to create mesh entity!");
		return false;
	}
	entity->SetParent(GetEntity());

	entity->GetBehaviourByType<MeshBehaviour>(_meshBehaviour);

	entity->SetSerialization(false);
	return true;
}

bool BillboardMeshBehaviour::ParallelUpdate(const Time &time, const Input &input)
{
	Scene *scene = GetScene();

	CameraBehaviour *viewCamera = scene->GetViewCamera();
	if (!viewCamera)
		return true;

	// Make the mesh face the camera
	Transform *camTrans = viewCamera->GetTransform();

	XMFLOAT3A billboardPos = GetTransform()->GetPosition(World);
	XMFLOAT3A camPos = camTrans->GetPosition(World);

	XMVECTOR lookAt = XMVector3Normalize(XMVectorSubtract(Load(camPos), Load(billboardPos)));
	XMVECTOR up;

	if (_keepUpright)
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	else
		up = XMVector3Normalize(XMVector3Cross(lookAt, Load(camTrans->GetRight(World))));

	if (_rotation != 0.0f)
	{
		XMMATRIX rotMat = XMMatrixRotationAxis(lookAt, _rotation);
		up = XMVector3Transform(up, rotMat);
	}

	XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, lookAt));
	up = XMVector3Normalize(XMVector3Cross(lookAt, right));

	XMVECTOR billboardPosVec = Load(billboardPos);
	billboardPosVec = XMVectorAdd(billboardPosVec, XMVectorScale(lookAt, _normalOffset));

	XMMATRIX billboardMatrix = XMMatrixIdentity();
	billboardMatrix.r[0] = _scale * right;
	billboardMatrix.r[1] = _scale * up;
	billboardMatrix.r[2] = _scale * lookAt;
	billboardMatrix.r[3] = billboardPosVec;
	billboardMatrix.r[3].m128_f32[3] = 1.0f;

	XMFLOAT4X4A billboardMatrixF;
	Store(billboardMatrixF, billboardMatrix);

	_meshBehaviour->GetTransform()->SetMatrix(billboardMatrixF, World);

	if (_scale >= 0.001f)
		_meshBehaviour->GetTransform()->RotatePitch(90.0f * DEG_TO_RAD);

	return true;
}

#ifdef USE_IMGUI
bool BillboardMeshBehaviour::RenderUI()
{
	ImGui::Checkbox("Keep Upright", &_keepUpright);
	ImGui::InputFloat("Rotation", &_rotation);
	ImGui::InputFloat("Normal Offset", &_normalOffset);
	ImGui::InputFloat("Scale", &_scale);

	return true;
}
#endif

void BillboardMeshBehaviour::OnEnable()
{
	_meshBehaviour->SetEnabled(true);
}
void BillboardMeshBehaviour::OnDisable()
{
	_meshBehaviour->SetEnabled(false);
}

bool BillboardMeshBehaviour::Serialize(std::string *code) const
{
	*code += "BillboardMeshBehaviour(";

	/**code += 
		std::to_string(_keepUpright) + " " +
		std::to_string(_rotation) + " " +
		std::to_string(_normalOffset) + " ";

	std::string meshCode;
	if (!_meshBehaviour->Serialize(&meshCode))
	{
		ErrMsg("Failed to serialize mesh behaviour!");
		return false;
	}

	int meshStart = meshCode.find("(");
	int meshEnd = meshCode.find(")");

	*code += meshCode.substr(meshStart + 1, meshEnd - meshStart - 1);*/

	*code += " )";

	return true;
}
bool BillboardMeshBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	/*std::istringstream stream(code);
	std::vector<UINT> mesh;

	std::string value;
	while (stream >> value)
	{  // Automatically handles spaces correctly
		UINT attribute = std::stoul(value);
		mesh.push_back(attribute);
	}

	_isTransparent = mesh[1];

	// This section is for the initialization of the behaviour
	Material mat;
	std::memcpy(&mat, &mesh[2], sizeof(Material));

	SetMesh(mesh[0], true);
	if (!SetMaterial(&mat))
	{
		ErrMsg("Failed to set material!");
		return false;
	}*/

	return true;
}
