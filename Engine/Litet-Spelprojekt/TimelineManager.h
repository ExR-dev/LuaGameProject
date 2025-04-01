#pragma once
#include <unordered_map>

#include "TimelineSequence.h"

class TimelineManager
{
private:
	std::unordered_map<std::string, TimelineSequence> _sequences;
	std::vector<std::string> _runningSequences;

	// For ImGui
	int selectedSequenceIndex = -1;
	std::string selectedSequence = "";
	bool creatingSequence = false;
	TimelineSequence newSequence = TimelineSequence();

public:
	SequenceStatus RunSequence(const std::string& sequenceName, Transform* transform, bool loop = false, bool lerpToStart = true);
	SequenceStatus StopSequence(const std::string& sequenceName);

	bool Update(Time& time);
	bool AddSequence(const std::string sequenceName, TimelineSequence sequence);
	bool RemoveSequence(const std::string sequenceName);

	bool Serialize(std::string *code) const;
	bool Deserialize();
	
	[[nodiscard]] bool RenderUI(Transform* transform);

	[[nodiscard]] std::unordered_map<std::string, TimelineSequence> GetSequences() const;
	[[nodiscard]] TimelineSequence GetSequence(std::string sequenceName) const;
};