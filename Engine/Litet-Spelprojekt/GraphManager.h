#pragma once
#include <vector>
#include <DirectXMath.h>
#include <DirectXCollision.h>

class GraphNodeBehaviour;

namespace Pathfinding
{
	struct PointRelativeGraph
	{
		DirectX::XMFLOAT3 point, projectedPoint;
		GraphNodeBehaviour *connectedNodeOne, *connectedNodeTwo;
	};

	struct GraphNode
	{
		DirectX::XMFLOAT4 point;
		std::vector<int> connections;
	};
}

class GraphManager
{
private:

	std::vector<GraphNodeBehaviour *> _nodes = {};
	std::vector<GraphNodeBehaviour *> _mineNodes = {};

public:
	GraphManager() = default;
	~GraphManager() = default;

	void AStar(Pathfinding::PointRelativeGraph start, Pathfinding::PointRelativeGraph end, std::vector<DirectX::XMFLOAT3> *points) const;

	int GetNodeCount() const;
	int GetMineNodeCount() const;
	void GetNodes(std::vector<GraphNodeBehaviour *> &nodes) const;
	void GetMineNodes(std::vector<GraphNodeBehaviour *> &nodes) const;

	void AddNode(GraphNodeBehaviour *node);
	void RemoveNode(GraphNodeBehaviour *node);
	void UpdateNode(GraphNodeBehaviour *node);

	void GetClosestPoint(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 &closestPoint, bool onlyMines = false);
	[[nodiscard]] bool isMinePoint(DirectX::XMFLOAT3 nodePos);
	[[nodiscard]] bool isPoint(DirectX::XMFLOAT3 nodePos);

	void GetPath(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 dest, std::vector<DirectX::XMFLOAT3> *points) const;

	[[nodiscard]] bool RenderUI(DirectX::XMFLOAT3 posA, DirectX::XMFLOAT3 posB);

	void CompleteDeserialization();
};