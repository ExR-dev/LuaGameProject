#include "stdafx.h"
#include "PhysicsHandler.h"
#include "Scene.h"

PhysicsHandler::PhysicsHandler()
{
}

PhysicsHandler::~PhysicsHandler()
{
    ZoneScopedC(RandomUniqueColor());

    if (b2World_IsValid(m_worldId))
        b2DestroyWorld(m_worldId);
}

void PhysicsHandler::Setup()
{
    ZoneScopedC(RandomUniqueColor());

    const float lengthUnitsPerMeter = 1; //128 pixels per meter
    b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity.y = 0; //9.81f * lengthUnitsPerMeter; // Disable gravity
    m_worldId = b2CreateWorld(&worldDef);

    Assert(b2World_IsValid(m_worldId), "Invalid Box2D world!");
}

void PhysicsHandler::Update(lua_State* L, Scene* scene)
{
    ZoneScopedC(RandomUniqueColor());

    b2World_Step(m_worldId, Time::DeltaTime(), 4);

    const b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_worldId);
    for (int i = 0; i < sensorEvents.beginCount; i++)
    {
        ZoneNamedNC(sensorEventZone, "PhysicsHandler::Update Sensor Event", RandomUniqueColor(), true);

        const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];

        int entity = (int)b2Shape_GetUserData(event.sensorShapeId);
        int other = (int)b2Shape_GetUserData(event.visitorShapeId);

        if (scene->IsEntity(entity) && scene->IsEntity(other))
        {
            int luaCallback = scene->GetComponent<ECS::Collider>(entity).luaRef;

            lua_rawgeti(L, LUA_REGISTRYINDEX, luaCallback);
            lua_pushinteger(L, other);
            lua_pcall(L, 1, 0, 0);
        }
        else
            std::cout << "Invalid Entities" << std::endl;
    }
}

b2WorldId PhysicsHandler::GetWorldId() const
{
    return m_worldId;
}

b2BodyId PhysicsHandler::CreateRigidBody(int entity, const ECS::Collider &collider, const ECS::Transform &transform)
{
    ZoneScopedC(RandomUniqueColor());

    b2BodyId bodyId;

	b2Polygon polygon = b2MakeBox(fabsf(transform.Scale[0]) / 2, fabsf(transform.Scale[1]) / 2);

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_kinematicBody;
	bodyDef.position = { transform.Position[0], transform.Position[1] };

	bodyId = b2CreateBody(m_worldId, &bodyDef);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true; // Disable automatic resolving
    shapeDef.userData = (void*)entity;
    shapeDef.enableSensorEvents = true;
	b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    return bodyId;
}
