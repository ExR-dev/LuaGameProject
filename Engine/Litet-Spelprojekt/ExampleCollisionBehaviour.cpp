#include "stdafx.h"
#include "ExampleCollisionBehaviour.h"

#include "ColliderBehaviour.h"
#include "Scene.h"
#include "Intersections.h"

#include <random>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace Collisions;
using namespace DirectX;

bool ExampleCollisionBehaviour::Start()
{
	if (_name == "")
		_name = "ExampleColliderBehaviour"; // For categorization in ImGui.

	// Get Collider Behaviour
	Entity *ent = GetEntity();
	ColliderBehaviour *colB;
	ent->GetBehaviourByType<ColliderBehaviour>(colB);

	// Add Collision Functions
	colB->AddOnIntersection([this, ent](const CollisionData &data) {
		DebugDrawer *debugDraw = GetScene()->GetDebugDrawer();
		ColliderBehaviour *cb;
		if (GetEntity()->GetBehaviourByType<ColliderBehaviour>(cb))
		{
			const Collider* collider = cb->GetCollider();
			if (collider)
			{
				XMFLOAT3 center{};
				switch (collider->colliderType)
				{
				case RAY_COLLIDER:
					center = static_cast<const Ray *>(collider)->origin;
					break;
				case SPHERE_COLLIDER:
					center = static_cast<const Sphere *>(collider)->center;
					break;
				case AABB_COLLIDER:
					center = static_cast<const AABB *>(collider)->center;
					break;
				case OBB_COLLIDER:
					center = static_cast<const OBB *>(collider)->center;
					break;
				case CAPSULE_COLLIDER:
					center = static_cast<const Capsule *>(collider)->center;
					break;
				case TERRAIN_COLLIDER:
					center = static_cast<const Terrain *>(collider)->center;
					break;
				default:
					break;
				}

				float d = data.depth;
				XMFLOAT3 max = XMFLOAT3(center.x + data.normal.x * d, center.y + data.normal.y * d, center.z + data.normal.z * d);
				
				// Draw normal
				debugDraw->DrawLine(center, max, 0.01f, { 1, 1, 0.25f, 1 }, false);

			}
		}

		// Resolve collision
		/*const static float uniformSkin = 0.001f;
		XMVECTOR newPos = XMLoadFloat3A(&ent->GetTransform()->GetPosition());

		newPos = XMVectorAdd(newPos, XMVectorScale(XMLoadFloat3(&data.normal), data.depth/2 + uniformSkin));

		XMFLOAT3A n;
		XMStoreFloat3A(&n, newPos);
		ent->GetTransform()->SetPosition(n);

		if (!GetScene()->GetSceneHolder()->UpdateEntityPosition(ent))
		{
			ErrMsg("Failed to update entity transform after collision!");
			return false;

		}

		if (!ent->GetTransform()->UpdateConstantBuffer(GetScene()->GetContext()))
		{
			ErrMsg("Failed to set world matrix buffer after collision!");
			return false;
		}*/
	});

	colB->AddOnCollisionEnter([ent](const CollisionData &data) {
		MeshBehaviour *mb;
		ent->GetBehaviourByType<MeshBehaviour>(mb);
		if (mb)
		{
			Material mat = *mb->GetMaterial();
			//mat.textureID = rand() % ent->GetScene()->GetContent()->GetTextureCount();
			mat.textureID = ent->GetScene()->GetContent()->GetTextureID("Tex_Red");

			if (!mb->SetMaterial(&mat))
				ErrMsg("Failed setting material on collision!");
		}
	});

	colB->AddOnCollisionExit([ent](const CollisionData &data) {
		MeshBehaviour *mb;
		ent->GetBehaviourByType<MeshBehaviour>(mb);
		if (mb)
		{
			Material mat = *mb->GetMaterial();
			mat.textureID = ent->GetScene()->GetContent()->GetTextureID("Tex_Maxwell");
			if (!mb->SetMaterial(&mat))
				ErrMsg("Failed setting material on collision!");
		}
	});

    return true;
}

bool ExampleCollisionBehaviour::Update(Time &time, const Input &input)
{
	ColliderBehaviour *cb;
	if (GetEntity()->GetBehaviourByType<ColliderBehaviour>(cb))
	{
		const Collider* collider = cb->GetCollider();
		if (cb->GetCollider()->colliderType == RAY_COLLIDER)
		{
			CollisionData data;
			if (GetScene()->GetCollisionHandler()->CheckCollision(collider, GetScene(), data))
			{
				Entity *ent = GetEntity();
				MeshBehaviour *mb;
				ent->GetBehaviourByType<MeshBehaviour>(mb);
				if (mb)
				{
					Material mat = *mb->GetMaterial();
					mat.textureID = ent->GetScene()->GetContent()->GetTextureID("Tex_White");
					if (!mb->SetMaterial(&mat))
						ErrMsg("Failed setting material on collision!");
				}
			}
		}
	}

	return true;
}

bool ExampleCollisionBehaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	*code += "ExampleCollisionBehaviour( )";

	return true;
}

bool ExampleCollisionBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<UINT> attributes;
	std::istringstream stream(code);

	std::string value;
	while (stream >> value) // Automatically handles spaces correctly
	{
		UINT attribute = static_cast<UINT>(std::stoi(value));
		attributes.push_back(attribute);
	}

	return true;
}

