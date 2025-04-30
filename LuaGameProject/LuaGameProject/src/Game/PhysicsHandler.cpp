#include "stdafx.h"
#include "PhysicsHandler.h"

PhysicsHandler::PhysicsHandler()
{
}

PhysicsHandler::~PhysicsHandler()
{
    if (b2World_IsValid(m_worldId))
        b2DestroyWorld(m_worldId);
}

void PhysicsHandler::Setup()
{
    const float lengthUnitsPerMeter = 1; //128 pixels per meter
    b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity.y = 0; //9.81f * lengthUnitsPerMeter; // Disable gravity
    m_worldId = b2CreateWorld(&worldDef);

    Assert(b2World_IsValid(m_worldId), "Invalid Box2D world!");
}

void PhysicsHandler::Update()
{
    b2World_Step(m_worldId, Time::DeltaTime(), 4);

    const b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_worldId);
    for (int i = 0; i < sensorEvents.beginCount; i++)
    {
        const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];

        float* scale = (float*)b2Shape_GetUserData(event.sensorShapeId);
        std::cout << "Collision: " << scale[0] << ", " << scale[1] << std::endl;
    }
}

b2WorldId PhysicsHandler::GetWorldId() const
{
    return m_worldId;
}

b2BodyId PhysicsHandler::CreateRigidBody(const ECS::Transform &transform)
{
    b2BodyId bodyId;

	b2Polygon polygon = b2MakeBox(fabsf(transform.Scale[0]) / 2, fabsf(transform.Scale[1]) / 2);

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { transform.Position[0], transform.Position[1] };

	bodyId = b2CreateBody(m_worldId, &bodyDef);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true; // Disable automatic resolving
    shapeDef.userData = (void*)transform.Scale;
    shapeDef.enableSensorEvents = true;
	b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    return bodyId;
}
