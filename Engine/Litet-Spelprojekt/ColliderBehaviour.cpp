#include "stdafx.h"
#include "ColliderBehaviour.h"
#include "Scene.h"
#include <format>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool ColliderBehaviour::Start()
{
	if (_name == "")
		_name = "ColliderBehaviour"; // For categorization in ImGui.

	_isDirty = true;
    return true;
}

bool ColliderBehaviour::Update(Time &time, const Input &input)
{
	if (!_baseCollider || !_transformedCollider)
		return true;

	if (_isDirty)
	{
		DirectX::XMFLOAT4X4A worldMatrix = GetTransform()->GetWorldMatrix();
		_baseCollider->Transform(worldMatrix, _transformedCollider);

#ifdef DEBUG_BUILD
		if (!_transformedCollider->TransformDebug(GetScene()->GetContext(), time, input))
		{
			ErrMsg("Failed to transform collider debug!");
			return false;
		}
#endif

		_isDirty = false;
	}

	return true;
}

bool ColliderBehaviour::Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo)
{
#ifdef DEBUG_BUILD
	if (_transformedCollider)
		if (!_transformedCollider->RenderDebug(GetScene(), queuer, rendererInfo))
		{
			ErrMsg("Failed to render collider!");
			return false;
		}
#endif
	
	return true;
}

#ifdef USE_IMGUI
bool ColliderBehaviour::RenderUI()
{
	if (ImGui::Checkbox("Render Collider Wireframe", &_transformedCollider->debug))
	{
		if (_transformedCollider->debug && !_transformedCollider->haveDebugEnt)
			if (!_transformedCollider->SetDebugEnt(GetScene(), GetScene()->GetContent()))
			{
				ErrMsg("Failed to create collider debug entity!");
				return false;
			}

		_isDirty = true;
		_toCheck = true;
	}

	bool status = _transformedCollider->GetActive();
	if (ImGui::Checkbox("Collider Actie", &status))
		_transformedCollider->SetActive(status);

	if (!_baseCollider->RenderUI())
	{
		ErrMsg("Failed to render collider UI!");
		return false;
	}

	if (_baseCollider->GetDirty())
	{
		_isDirty = true;
		_toCheck = true;
	}
	return true;
}
#endif

void ColliderBehaviour::OnDirty()
{
	_isDirty = true;
	_toCheck = true;
}

ColliderBehaviour::~ColliderBehaviour()
{
	delete _baseCollider;
	delete _transformedCollider;
}

void ColliderBehaviour::SetCollider(Collisions::Collider *collider)
{
	if (!collider)
		return;
	if (_baseCollider)
		delete _baseCollider;
	if (_transformedCollider)
		delete _transformedCollider;

	_baseCollider = collider;
	switch (_baseCollider->colliderType)
	{
	case Collisions::RAY_COLLIDER:
		_transformedCollider = new Collisions::Ray(*static_cast<Collisions::Ray *>(collider));
		break;
	case Collisions::SPHERE_COLLIDER:
		_transformedCollider = new Collisions::Sphere(*static_cast<Collisions::Sphere *>(collider));
		break;
	case Collisions::CAPSULE_COLLIDER:
		_transformedCollider = new Collisions::Capsule(*static_cast<Collisions::Capsule *>(collider));
		break;
	case Collisions::AABB_COLLIDER:
		_transformedCollider = new Collisions::AABB(*static_cast<Collisions::AABB *>(collider));
		break;
	case Collisions::OBB_COLLIDER:
		_transformedCollider = new Collisions::OBB(*static_cast<Collisions::OBB *>(collider));
		break;
	case Collisions::TERRAIN_COLLIDER:
		_transformedCollider = new Collisions::Terrain(*static_cast<Collisions::Terrain *>(collider));
		break;
	default:
		break;
	}
	_isDirty = true;
	_toCheck = true;
}

const Collisions::Collider *ColliderBehaviour::GetCollider() const
{
    return _transformedCollider;
}

void ColliderBehaviour::AddOnIntersection(std::function<void(const Collisions::CollisionData &)> callback)
{
	if (_transformedCollider)
		_transformedCollider->AddIntersectionCallback(callback);
}

void ColliderBehaviour::AddOnCollisionEnter(std::function<void(const Collisions::CollisionData &)> callback)
{
	if (_transformedCollider)
		_transformedCollider->AddOnCollisionEnterCallback(callback);
}

void ColliderBehaviour::AddOnCollisionExit(std::function<void(const Collisions::CollisionData &)> callback)
{
	if (_transformedCollider)
		_transformedCollider->AddOnCollisionExitCallback(callback);
}

#ifdef DEBUG_BUILD
bool ColliderBehaviour::SetDebugCollider(Scene *scene, const Content *content)
{
	if (!_transformedCollider->SetDebugEnt(scene, content))
	{
		ErrMsg("Failed to create collider debug entity!");
		return false;
	}
	return true;
}
#endif

void ColliderBehaviour::SetIntersecting(bool value)
{
	_isIntersecting = value;
}

bool ColliderBehaviour::GetIntersecting()
{
	return _isIntersecting;
}

void ColliderBehaviour::CheckDone()
{
	_toCheck = false;
}

bool ColliderBehaviour::GetToCheck()
{
	return IsEnabled() && _toCheck && _transformedCollider->GetActive();
}

bool ColliderBehaviour::Serialize(std::string *code) const
{
	using namespace Collisions;
	using namespace std;

	// Standard code for Serialize
	*code += "ColliderBehaviour(";
		//+ std::to_string(_isOn) + " " + std::to_string(_battery) + " " + std::to_string(_isDead) + " " + std::to_string(_isCharging) +
		//" )";

	ColliderTypes type = _baseCollider->colliderType;
	if (type == ColliderTypes::RAY_COLLIDER)
	{
		Ray *base = dynamic_cast<Ray *>(_baseCollider);
		Ray *transformed = dynamic_cast<Ray *>(_transformedCollider);

		// [type] [tag] (base) [origon.x] [origon.y] [origon.z] [dir.x] [dir.y] [dir.z] [length]
		// (transformed) [origon.x] [origon.y] [origon.z] [dir.x] [dir.y] [dir.z] [length]
		code->append(std::format("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
			(int)type, base->GetTag(), base->origin.x, base->origin.y, base->origin.z, base->dir.x, base->dir.y, base->dir.y, base->length,
			transformed->origin.x, transformed->origin.y, transformed->origin.z, transformed->dir.x, transformed->dir.y, transformed->dir.y, base->length
		));
	}
	else if (type == ColliderTypes::SPHERE_COLLIDER)
	{
		Sphere *base = dynamic_cast<Sphere *>(_baseCollider);
		Sphere *transformed = dynamic_cast<Sphere *>(_transformedCollider);

		// [type] [tag] (base) [center.x] [center.y] [center.z] [radius] (transformed) [center.x] [center.y] [center.z] [radius]
		code->append(std::format("{} {} {} {} {} {} {} {} {} {}",
			(int)type, base->GetTag(), base->center.x, base->center.y, base->center.z, base->radius,
			transformed->center.x, transformed->center.y, transformed->center.z, transformed->radius
		));
	}
	else if (type == ColliderTypes::CAPSULE_COLLIDER)
	{
		Capsule *base = dynamic_cast<Capsule *>(_baseCollider);
		Capsule *transformed = dynamic_cast<Capsule *>(_transformedCollider);

		// [type] [tag] (base) [center.x] [center.y] [center.z] [upDir.x] [upDir.y] [upDir.z] [radius] [height]
		// (transformed) [center.x] [center.y] [center.z] [upDir.x] [upDir.y] [upDir.z] [radius] [height]
		code->append(std::format("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
			(int)type, base->GetTag(), base->center.x, base->center.y, base->center.z, base->upDir.x, base->upDir.y, base->upDir.z, base->radius, base->height,
			transformed->center.x, transformed->center.y, transformed->center.z,
			transformed->upDir.x, transformed->upDir.y, transformed->upDir.z, transformed->radius, transformed->height
		));
	}
	else if (type == ColliderTypes::AABB_COLLIDER)
	{
		AABB *base = dynamic_cast<AABB *>(_baseCollider);
		AABB *transformed = dynamic_cast<AABB *>(_transformedCollider);

		// [type] [tag] (base) [center.x] [center.y] [center.z] [halfLength.x] [halfLength.y] [halfLength.z]
		// (transformed) [center.x] [center.y] [center.z] [halfLength.x] [halfLength.y] [halfLength.z]
		code->append(std::format("{} {} {} {} {} {} {} {} {} {} {} {} {} {}",
			(int)type, base->GetTag(), base->center.x, base->center.y, base->center.z, base->halfLength.x, base->halfLength.y, base->halfLength.z,
			transformed->center.x, transformed->center.y, transformed->center.z, 
			transformed->halfLength.x, transformed->halfLength.y, transformed->halfLength.z
		));
	}
	else if (type == ColliderTypes::OBB_COLLIDER)
	{
		OBB *base = dynamic_cast<OBB *>(_baseCollider);
		OBB *transformed = dynamic_cast<OBB *>(_transformedCollider);

		// [type] [tag] (base) [center] [halfLength] [axes[0]] [axes[1]] [axes[2]]
		// (transformed) [center] [halfLength] [axes[0]] [axes[1]] [axes[2]]
		code->append(std::format("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
			(int)type, base->GetTag(),
			base->center.x, base->center.y, base->center.z, 
			base->halfLength.x, base->halfLength.y, base->halfLength.z,
			base->axes[0].x, base->axes[0].y, base->axes[0].z, 
			base->axes[1].x, base->axes[1].y, base->axes[1].z,
			base->axes[2].x, base->axes[2].y, base->axes[2].z,

			transformed->center.x, transformed->center.y, transformed->center.z, 
			transformed->halfLength.x, transformed->halfLength.y, transformed->halfLength.z,
			transformed->axes[0].x, transformed->axes[0].y, transformed->axes[0].z, 
			transformed->axes[1].x, transformed->axes[1].y, transformed->axes[1].z,
			transformed->axes[2].x, transformed->axes[2].y, transformed->axes[2].z
		));
	}
	else if (type == ColliderTypes::TERRAIN_COLLIDER)
	{
		Terrain *base = dynamic_cast<Terrain *>(_baseCollider);
		Terrain *transformed = dynamic_cast<Terrain *>(_transformedCollider);

		// [type] [tag] [center] [halfLength] [tileSize] [map] [_minIndex] [_maxIndex] [_heightScale]
		code->append(std::format("{} {} ", (int)type, base->GetTag()));

		base->Serialize(code);
		*code += " ";
		transformed->Serialize(code);
	}

	*code += " )";
	return true;
}

bool ColliderBehaviour::Deserialize(const std::string &code)
{
	using namespace Collisions;
	using namespace std;

	// Standard code for all behaviours deserialize
	std::vector<float> attributes;
	std::istringstream stream(code);

	std::string value;
	stream >> value;
	ColliderTypes type = (ColliderTypes)stoi(value);
	stream >> value;
	ColliderTags tag = (ColliderTags)stoi(value);

	while (stream >> value) // Automatically handles spaces correctly
	{
		if (type != ColliderTypes::TERRAIN_COLLIDER)
		{
			float attribute = std::stof(value);
			attributes.push_back(attribute);
		}
	}

	if (type == ColliderTypes::RAY_COLLIDER)
	{
		// [type] [tag] (base) [origon.x] [origon.y] [origon.z] [dir.x] [dir.y] [dir.z] [length]
		// (transformed) [origon.x] [origon.y] [origon.z] [dir.x] [dir.y] [dir.z] [length]
		DirectX::XMFLOAT3A origon = { attributes[0], attributes[1], attributes[2]};
		DirectX::XMFLOAT3A dir = { attributes[3], attributes[4], attributes[5] };
		_baseCollider = new Ray(origon, dir, attributes[6], tag);

		origon = { attributes[7], attributes[8], attributes[9] };
		dir = { attributes[10], attributes[11], attributes[12] };
		_transformedCollider = new Ray(origon, dir, attributes[13], tag);
	}
	else if (type == ColliderTypes::SPHERE_COLLIDER)
	{
		// [type] (base) [center.x] [center.y] [center.z] [radius] (transformed) [center.x] [center.y] [center.z] [radius]
		DirectX::XMFLOAT3A center = { attributes[0], attributes[1], attributes[2] };
		_baseCollider = new Sphere(center, attributes[3], tag);

		center = { attributes[4], attributes[5], attributes[6] };
		_transformedCollider = new Sphere(center, attributes[7], tag);
	}
	else if (type == ColliderTypes::CAPSULE_COLLIDER)
	{
		// [type] (base) [center.x] [center.y] [center.z] [upDir.x] [upDir.y] [upDir.z] [radius] [height]
		// (transformed) [center.x] [center.y] [center.z] [upDir.x] [upDir.y] [upDir.z] [radius] [height]
		DirectX::XMFLOAT3A center = { attributes[0], attributes[1], attributes[2] };
		DirectX::XMFLOAT3A upDir = { attributes[3], attributes[4], attributes[5] };
		float radius = attributes[6];
		float height = attributes[7];
		_baseCollider = new Capsule(center, upDir, radius, height, tag);

		center = { attributes[8], attributes[9], attributes[10] };
		upDir = { attributes[11], attributes[12], attributes[13] };
		radius = attributes[14];
		height = attributes[15];
		_transformedCollider = new Capsule(center, upDir, radius, height, tag);
	}
	else if (type == ColliderTypes::AABB_COLLIDER)
	{
		// [type] (base) [center.x] [center.y] [center.z] [halfLength.x] [halfLength.y] [halfLength.z]
		// (transformed) [center.x] [center.y] [center.z] [halfLength.x] [halfLength.y] [halfLength.z]
		DirectX::XMFLOAT3A center = { attributes[0], attributes[1], attributes[2] };
		DirectX::XMFLOAT3A halfLength = { attributes[3], attributes[4], attributes[5] };
		_baseCollider = new AABB(center, halfLength, tag);

		center = { attributes[6], attributes[7], attributes[8] };
		halfLength = { attributes[9], attributes[10], attributes[11] };
		_transformedCollider = new AABB(center, halfLength, tag);
	}
	else if (type == ColliderTypes::OBB_COLLIDER)
	{
		// [type] (base) [center] [halfLength] [axes[0]] [axes[1]] [axes[2]]
		// (transformed) [center] [halfLength] [axes[0]] [axes[1]] [axes[2]]
		DirectX::XMFLOAT3 center = { attributes[0], attributes[1], attributes[2] };
		DirectX::XMFLOAT3 halfLength = { attributes[3], attributes[4], attributes[5] };
		DirectX::XMFLOAT3 axes[3] = {
			{ attributes[6], attributes[7], attributes[8] }, 
			{ attributes[9], attributes[10], attributes[11] }, 
			{ attributes[12], attributes[13], attributes[14] } 
		};
		_baseCollider = new OBB(center, halfLength, axes, tag);

		center = { attributes[15], attributes[16], attributes[17] };
		halfLength = { attributes[18], attributes[19], attributes[20] };
		axes[0] = { attributes[21], attributes[22], attributes[23] };
		axes[1] = { attributes[24], attributes[25], attributes[26] };
		axes[2] = { attributes[27], attributes[28], attributes[29] };
		_transformedCollider = new OBB(center, halfLength, axes, tag);
	}
	else if (type == ColliderTypes::TERRAIN_COLLIDER)
	{
		// Standard code for all behaviours deserialize
		stream = std::istringstream(code);

		std::string value;
		stream >> value;
		stream >> value;
		// float3   float3       int2	   string uint		  uint		  float       
		// [center] [halfLength] [tileSize] [map] [_minIndex] [_maxIndex] [_heightScale]
		float floatAttr[6];
		for (int i = 0; i < 6; i++)
		{
			stream >> value;
			floatAttr[i] = std::stof(value);
		}
		DirectX::XMFLOAT3 center = { floatAttr[0], floatAttr[1], floatAttr[2] };
		DirectX::XMFLOAT3 halfLength = { floatAttr[3], floatAttr[4], floatAttr[5] };

		int intAttr[2];
		for (int i = 0; i < 2; i++)
		{
			stream >> value;
			intAttr[i] = std::stoi(value);
		}
		DirectX::XMINT2 tileSize = { intAttr[0], intAttr[1] };

		string map;
		stream >> map;

		HeightMap *heightMap = GetScene()->GetContent()->GetHeightMap(map);
		_baseCollider = new Terrain(center, halfLength, heightMap, tag);

		for (int i = 0; i < 6; i++)
		{
			stream >> value;
			floatAttr[i] = std::stof(value);
		}
		center = { floatAttr[0], floatAttr[1], floatAttr[2] };
		halfLength = { floatAttr[3], floatAttr[4], floatAttr[5] };

		for (int i = 0; i < 2; i++)
		{
			stream >> value;
			intAttr[i] = std::stoi(value);
		}
		tileSize = { intAttr[0], intAttr[1] };

		//stream >> map;

		heightMap = GetScene()->GetContent()->GetHeightMap(map);
		_transformedCollider = new Terrain(center, halfLength, heightMap, tag);
	}

	return true;
}