#pragma once
#include <vector>

#include "Transform.h"
#include "TimelineWaypoint.h"
#include "Time.h"
#include "EasingFunctions.h"

enum class SequenceStatus
{
	ALREADY_RUNNING = -3,
	SEQUENCE_ERROR = -2,
	INVALID_SEQUENCE = -1,
	FINISHED = 0,
	RUNNING = 1,
	NOT_RUNNING = 2,
	STOPPED = 3,
	LERP_TO_START = 4,
};

enum SequenceType
{
	SEQUENCE = 0,
	LERP_TO_POINT = 1,
};

class TimelineSequence
{
private:
	std::string _latestError = "";
	std::vector<TimelineWaypoint> _waypoints;
	float _elapsedTime = 0.0f;
	SequenceStatus _status = SequenceStatus::NOT_RUNNING;
	bool _loop = false;
	bool _lerpToStart = true;
	Transform* _transform = nullptr;
	float _lerpToStartTime = 1.0f;
	SequenceType _type = SequenceType::SEQUENCE;

	DirectX::XMFLOAT3A _startPosition = { 0, 0, 0 };
	DirectX::XMFLOAT4A _startRotation = { 0, 0, 0, 1 };
	DirectX::XMFLOAT3A _startScale = { 1, 1, 1 };

	EasingFunction _easing = EasingFunction::Func_None;

	void Lerp(Transform* transform, float t, TimelineWaypoint* start, TimelineWaypoint* end);

public:
	TimelineSequence() = default;
	~TimelineSequence() = default;

	TimelineSequence(const TimelineSequence& other) = default;
	TimelineSequence& operator=(const TimelineSequence& other) = default;
	TimelineSequence(TimelineSequence&& other) = default;
	TimelineSequence& operator=(TimelineSequence&& other) = default;

	void AddWaypoint(TimelineWaypoint waypoint);
	void SetType(SequenceType type);
	void SetType(int type);
	bool RemoveLastWaypoint();
	
	SequenceStatus Run(Transform* transform, bool loop = false, bool lerpToStart = true);
	
	//Move the logic in here to a private function?
	SequenceStatus Update(Time& time);
	SequenceStatus Stop();
	
	bool Validate();
	bool Serialize(std::string* code) const;
	bool Deserialize(const std::string& code);

	[[nodiscard]] std::vector<TimelineWaypoint>& GetWaypoints();
	[[nodiscard]] TimelineWaypoint GetWaypoint(int index);
	[[nodiscard]] const std::string GetError() const;
	[[nodiscard]] const SequenceStatus GetStatus() const;
	[[nodiscard]] const SequenceType GetType() const;
	[[nodiscard]] float GetLength() const;

	void SetEasing(EasingFunction type);

	/*[[nodiscard]] const float GetElapsedTime() const;
	[[nodiscard]] const bool GetLoop() const;
	[[nodiscard]] const bool GetLerpToStart() const;
	[[nodiscard]] const float GetLerpToStartTime() const;
	[[nodiscard]] const DirectX::XMFLOAT3A& GetStartPosition() const;
	[[nodiscard]] const DirectX::XMFLOAT4A& GetStartRotation() const;
	[[nodiscard]] const DirectX::XMFLOAT3A& GetStartScale() const;*/
};