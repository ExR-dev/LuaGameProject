#include "stdafx.h"
#include "CollisionHandler.h"
#include "Scene.h"
#include "Intersections.h"

#include <vector>
#include <map>
#include <algorithm>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace Collisions;
using namespace DirectX;

bool CollisionHandler::Initialize(Scene *scene)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(864282438, "Initialize Collisions");
#endif

	SceneHolder *sh = scene->GetSceneHolder();

	std::vector<Entity *> entities;
	sh->GetEntities(entities);

	_entitiesToCheck.reserve(entities.size());
	_collidersToCheck.reserve(entities.size());
	_colliderBehavioursToCheck.reserve(entities.size());

	// Check if entities have colliders
	for (int i = 0; i < entities.size(); i++)
	{
		ColliderBehaviour *colBehaviour;
		Entity *ent = entities[i];

		if (ent->GetBehaviourByType<ColliderBehaviour>(colBehaviour))
		{
			const Collider *col = colBehaviour->GetCollider();
			if (col)
			{
				if (col->colliderType != NULL_COLLIDER)
				{
					// Reset Intersecting status

					// Add collider and entity to be checked
					_collidersToCheck.push_back(col);
					_entitiesToCheck.push_back(ent);
					_colliderBehavioursToCheck.push_back(colBehaviour);
				}
			}
		}
	}

	return true;
}

bool CollisionHandler::CheckCollisions(Time &time, Scene *scene, ID3D11DeviceContext *context)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(864282438, "Handle Collisions");
#endif
	time.TakeSnapshot("CollisionChecks");
	
	std::unordered_multimap<const Collider *, CollisionData> intersections(_collidersToCheck.size()),
															 exiting(_collidersToCheck.size()),
															 entering(_collidersToCheck.size());

	SceneHolder *sh = scene->GetSceneHolder();

	if (sh->GetRecalculateColliders())
	{
		std::vector<Entity *> entities;
		sh->GetEntities(entities);

		_entitiesToCheck.clear();
		_collidersToCheck.clear();
		_colliderBehavioursToCheck.clear();
		// Check if entities have colliders
		for (int i = 0; i < entities.size(); i++)
		{
			std::vector<ColliderBehaviour*> colBehaviours;
			Entity *ent = entities[i];

			if (ent->GetBehavioursByType<ColliderBehaviour>(colBehaviours))
			{
				for (auto colBehaviour : colBehaviours)
				{
					const Collider *col = colBehaviour->GetCollider();
					if (col)
					{
						if (col->colliderType != NULL_COLLIDER)
						{
							// Add collider and entity to be checked
							_collidersToCheck.push_back(col);
							_entitiesToCheck.push_back(ent);
							_colliderBehavioursToCheck.push_back(colBehaviour);
						}
					}
				}
			}
		}
	}

	std::vector<bool> wasIntersecting(_entitiesToCheck.size());

	// TODO: Try insertions sort
	// TODO: Verify last index

	// Insertion sort
	{
		#ifdef PIX_TIMELINING
			PIXScopedEvent(542154321, "Collider Sorting");
		#endif

		int j;
		for (int i = 1; i < _collidersToCheck.size(); i++)
		{

			j = i;
			while (j > 0 && _collidersToCheck[j - 1]->GetMin().x > _collidersToCheck[j]->GetMin().x)
			{
				std::swap(_collidersToCheck[j], _collidersToCheck[j - 1]);
				std::swap(_entitiesToCheck[j], _entitiesToCheck[j - 1]);
				std::swap(_colliderBehavioursToCheck[j], _colliderBehavioursToCheck[j - 1]);
				j--;
			}
		}
	}

	{
		#ifdef PIX_TIMELINING
			PIXScopedEvent(95234353, "Adding Intersection Status");
		#endif

		for (int i = 0; i < _entitiesToCheck.size(); i++)
		{
			wasIntersecting[i] = _colliderBehavioursToCheck[i]->GetIntersecting();
			_colliderBehavioursToCheck[i]->SetIntersecting(false);
		}
	}


	/*
	// Bouble Sort
	for (int i = 0; i < _collidersToCheck.size()-1; i++)
	{
		for (int j = 0; j < _collidersToCheck.size()-1-i; j++)
		{
			if (_collidersToCheck[j + 1]->GetMin().x < _collidersToCheck[j]->GetMin().x)
			{
				std::swap(_collidersToCheck[j], _collidersToCheck[j + 1]);
				std::swap(_entitiesToCheck[j], _entitiesToCheck[j + 1]);

				{
					bool temp = wasIntersecting[j];
					wasIntersecting[j] = wasIntersecting[j + 1];
					wasIntersecting[j + 1] = temp;
				}
			}
		}
	}
	*/

	CollisionData data;
	ColliderBehaviour *colBehaviour, *colBehaviour2;

	// Check collisions
	{
#ifdef PIX_TIMELINING
			PIXScopedEvent(541023431, "Entity Check");
#endif

		// TODO: Try makeing parallel solution work

		//#pragma omp parallel for num_threads(PARALLEL_THREADS)
		for (int i = 0; i < _collidersToCheck.size(); i++)
		{
			// Main Enity
			colBehaviour = _colliderBehavioursToCheck[i];

			// Only check collider that have moved
			if (colBehaviour->GetToCheck())
			{
				const Collider *col1 = _collidersToCheck[i];

				for (int j = 0; j < _collidersToCheck.size(); j++)
				{
					if (i == j) continue;

					const Collider *col2 = _collidersToCheck[j];

					if (col2->GetMin().x > col1->GetMax().x) break;
					if (col2->GetMin().y > col1->GetMax().y || col1->GetMin().y > col2->GetMax().y) continue;
					if (col2->GetMin().z > col1->GetMax().z || col1->GetMin().z > col2->GetMax().z) continue;

					if (CheckIntersection(col1, col2, data))
					{
						//#pragma omp critical
						{
							data.other = col2;

							colBehaviour->SetIntersecting(true);
							intersections.insert({ col1, data });

							if (!wasIntersecting[i])
								entering.insert({ col1, data });

							// Chnage data values
							data.normal = { -1 * data.normal.x, -1 * data.normal.y, -1 * data.normal.z };
							data.other = col1;

							// Other entity
							colBehaviour2 = _colliderBehavioursToCheck[j];

							colBehaviour2->SetIntersecting(true);
							intersections.insert({ col2, data });

							if (!wasIntersecting[j])
								entering.insert({ col2, data });
						}
					}
				}

				if (!colBehaviour->GetIntersecting())
					colBehaviour->CheckDone();
			}
		}
	}

	// Check exiting collisions
	for (int i = 0; i < _entitiesToCheck.size(); i++)
		if (!_colliderBehavioursToCheck[i]->GetIntersecting() && wasIntersecting[i])
			exiting.insert({ _collidersToCheck[i], data });

	time.TakeSnapshot("CollisionChecks");
	
	// Call Intersection functions

	for (auto const &[ent, data] : intersections)
		ent->Intersection(data);

	for (auto const &[ent, data] : entering)
		ent->OnCollisionEnter(data);

	// TODO: Remove data from exit
	for (auto const &[ent, data] : exiting)
		ent->OnCollisionExit(data);


	return true;
}

bool CollisionHandler::CheckCollision(const Collider *col, Scene *scene, CollisionData &data)
{
	SceneHolder *sh = scene->GetSceneHolder();

	if (sh->GetRecalculateColliders())
	{
		std::vector<Entity *> entities;
		sh->GetEntities(entities);

		_entitiesToCheck.clear();
		_collidersToCheck.clear();
		_colliderBehavioursToCheck.clear();
		// Check if entities have colliders
		for (int i = 0; i < entities.size(); i++)
		{
			std::vector<ColliderBehaviour*> colBehaviours;
			Entity *ent = entities[i];

			if (ent->GetBehavioursByType<ColliderBehaviour>(colBehaviours))
			{
				for (auto colBehaviour : colBehaviours)
				{
					const Collider *col = colBehaviour->GetCollider();
					if (col)
					{
						if (col->colliderType != NULL_COLLIDER)
						{
							// Add collider and entity to be checked
							_collidersToCheck.push_back(col);
							_entitiesToCheck.push_back(ent);
							_colliderBehavioursToCheck.push_back(colBehaviour);
						}
					}
				}
			}
		}
	}


	for (int j = 0; j < _collidersToCheck.size(); j++)
	{
		const Collider *col2 = _collidersToCheck[j];

		if (col2->GetMin().x > col->GetMax().x) break;
		if (col2->GetMin().y > col->GetMax().y || col->GetMin().y > col2->GetMax().y) continue;
		if (col2->GetMin().z > col->GetMax().z || col->GetMin().z > col2->GetMax().z) continue;

		if (CheckIntersection(col, col2, data))
		{
			data.other = col2;
			return true;
		}
	}


	return false;
}
