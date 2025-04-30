#include "stdafx.h"
#include "PhysicsHandler.h"

struct CallbackDef
{
    int callbackRef;
    int entityId;
};

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

void PhysicsHandler::Update(lua_State* L)
{
    b2World_Step(m_worldId, Time::DeltaTime(), 4);

    const b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_worldId);
    for (int i = 0; i < sensorEvents.beginCount; i++)
    {
        const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];

        int luaCallback = ((ECS::Collider*)b2Shape_GetUserData(event.sensorShapeId))->luaRef;
        int otherEnt = ((ECS::Collider*)b2Shape_GetUserData(event.visitorShapeId))->entity;

        lua_rawgeti(L, LUA_REGISTRYINDEX, luaCallback);
        lua_pushnumber(L, otherEnt);
        lua_pcall(L, 1, 0, 0);
    }
}

b2WorldId PhysicsHandler::GetWorldId() const
{
    return m_worldId;
}

b2BodyId PhysicsHandler::CreateRigidBody(int entity, const ECS::Collider &collider, const ECS::Transform &transform)
{
    b2BodyId bodyId;

    CallbackDef callbackDef = { collider.luaRef, entity };

	b2Polygon polygon = b2MakeBox(fabsf(transform.Scale[0]) / 2, fabsf(transform.Scale[1]) / 2);

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_kinematicBody;
	bodyDef.position = { transform.Position[0], transform.Position[1] };

	bodyId = b2CreateBody(m_worldId, &bodyDef);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true; // Disable automatic resolving
    shapeDef.userData = (void*)&collider;
    shapeDef.enableSensorEvents = true;
	b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    return bodyId;
}
