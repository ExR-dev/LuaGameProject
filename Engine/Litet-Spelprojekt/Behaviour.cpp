#include "stdafx.h"
#include "Behaviour.h"
#include "CameraBehaviour.h"
#include "RenderQueuer.h"
#include "Scene.h"
#include "Entity.h"
#include "ErrMsg.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool Behaviour::Initialize(Entity *entity, const std::string &behaviourName)
{
	if (_isInitialized)
	{
		ErrMsg("Behaviour is already initialized!");
		return false;
	}

	if (!entity)
	{
		ErrMsg("Entity is null!");
		return false;
	}

	_entity = entity;
	entity->AddBehaviour(this);

	if (behaviourName != "")
	{
		_name = behaviourName;
	}

	if (!Start())
	{
		ErrMsg("Failed to initialize behaviour!");
		return false;
	}

#ifdef DEBUG_BUILD
	if (_name == "")
	{
		ErrMsg("Behaviour name is empty! Did you forget to assign a name in Start?");
		return false;
	}
#endif

	_isInitialized = true;
	return true;
}
bool Behaviour::IsInitialized() const
{
	return _isInitialized;
}

void Behaviour::SetSerialization(bool state)
{
	_doSerialize = state;
}
bool Behaviour::IsSerializable() const
{
	return _doSerialize;
}

Entity *Behaviour::GetEntity() const
{
	return _entity;
}
Transform *Behaviour::GetTransform() const
{
	if (!_entity)
		return nullptr;

	return _entity->GetTransform();
}
Scene *Behaviour::GetScene() const
{
	if (!_entity)
		return nullptr;

	return _entity->GetScene();
}

void Behaviour::SetName(const std::string &name)
{
	_name = name;
}
const std::string &Behaviour::GetName() const
{
	return _name;
}

bool Behaviour::IsEnabled() const
{
	return _entity->IsEnabled() && _isEnabledSelf;
}
bool Behaviour::IsEnabledSelf() const
{
	return _isEnabledSelf;
}
void Behaviour::InheritEnabled(bool state)
{
	if (!_isEnabledSelf)
		return;

	if (state)
		OnEnable();
	else
		OnDisable();
}
void Behaviour::SetEnabled(bool state)
{
	if (_isEnabledSelf == state)
		return;

	_isEnabledSelf = state;

	if (_entity)
	{
		if (!_entity->IsEnabled())
			return;
	}

	if (state)
		OnEnable();
	else
		OnDisable();
}

void Behaviour::SetDirty()
{
	OnDirty();
}

bool Behaviour::InitialUpdate(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(861732418, std::format("Behaviour Update '{}'", GetName()).c_str());
#endif

	if (!IsEnabled())
		return true;

	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}

	return Update(time, input);
}
bool Behaviour::InitialParallelUpdate(const Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(751975251, std::format("Behaviour Parallel Update '{}'", GetName()).c_str());
#endif

	if (!IsEnabled())
		return true;

	if (!_isInitialized)
	{
#pragma omp critical
		{
			ErrMsg("Behaviour is not initialized!");
		}
		return false;
	}

	return ParallelUpdate(time, input);
}
bool Behaviour::InitialLateUpdate(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(35152246, std::format("Behaviour Late Update '{}'", GetName()).c_str());
#endif

	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return LateUpdate(time, input);
}
bool Behaviour::InitialFixedUpdate(const float &deltaTime, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(3888888, std::format("Behaviour Fixed Update '{}'", GetName()).c_str());
#endif

	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return FixedUpdate(deltaTime, input);
}
bool Behaviour::InitialBeforeRender()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(6186685, std::format("Behaviour Before Render '{}'", GetName()).c_str());
#endif

	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return BeforeRender();
}
bool Behaviour::InitialRender(const RenderQueuer &queuer, const RendererInfo &rendererInfo)
{
#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	if (!GetEntity())
		return true;

	if (GetEntity()->IsRemoved())
		return true;

	if (!IsEnabled())
		return true;

	return Render(queuer, rendererInfo);
}
#ifdef USE_IMGUI
bool Behaviour::InitialRenderUI()
{
	ImGui::PushID("enabled");
	bool behEnabled = IsEnabledSelf();
	if (ImGui::Checkbox("Active", &behEnabled))
	{
		if (behEnabled)
			SetEnabled(true);
		else
			SetEnabled(false);
	}
	ImGui::PopID();

	ImGui::PushID("behSerialize");
	ImGui::Checkbox("Serialize", &_doSerialize);
	ImGui::PopID();

	ImGui::Dummy({0, 6});
	return RenderUI();
}
#endif
bool Behaviour::InitialBindBuffers()
{
	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return BindBuffers();
}

bool Behaviour::InitialOnSelect()
{
	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return OnSelect();
}
bool Behaviour::InitialOnHover()
{
	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return OnHover();
}

bool Behaviour::InitialOffHover()
{
	if (!IsEnabled())
		return true;

#ifdef DEBUG_BUILD
	if (!_isInitialized)
	{
		ErrMsg("Behaviour is not initialized!");
		return false;
	}
#endif

	return OffHover();
}

bool Behaviour::InitialSerialize(std::string *code) const
{
	if (!_doSerialize)
		return true;

	if (!Serialize(code))
	{
		ErrMsg("Failed to serialize behaviour!");
		return false;
	}

	return true;
}
bool Behaviour::InitialDeserialize(const std::string &code)
{
	if (!Deserialize(code))
	{
		ErrMsg("Failed to deserialize behaviour!");
		return false;
	}

	return true;
}

bool Behaviour::Start() { return true; }
bool Behaviour::Update(Time &time, const Input &input) { return true; }
bool Behaviour::ParallelUpdate(const Time &time, const Input &input) { return true; }
bool Behaviour::LateUpdate(Time &time, const Input &input) { return true; }
bool Behaviour::FixedUpdate(const float &deltaTime, const Input &input) { return true; }
bool Behaviour::BeforeRender()
{
	return true;
}
bool Behaviour::Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo) { return true; }
#ifdef USE_IMGUI
bool Behaviour::RenderUI() { return true; }
#endif
bool Behaviour::BindBuffers() { return true; }

void Behaviour::OnEnable() { }
void Behaviour::OnDisable() { }
void Behaviour::OnDirty() { }
bool Behaviour::OnSelect() { return true; }
bool Behaviour::OnHover() { return true; }
bool Behaviour::OffHover() { return true; }

bool Behaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	//*code += _name + "(" 
	//	+ std::to_string(valfri variabel) +
	//	" )";

	return true;
}
bool Behaviour::Deserialize(const std::string &code) { return true; }

