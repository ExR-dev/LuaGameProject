#include "stdafx.h"
#include "SolidObjectBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "ErrMsg.h"
#include "ImGui/imgui.h"
#include "GameMath.h"
#include "ColliderBehaviour.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace Collisions;

bool SolidObjectBehaviour::Start()
{
	if (_name == "")
		_name = "SolidObjectBehaviour"; // For categorization in ImGui.

	Entity *ent = GetEntity();

	ColliderBehaviour *colB;
	ent->GetBehaviourByType<ColliderBehaviour>(colB);

	if (colB)
		colB->AddOnIntersection([this](const Collisions::CollisionData &data) { AdjustForCollision(data); });

	return true;
}

void SolidObjectBehaviour::OnEnable()
{

}

void SolidObjectBehaviour::OnDisable()
{

}

void SolidObjectBehaviour::OnDirty()
{

}

bool SolidObjectBehaviour::Update(Time &time, const Input &input)
{
	return true;
}

bool SolidObjectBehaviour::LateUpdate(Time &time, const Input &input)
{
	return true;
}

bool SolidObjectBehaviour::FixedUpdate(const float &deltaTime, const Input &input)
{
	return true;
}

bool SolidObjectBehaviour::Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo)
{
	return true;
}

#ifdef USE_IMGUI
bool SolidObjectBehaviour::RenderUI()
{
	return true;
}
#endif

bool SolidObjectBehaviour::BindBuffers()
{
	return true;
}

bool SolidObjectBehaviour::Serialize(std::string *code) const
{
	*code += _name + "( )";
	//+ std::to_string(valfri variabel) +
	//" )";
	return true;
}

bool SolidObjectBehaviour::Deserialize(const std::string &code)
{
	return true;
}

bool SolidObjectBehaviour::OnSelect()
{
	return true;
}

bool SolidObjectBehaviour::OnHover()
{
	return true;
}

void SolidObjectBehaviour::AdjustForCollision(const Collisions::CollisionData &data)
{
	using namespace DirectX;

	XMFLOAT3A move;
	Store(move, XMVectorScale(XMVector3Normalize(XMLoadFloat3(&data.normal)), data.depth/2));

	Transform *t = GetEntity()->GetTransform();
	t->Move(move, World);
}

