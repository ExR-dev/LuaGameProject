#include "stdafx.h"
#include "GraphNodeBehaviour.h"
#include "GraphManager.h"
#include "DebugDrawer.h"
#include "MeshBehaviour.h"
#include "Scene.h"
#include "DebugPlayerBehaviour.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

GraphNodeBehaviour::~GraphNodeBehaviour()
{
	int connectionSize = static_cast<int>(_connections.size());
	for (int i = 0; i < connectionSize; i++)
	{
		auto &connection = _connections.at(i);

		if (connection == nullptr)
			continue;

		RemoveConnection(connection);
		connectionSize--;
		i--;
	}

	GetScene()->GetGraphManager()->RemoveNode(this);
}

bool GraphNodeBehaviour::Start()
{
	if (_name == "")
		_name = "GraphNodeBehaviour"; // For categorization in ImGui.

	GetScene()->GetGraphManager()->AddNode(this);

#ifdef DEBUG_BUILD
	Content *content = GetScene()->GetContent();

	BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
	UINT meshID = content->GetMeshID("Mesh_Sphere");
	Material mat;
	mat.textureID = mat.ambientID = content->GetTextureID("Tex_Red");
	mat.textureID = mat.ambientID = (_cost < 0.1f) ? content->GetTextureID("Tex_Blue") : content->GetTextureID("Tex_Red");

	MeshBehaviour *mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);
	if (!mesh->Initialize(GetEntity()))
	{
		ErrMsg("Failed to initialize MeshBehaviour for GraphNodeBehaviour.");
		return false;
	}
	mesh->SetSerialization(false);
	mesh->SetEnabled(false);
#endif
	return true;
}

bool GraphNodeBehaviour::Serialize(std::string *code) const
{
	*code += _name + "(";
	if (_connections.size() <= 0)
	{
		*code += " )";
		return true;
	}

	*code += "C" +std::to_string(_cost) + " ";

	for (auto &connection : _connections)
	{
		if (connection == nullptr)
			continue;

		*code += std::to_string(connection->GetEntity()->GetID()) + " ";
	}

	*code += ")";
	return true;
}
bool GraphNodeBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<UINT> attributes;
	std::istringstream stream(code);

	bool first = true;

	std::string value;
	while (stream >> value) // Automatically handles spaces correctly
	{
		if (first)
		{
			first = false;

			if (value.find("C") != std::string::npos)
			{
				_cost = std::stof(value.substr(1));
				continue;
			}
		}

		UINT attribute = std::stoul(value);
		attributes.push_back(attribute);
	}

	_deserializedConnections.clear();
	_deserializedConnections.reserve(attributes.size());
	for (auto &attribute : attributes)
	{
		_deserializedConnections.push_back(attribute);
	}

	return true;
}

void GraphNodeBehaviour::CompleteDeserialization()
{
	_connections.reserve(_deserializedConnections.size());
	for (auto &connection : _deserializedConnections)
	{
		Entity *ent = GetScene()->GetSceneHolder()->GetEntityByDeserializedID(connection);
		if (ent == nullptr)
			continue;

		GraphNodeBehaviour *node;
		ent->GetBehaviourByType<GraphNodeBehaviour>(node);
		if (node == nullptr)
			continue;

		AddConnection(node);
	}
	_deserializedConnections.clear();
}

#ifdef USE_IMGUI
bool GraphNodeBehaviour::RenderUI()
{
	UINT thisID = GetEntity()->GetID();

	float newCost = _cost;
	ImGui::Text("Cost:");
	ImGui::SameLine();
	if (ImGui::InputFloat("##NodeCost", &newCost))
		SetCost(newCost);

	// Connection Drop field
	{
		ImGui::PushID(("Connection Drop " + std::to_string(thisID)).c_str());

		if (ImGui::Button("Drop Node Here To Connect", { 100, 35 }))
		{

		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
			{
				IM_ASSERT(payload->DataSize == sizeof(UINT));
				UINT payloadID = *(const UINT *)payload->Data;

				if (payloadID != thisID)
				{
					Entity *payloadEnt = GetScene()->GetSceneHolder()->GetEntityByID(payloadID);

					if (payloadEnt)
					{
						GraphNodeBehaviour *payloadNode;
						payloadEnt->GetBehaviourByType<GraphNodeBehaviour>(payloadNode);

						if (payloadNode)
							AddConnection(payloadNode);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();
	}

	ImGui::Dummy({ 0, 15 });
	ImGui::Text("Connections:");
	UINT connectionSize = static_cast<UINT>(_connections.size());
	for (UINT i = 0; i < connectionSize; i++)
	{
		ImGui::PushID(std::format("Node {} Connection {}", thisID, i).c_str());

		auto &connection = _connections.at(i);
		if (connection == nullptr)
			continue;

		if (ImGui::SmallButton(std::format("[{}] {}", i, connection->GetEntity()->GetName()).c_str()))
			GetScene()->GetDebugPlayer()->SetSelection(GetScene()->GetSceneHolder()->GetEntityIndex(connection->GetEntity()));

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.5f, 0.55f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.5f, 0.65f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.5f, 0.75f, 0.8f));
		ImGui::SmallButton("?");
		if (ImGui::IsItemActive())
		{
			DebugDrawer *drawer = DebugDrawer::GetInstance();

			drawer->DrawLine(
				GetTransform()->GetPosition(World),
				connection->GetTransform()->GetPosition(World),
				1.0f,
				DirectX::XMFLOAT4(0, 1, 1, 1),
				false
			);
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.55f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.65f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.75f, 0.8f));
		if (ImGui::SmallButton("X"))
		{
			RemoveConnection(connection);
			connectionSize--;
			i--;
		}
		ImGui::PopStyleColor(3);

		ImGui::PopID();
	}

	return true;
}
#endif

void GraphNodeBehaviour::SetCost(float cost)
{
	_cost = cost;
	GraphManager *manager = GetScene()->GetGraphManager();
	if (manager)
		manager->UpdateNode(this);

#ifdef DEBUG_BUILD
	Content *content = GetScene()->GetContent();

	MeshBehaviour *mesh;
	if (GetEntity()->GetBehaviourByType<MeshBehaviour>(mesh))
	{
		Material mat = *mesh->GetMaterial();
		mat.textureID = mat.ambientID = cost < 0.1f ? content->GetTextureID("Tex_Blue") : content->GetTextureID("Tex_Red");
		mesh->SetMaterial(&mat);
	}
#endif
}
float GraphNodeBehaviour::GetCost() const
{
	return _cost;
}

const std::vector<GraphNodeBehaviour*> &GraphNodeBehaviour::GetConnections() const
{
	return _connections;
}

void GraphNodeBehaviour::AddConnection(GraphNodeBehaviour *connection, bool secondIteration)
{
	if (connection == nullptr)
		return;

	if (std::find(_connections.begin(), _connections.end(), connection) != _connections.end())
		return;

	_connections.push_back(connection);

	if (!secondIteration)
		connection->AddConnection(this, true);
}
void GraphNodeBehaviour::RemoveConnection(GraphNodeBehaviour *connection, bool secondIteration)
{
	if (connection == nullptr)
		return;

	auto it = std::find(_connections.begin(), _connections.end(), connection);
	if (it == _connections.end())
		return;

	_connections.erase(it);

	if (!secondIteration)
		connection->RemoveConnection(this, true);
}

bool GraphNodeBehaviour::DrawConnections()
{
	DebugDrawer *drawer = DebugDrawer::GetInstance();

	for (auto &connection : _connections)
	{
		if (connection == nullptr)
			continue;

		drawer->DrawLine(
			GetTransform()->GetPosition(World), 
			connection->GetTransform()->GetPosition(World),
			0.33f,
			DirectX::XMFLOAT4(0, 1, 0, 0.1f), 
			false
		);
	}

	return true;
}

#ifdef DEBUG_BUILD
void GraphNodeBehaviour::SetShowNode(bool show)
{
	MeshBehaviour *mesh;
	if (GetEntity()->GetBehaviourByType<MeshBehaviour>(mesh))
		mesh->SetEnabled(show);
}
#endif