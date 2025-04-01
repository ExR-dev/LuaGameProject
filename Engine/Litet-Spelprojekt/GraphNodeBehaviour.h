#pragma once
#include <vector>

#include "Behaviour.h"

class GraphNodeBehaviour final : public Behaviour
{
private:
	std::vector<GraphNodeBehaviour*> _connections;
	std::vector<UINT> _deserializedConnections;
	float _cost = 0.0f;

protected:
	[[nodiscard]] bool Start() override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

public:
	GraphNodeBehaviour() = default;
	~GraphNodeBehaviour();

	[[nodiscard]] bool Serialize(std::string *code) const override;
	[[nodiscard]] bool Deserialize(const std::string &code) override;

	void CompleteDeserialization();

	void SetCost(float cost);
	[[nodiscard]] float GetCost() const;
	[[nodiscard]] const std::vector<GraphNodeBehaviour *> &GetConnections() const;
	void AddConnection(GraphNodeBehaviour *connection, bool secondIteration = false);
	void RemoveConnection(GraphNodeBehaviour *connection, bool secondIteration = false);

	[[nodiscard]] bool DrawConnections();

#ifdef DEBUG_BUILD
	void SetShowNode(bool show);
#endif
};