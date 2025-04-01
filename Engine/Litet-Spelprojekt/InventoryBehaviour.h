#pragma once
#include "Behaviour.h"
#include "BreadcrumbBehaviour.h"
#include "CompassBehaviour.h"
#include "PictureBehaviour.h"

enum class HandState
{
	Empty,
	Breadcrumbs,
	Compass,
	PicturePiece,

	Count
};

// Behaviour for the player
class InventoryBehaviour : public Behaviour
{
private:
	HandState _handState = HandState::Empty;

	UINT _breadcrumbCount = 15;
	BreadcrumbColor _breadcrumbColor = BreadcrumbColor::Red;
	
	MeshBehaviour *_heldBreadcrumb = nullptr;
	CompassBehaviour *_compass = nullptr;
	Behaviour *_heldItem = nullptr;

	InteractorBehaviour *_interactor = nullptr;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	InventoryBehaviour() = default;
	~InventoryBehaviour() = default;

	void AddBreadcrumbs(UINT count) { _breadcrumbCount += count; }

	void SetBreadcrumbColor(BreadcrumbColor color);

	bool SetInventoryItemParents(Entity* parent);
	void SetHeldItem(bool cycle = true, HandState handState = HandState::Empty);
	void SetHeldEnabled(bool state);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};

