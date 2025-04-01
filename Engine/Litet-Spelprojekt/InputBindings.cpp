#include "stdafx.h"
#include "InputBindings.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace InputBindings;

void BindingCollection::LoadBindings(const std::string &path)
{
	// TODO: Implement
	// For now, just set up some default bindings manually

	std::vector<std::unique_ptr<InputBindingStep>> steps;
	steps.resize(4);

	// Debug
	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftShift, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::Escape, KeyState::Pressed));
	SetBinding(InputAction::Exit, std::make_unique<InputBindingCombo>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftControl, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::Tab, KeyState::Pressed));
	SetBinding(InputAction::LockCursor, std::make_unique<InputBindingCombo>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftShift, KeyState::None));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftControl, KeyState::Held));
	steps[2] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::D, KeyState::Pressed));
	SetBinding(InputAction::CopySelected, std::make_unique<InputBindingCombo>(steps.data(), 3));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftShift, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftControl, KeyState::Held));
	steps[2] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::D, KeyState::Pressed));
	SetBinding(InputAction::SuperCopySelected, std::make_unique<InputBindingCombo>(steps.data(), 3));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftShift, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::D1, KeyState::Pressed));
	SetBinding(InputAction::CycleScene, std::make_unique<InputBindingCombo>(steps.data(), 2));

	SetBinding(InputAction::Save, std::make_unique<InputBindingSingle>(KeyCode::F5, KeyState::Pressed));
	SetBinding(InputAction::Load, std::make_unique<InputBindingSingle>(KeyCode::F9, KeyState::Pressed));

	// General
	SetBinding(InputAction::Pause, std::make_unique<InputBindingSingle>(KeyCode::Escape, KeyState::Pressed));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftControl, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::Enter, KeyState::Pressed));
	SetBinding(InputAction::Fullscreen, std::make_unique<InputBindingCombo>(steps.data(), 2));

	// Player Movement
	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::W, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::W, KeyState::Pressed));
	SetBinding(InputAction::WalkForward, std::make_unique<InputBindingSingle>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::S, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::S, KeyState::Pressed));
	SetBinding(InputAction::WalkBackward, std::make_unique<InputBindingSingle>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::A, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::A, KeyState::Pressed));
	SetBinding(InputAction::StrafeLeft, std::make_unique<InputBindingSingle>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::D, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::D, KeyState::Pressed));
	SetBinding(InputAction::StrafeRight, std::make_unique<InputBindingSingle>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftControl, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftControl, KeyState::Pressed));
	SetBinding(InputAction::Sneak, std::make_unique<InputBindingSingle>(steps.data(), 2));

	steps[0] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftShift, KeyState::Held));
	steps[1] = std::move(std::make_unique<InputBindingStepSimple>(KeyCode::LeftShift, KeyState::Pressed));
	SetBinding(InputAction::Sprint, std::make_unique<InputBindingSingle>(steps.data(), 2));

	// Player Actions
	SetBinding(InputAction::ShowPicture, std::make_unique<InputBindingSingle>(KeyCode::P, KeyState::Pressed));

	SetBinding(InputAction::PlaceBreadcrumb, std::make_unique<InputBindingSingle>(KeyCode::B, KeyState::Pressed));











	/*

	std::ifstream file("Content/Bindings.txt");
	std::string line;

	while (std::getline(file, line)) 
	{
		// Find the name of the object
		size_t space = line.find("ID:");
		std::string name = line.substr(0, space - 1);

		// Find the serialized ID
		size_t colon = line.find(":");
		size_t parentSpace = line.find(" ", space);
		UINT deserializedID = std::stoul(line.substr(colon + 1, parentSpace));

		// Find entity attributes
		size_t bracket = line.find("<");
		size_t parenthesis = line.find("(");
		std::string entline = line.substr(parenthesis + 1, bracket - parenthesis - 2);

		// Convert entity attributes to floats
		std::vector<float> entAttributes;
		std::istringstream stream(entline);
		std::string value;
		while (stream >> value) // Automatically handles spaces correctly
		{
			float attribute = stof(value);
			entAttributes.push_back(attribute);
		}


		// substring with all the behaviours data 
		std::string behavioursLine = line.substr(bracket);
		size_t behavioursSize = behavioursLine.find(">");

		// find parenthesis for one behaviour
		space = 0;
		bool behavioursLeft = behavioursSize > 1;

		while (behavioursLeft)
		{
			parenthesis = behavioursLine.find(")", space + 1);
			std::string currentBev = behavioursLine.substr(space + 1, parenthesis);

			size_t bevLineParenthesis = currentBev.find(")");
			std::string bevName = currentBev.substr(0, currentBev.find("("));

			size_t size = bevName.size();
			std::string bevAttributes = currentBev.substr(size + 1, bevLineParenthesis - size - 2);
		}
	}
	
	*/

}
