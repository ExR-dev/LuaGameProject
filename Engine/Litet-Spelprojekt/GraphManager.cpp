#include "stdafx.h"
#include "GraphManager.h"
#include "GraphNodeBehaviour.h"
#include "Intersections.h"
#include "DebugDrawer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

inline static void ReconstructPath(int start, int goal, std::unordered_map<int, int> cameFrom, std::vector<int> &path)
{
	int current = goal;
	if (cameFrom.find(goal) == cameFrom.end())
		return; // No path found

	while (current != start) 
	{
		path.push_back(current);
		current = cameFrom[current];
	}

	std::reverse(path.begin(), path.end());
}

inline static float CalculateCost(XMFLOAT3 a, XMFLOAT3 b)
{
	return XMVectorGetX(XMVector3Length(XMVectorSubtract(Load(b), Load(a))));
}

inline static void AStarStep(
	std::vector<Pathfinding::GraphNode> &graph,
	int start, int goal,
	std::unordered_map<int, int> &cameFrom,
	std::unordered_map<int, float> &cost)
{
	std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, std::greater<std::pair<int, float>>> frontier;
	frontier.emplace(start, 0);

	cameFrom[start] = start;
	cost[start] = 0;

	while (!frontier.empty()) 
	{
		int current = frontier.top().first;
		frontier.pop();

		if (current == goal)
			return;

		for (int next : graph.at(current).connections) 
		{
			float avgCost = 0.5f * (graph.at(current).point.w + graph.at(next).point.w);

			float newCost = avgCost + cost[current] + CalculateCost(
				To3(graph.at(current).point), 
				To3(graph.at(next).point)
			);

			if (cost.find(next) == cost.end() || newCost < cost[next])
			{
				cost[next] = newCost;
				float priority = newCost + CalculateCost(
					To3(graph.at(next).point), 
					To3(graph.at(goal).point)
				);

				frontier.emplace(next, priority);
				cameFrom[next] = current;
			}
		}
	}
}


void GraphManager::AStar(Pathfinding::PointRelativeGraph start, Pathfinding::PointRelativeGraph end, std::vector<XMFLOAT3> *points) const
{
	// Convert entity graph to indexed format
	std::vector<Pathfinding::GraphNode> graph = {};
	graph.reserve(_nodes.size() + 2);

	for (int i = 0; i < _nodes.size(); i++)
	{
		auto node = _nodes.at(i);
		if (node == nullptr)
			continue;

		std::vector<int> connections = {};
		connections.reserve(node->GetConnections().size());

		for (int j = 0; j < node->GetConnections().size(); j++)
		{
			auto connection = node->GetConnections().at(j);
			if (connection == nullptr)
				continue;

			int connectedIndex = -1;
			auto it = std::find(_nodes.begin(), _nodes.end(), connection);
			if (it != _nodes.end())
				connectedIndex = static_cast<int>(std::distance(_nodes.begin(), it));

			connections.push_back(connectedIndex);
		}

		XMFLOAT4 point = To4(node->GetTransform()->GetPosition(World));
		point.w = node->GetCost();

		graph.push_back({ 
			point,
			connections
		});
	}
	
	// Add start node
	{
		int nodeOneIndex = -1;
		int nodeTwoIndex = -1;

		std::vector<int> connections = {};
		connections.reserve(2);

		auto it = std::find(_nodes.begin(), _nodes.end(), start.connectedNodeOne);
		if (it != _nodes.end())
		{
			nodeOneIndex = static_cast<int>(std::distance(_nodes.begin(), it));
			connections.push_back(nodeOneIndex);
			graph[nodeOneIndex].connections.push_back(static_cast<int>(graph.size()));
		}

		it = std::find(_nodes.begin(), _nodes.end(), start.connectedNodeTwo);
		if (it != _nodes.end())
		{
			nodeTwoIndex = static_cast<int>(std::distance(_nodes.begin(), it));
			connections.push_back(nodeTwoIndex);
			graph[nodeTwoIndex].connections.push_back(static_cast<int>(graph.size()));
		}

		XMFLOAT4 point = To4(start.point);
		point.w = 0.0f;

		graph.push_back({
			point,
			connections
		});

		// Remove all pre-existing paths connecting the two nodes of the start line
		/*auto &nodeOneConnections = graph[nodeOneIndex].connections;
		nodeOneConnections.erase(
			std::remove(nodeOneConnections.begin(), nodeOneConnections.end(), nodeTwoIndex),
			nodeOneConnections.end()
		);

		auto &nodeTwoConnections = graph[nodeTwoIndex].connections;
		nodeTwoConnections.erase(
			std::remove(nodeTwoConnections.begin(), nodeTwoConnections.end(), nodeOneIndex),
			nodeTwoConnections.end()
		);*/
	}

	// Add end node
	{
		int nodeOneIndex = -1;
		int nodeTwoIndex = -1;

		std::vector<int> connections = {};
		connections.reserve(2);

		auto it = std::find(_nodes.begin(), _nodes.end(), end.connectedNodeOne);
		if (it != _nodes.end())
		{
			nodeOneIndex = static_cast<int>(std::distance(_nodes.begin(), it));
			connections.push_back(nodeOneIndex);
			graph[nodeOneIndex].connections.push_back(static_cast<int>(graph.size()));
		}

		it = std::find(_nodes.begin(), _nodes.end(), end.connectedNodeTwo);
		if (it != _nodes.end())
		{
			nodeTwoIndex = static_cast<int>(std::distance(_nodes.begin(), it));
			connections.push_back(nodeTwoIndex);
			graph[nodeTwoIndex].connections.push_back(static_cast<int>(graph.size()));
		}

		XMFLOAT4 point = To4(end.point);
		point.w = 0.0f;

		graph.push_back({
			point,
			connections
		});

		// Remove all pre-existing paths connecting the two nodes of the end line
		/*auto &nodeOneConnections = graph[nodeOneIndex].connections;
		nodeOneConnections.erase(
			std::remove(nodeOneConnections.begin(), nodeOneConnections.end(), nodeTwoIndex),
			nodeOneConnections.end()
		);

		auto &nodeTwoConnections = graph[nodeTwoIndex].connections;
		nodeTwoConnections.erase(
			std::remove(nodeTwoConnections.begin(), nodeTwoConnections.end(), nodeOneIndex),
			nodeTwoConnections.end()
		);*/
	}

	// Find shortest path between the two points using A*
	int startNode = static_cast<int>(graph.size() - 2);
	int goalNode = static_cast<int>(graph.size() - 1);

	std::unordered_map<int, int> cameFrom = {};
	std::unordered_map<int, float> cost = {};
	AStarStep(graph, startNode, goalNode, cameFrom, cost);

	// Reconstruct path
	std::vector<int> path = {};
	ReconstructPath(startNode, goalNode, cameFrom, path);

	points->push_back(start.point);

	for (int i = 0; i < path.size(); i++)
	{
		Pathfinding::GraphNode &node = graph.at(path.at(i));

		XMFLOAT3 point = { node.point.x, node.point.y, node.point.z };
		points->push_back(point);
	}
}

int GraphManager::GetNodeCount() const
{
	return static_cast<int>(_nodes.size());
}
int GraphManager::GetMineNodeCount() const
{
	return static_cast<int>(_mineNodes.size());
}

void GraphManager::GetNodes(std::vector<GraphNodeBehaviour *> &nodes) const
{
	nodes = _nodes;
}
void GraphManager::GetMineNodes(std::vector<GraphNodeBehaviour *> &nodes) const
{
	nodes = _mineNodes;
}

void GraphManager::AddNode(GraphNodeBehaviour *node)
{
	if (node == nullptr)
		return;

	if (std::find(_nodes.begin(), _nodes.end(), node) != _nodes.end())
		return;

	_nodes.push_back(node);

	if (node->GetCost() < 0.1f)
		_mineNodes.push_back(node);
}
void GraphManager::RemoveNode(GraphNodeBehaviour *node)
{
	if (node == nullptr)
		return;

	auto it = std::find(_nodes.begin(), _nodes.end(), node);
	if (it == _nodes.end())
		return;
	_nodes.erase(it);

	it = std::find(_mineNodes.begin(), _mineNodes.end(), node);
	if (it == _mineNodes.end())
		return;
	_mineNodes.erase(it);
}
void GraphManager::UpdateNode(GraphNodeBehaviour *node)
{
	if (node == nullptr)
		return;

	auto it = std::find(_mineNodes.begin(), _mineNodes.end(), node);
	if (it == _mineNodes.end())
	{
		if (node->GetCost() < 0.1f)
			_mineNodes.push_back(node);
	}
	else
	{
		if (node->GetCost() >= 0.1f)
			_mineNodes.erase(it);
	}
}

void GraphManager::GetClosestPoint(XMFLOAT3 pos, XMFLOAT3 &closestPoint, bool onlyMines)
{
	float closestStartDist = FLT_MAX;
	auto &nodes = onlyMines ? _mineNodes : _nodes;

	XMVECTOR startVec = Load(pos);

	// TODO: Skip duplicates
	for (auto &node : nodes)
	{
		if (node == nullptr)
			continue;

		XMFLOAT3 connectionStart = node->GetTransform()->GetPosition(World);

		for (auto &connection : node->GetConnections())
		{
			if (connection == nullptr)
				continue;

			XMFLOAT3 connectionEnd = connection->GetTransform()->GetPosition(World);

			// Find closest point on line to pos & dest
			XMFLOAT3 closestToStart = Collisions::ClosestPoint({ connectionStart, connectionEnd }, pos);

			XMVECTOR closestToStartVec = Load(closestToStart);

			float lengthToStart = XMVectorGetX(XMVector3Length(XMVectorSubtract(startVec, closestToStartVec)));

			if (lengthToStart < closestStartDist)
			{
				closestStartDist = lengthToStart;
				closestPoint = closestToStart;
			}
		}
	}
}

bool GraphManager::isMinePoint(DirectX::XMFLOAT3 nodePos)
{
	for (int i = 0; i < _mineNodes.size(); i++)
	{
		XMFLOAT3A pos = _mineNodes.at(i)->GetTransform()->GetPosition();
		float dist;
		XMStoreFloat(&dist, XMVector3Length(Load(nodePos) - Load(pos)));
		if (dist <= 0.1f)
			return true;
	}
	return false;
}

bool GraphManager::isPoint(DirectX::XMFLOAT3 nodePos)
{
	for (int i = 0; i < _nodes.size(); i++)
	{
		XMFLOAT3A pos = _nodes.at(i)->GetTransform()->GetPosition();
		float dist;
		XMStoreFloat(&dist, XMVector3Length(Load(nodePos) - Load(pos)));
		if (dist <= 0.1f)
			return true;
	}
	return false;
}

void GraphManager::GetPath(XMFLOAT3 pos, XMFLOAT3 dest, std::vector<XMFLOAT3> *points) const
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(68174864, "Generate A* Path");
#endif

	// Find closest point on line to pos
	GraphNodeBehaviour *startNodeOne = nullptr;
	GraphNodeBehaviour *startNodeTwo = nullptr;
	GraphNodeBehaviour *destNodeOne = nullptr;
	GraphNodeBehaviour *destNodeTwo = nullptr;

	float closestStartDist = FLT_MAX;
	XMFLOAT3 closestStartPoint = { FLT_MAX, FLT_MAX, FLT_MAX };

	float closestDestDist = FLT_MAX;
	XMFLOAT3 closestDestPoint = { FLT_MAX, FLT_MAX, FLT_MAX };

	XMVECTOR startVec = Load(pos);
	XMVECTOR destVec = Load(dest);

	// TODO: Skip duplicates
	for (auto &node : _nodes)
	{
		if (node == nullptr)
			continue;

		XMFLOAT3 connectionStart = node->GetTransform()->GetPosition(World);

		for (auto &connection : node->GetConnections())
		{
			if (connection == nullptr)
				continue;

			XMFLOAT3 connectionEnd = connection->GetTransform()->GetPosition(World);

			// Find closest point on line to pos & dest
			XMFLOAT3 closestToStart = Collisions::ClosestPoint({ connectionStart, connectionEnd }, pos);
			XMFLOAT3 closestToDest = Collisions::ClosestPoint({ connectionStart, connectionEnd }, dest);

			XMVECTOR closestToStartVec = Load(closestToStart);
			XMVECTOR closestToDestVec = Load(closestToDest);

			float lengthToStart = XMVectorGetX(XMVector3Length(XMVectorSubtract(startVec, closestToStartVec)));
			float lengthToDest = XMVectorGetX(XMVector3Length(XMVectorSubtract(destVec, closestToDestVec)));

			if (lengthToStart < closestStartDist)
			{
				closestStartDist = lengthToStart;
				closestStartPoint = closestToStart;
				startNodeOne = node;
				startNodeTwo = connection;
			}

			if (lengthToDest < closestDestDist)
			{
				closestDestDist = lengthToDest;
				closestDestPoint = closestToDest;
				destNodeOne = node;
				destNodeTwo = connection;
			}
		}
	}

	if ((startNodeOne == destNodeOne || startNodeOne == destNodeTwo) &&
		(startNodeTwo == destNodeOne || startNodeTwo == destNodeTwo))
	{
		points->push_back(pos);
		points->push_back(dest);
		return;
	}

	Pathfinding::PointRelativeGraph start{
		pos, closestStartPoint,
		startNodeOne, startNodeTwo
	};

	Pathfinding::PointRelativeGraph end{
		dest, closestDestPoint,
		destNodeOne, destNodeTwo
	};

	// Find shortest path between the two points using A*
	AStar(start, end, points);
}

#ifdef USE_IMGUI
bool GraphManager::RenderUI(XMFLOAT3 posA, XMFLOAT3 posB)
{
	static bool showNodes = false;
	if (ImGui::Checkbox("Show Nodes", &showNodes))
	{
		for (auto &node : _nodes)
		{
			if (node == nullptr)
				continue;
			node->SetShowNode(showNodes);
		}
	}

	static bool showConnections = true;
	ImGui::Checkbox("Show Connections", &showConnections);
	if (showConnections)
	{
		for (auto &node : _nodes)
		{
			if (node == nullptr)
				continue;

			if (!node->DrawConnections())
			{
				ErrMsg("Failed to draw connections for GraphNodeBehaviour.");
				return false;
			}
		}
	}

	static bool showPath = false;
	ImGui::Checkbox("Show Path", &showPath);
	if (showPath)
	{
		static bool overlayPath = false;
		ImGui::Checkbox("Overlay", &overlayPath);

		std::vector<XMFLOAT3> points = {};
		GetPath(posA, posB, &points);

		DebugDrawer *drawer = DebugDrawer::GetInstance();
		drawer->DrawLineStrip(points.data(), static_cast<UINT>(points.size()), 0.25f, { 1,1,0,1 }, !overlayPath);
	}

	return true;
}
#endif

void GraphManager::CompleteDeserialization()
{
	for (auto &node : _nodes)
	{
		if (node == nullptr)
			continue;

		node->CompleteDeserialization();
		UpdateNode(node);
	}
}
