#include "stdafx.h"
#include "TransformGizmoBehaviour.h"
#include "DebugPlayerBehaviour.h"
#include "Entity.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

#ifdef DEBUG_BUILD
bool TransformGizmoBehaviour::Start()
{
	if (_name == "")
		_name = "TransformGizmoBehaviour"; // For categorization in ImGui.

	Content *content = GetScene()->GetContent();

	_translationMeshID = content->GetMeshID("Mesh_TranslationGizmo");
	_rotationMeshID = content->GetMeshID("Mesh_RotationGizmo");
	_scalingMeshID = content->GetMeshID("Mesh_ScalingGizmo");

	Material mat;
	mat.textureID = content->GetTextureID("Tex_TransformGizmo");
	mat.ambientID = content->GetTextureID("Tex_TransformGizmo");
	mat.samplerID = content->GetSamplerID("SS_Point");
	mat.vsID = content->GetShaderID("VS_Geometry");

	BoundingOrientedBox bounds = { {0,0,0},{1,1,1},{0,0,0,1} };
	_gizmoMesh = new MeshBehaviour(bounds, _translationMeshID, &mat, false, false, true);
	if (!_gizmoMesh->Initialize(GetEntity()))
	{
		ErrMsg("Failed to bind mesh to transform gizmo entity!");
		return false;
	}

	return true;
}

bool TransformGizmoBehaviour::Update(Time &time, const Input &input)
{
	Scene *scene = GetScene();
	SceneHolder *sceneHolder = scene->GetSceneHolder();
	DebugPlayerBehaviour *debugPlayer = scene->GetDebugPlayer();
	DebugDrawer *debugDrawer = scene->GetDebugDrawer();
	std::vector<std::unique_ptr<Entity>> *globalEntities = scene->GetGlobalEntities();
	CameraBehaviour *viewCamera = scene->GetViewCamera();

	TransformationType newEditType = debugPlayer->GetEditType();
	if (newEditType != _editType)
	{
		_editType = newEditType;
		switch (_editType) // Set gizmo mesh based on edit type.
		{
		case Translate:
			_gizmoMesh->SetMesh(_translationMeshID);
			break;

		case Rotate:
			_gizmoMesh->SetMesh(_rotationMeshID);
			break;

		case Scale:
			_gizmoMesh->SetMesh(_scalingMeshID);
			break;

		default:
			break;
		}
	}

	bool gridSnap = (input.GetKey(KeyCode::LeftControl) == KeyState::Held);

	_editSpace = debugPlayer->GetEditSpace();
	int currSelection = debugPlayer->GetSelection();

	Entity 
		*selection = (currSelection < 0) ? nullptr : sceneHolder->GetEntity(currSelection),
		*gizmo = nullptr;

	if (!selection)
	{
		_selectedAxisID = -1;
		_gizmoMesh->SetEnabled(false);
		return true;
	}

	float distanceScalar = 1.0f; // Scale gizmo to keep screen-size constant.
	if (viewCamera)
	{
		// size = 2.0 * distance * tan(angle * 0.5)
		float angularSize = 3.0f * _gizmoScale * DEG_TO_RAD;

		XMFLOAT3A camPos = viewCamera->GetTransform()->GetPosition(World);
		XMFLOAT3A entPos = selection->GetTransform()->GetPosition(World);
		float dist = XMVectorGetX(XMVector3Length(Load(camPos) - Load(entPos)));

		distanceScalar = 2.0f * dist * tanf(angularSize * 0.5f);
	}

	for (auto &entPtr : *globalEntities)
	{
		if (entPtr->GetName() == "Transform Gizmo")
		{
			gizmo = entPtr.get();
			break;
		}
	}

	if (gizmo)
	{
		_gizmoMesh->SetEnabled(true);
		Transform *gizmoTransform = gizmo->GetTransform();

		GetTransform()->SetMatrix(gizmoTransform->GetWorldMatrix(), World);
		GetTransform()->SetScale({ distanceScalar, distanceScalar, distanceScalar }, Local);
	}

	if (!input.IsCursorLocked() && input.GetKey(KeyCode::M1) == KeyState::Pressed)
	{
		WindowSize windowSize = input.GetActiveWindowSize();

		XMFLOAT2A mousePos = { input.GetMouse().x, input.GetMouse().y };
		XMFLOAT2A screenSize = { (float)windowSize.width, (float)windowSize.height };

		XMFLOAT3A rayOrigin, rayDir;
		scene->GetViewCamera()->GetViewRay(mousePos, screenSize, rayOrigin, rayDir);

		XMVECTOR rayOriginVec = Load(rayOrigin);
		XMVECTOR rayDirVec = Load(rayDir);

		_selectedAxisID = -1;
		switch (_editType) // Perform different intersection tests based on edit type.
		{
		case Translate: 
		case Scale: 
		{
			// Perform intersection test on axis-arrrows, set selected axis if hit.
			float len = _gizmoLen * distanceScalar;
			float hLen = len * 0.75f;
			float width = 0.13f * distanceScalar;
			float widthSq = width*width;

			_prevEntPos = selection->GetTransform()->GetPosition(World);
			XMVECTOR entPos = Load(_prevEntPos);

			XMFLOAT3A r, u, f;
			GetTransform()->GetAxes(&r, &u, &f, World);

			XMVECTOR right = Load(r);
			XMVECTOR up = Load(u);
			XMVECTOR forward = Load(f);

			XMVECTOR rayLineVec = rayOriginVec + rayDirVec;

			XMVECTOR xCross = XMVector3Cross(entPos - rayOriginVec, right);
			XMVECTOR xPlane = XMPlaneFromPoints(entPos, entPos + right, entPos + xCross);
			XMVECTOR xIntersectPoint = XMPlaneIntersectLine(xPlane, rayOriginVec, rayLineVec);
			bool skipX = XMVectorGetX(XMVector3Dot(rayDirVec, xIntersectPoint - rayOriginVec)) <= 0.0f;
				
			XMVECTOR yCross = XMVector3Cross(entPos - rayOriginVec, up);
			XMVECTOR yPlane = XMPlaneFromPoints(entPos, entPos + up, entPos + yCross);
			XMVECTOR yIntersectPoint = XMPlaneIntersectLine(yPlane, rayOriginVec, rayLineVec);
			bool skipY = XMVectorGetX(XMVector3Dot(rayDirVec, yIntersectPoint - rayOriginVec)) <= 0.0f;
				
			XMVECTOR zCross = XMVector3Cross(entPos - rayOriginVec, forward);
			XMVECTOR zPlane = XMPlaneFromPoints(entPos, entPos + forward, entPos + zCross);
			XMVECTOR zIntersectPoint = XMPlaneIntersectLine(zPlane, rayOriginVec, rayLineVec);
			bool skipZ = XMVectorGetX(XMVector3Dot(rayDirVec, zIntersectPoint - rayOriginVec)) <= 0.0f;

			float shortestDistance = FLT_MAX;
			float tempDist = 0.0f;

			if (!skipX)
			{
				float projectedDist = XMVectorGetX(XMVector3Dot(xIntersectPoint - entPos, right));
					
				if (projectedDist >= 0.0f && projectedDist <= len)
				{
					float radSq = XMVectorGetX(XMVector3LengthSq(xIntersectPoint - (entPos + right * projectedDist)));
					if (radSq <= widthSq)
					{
						tempDist = XMVectorGetX(XMVector3LengthSq(xIntersectPoint - rayOriginVec));
						if (tempDist < shortestDistance)
						{
							shortestDistance = tempDist;
							_selectedAxisID = 0;
						}
					}
				}
			}
			if (!skipY)
			{
				float projectedDist = XMVectorGetX(XMVector3Dot(yIntersectPoint - entPos, up));
					
				if (projectedDist >= 0.0f && projectedDist <= len)
				{
					float radSq = XMVectorGetX(XMVector3LengthSq(yIntersectPoint - (entPos + up * projectedDist)));
					if (radSq <= widthSq)
					{
						tempDist = XMVectorGetX(XMVector3LengthSq(yIntersectPoint - rayOriginVec));
						if (tempDist < shortestDistance)
						{
							shortestDistance = tempDist;
							_selectedAxisID = 1;
						}
					}
				}
			}
			if (!skipZ)
			{
				float projectedDist = XMVectorGetX(XMVector3Dot(zIntersectPoint - entPos, forward));
					
				if (projectedDist >= 0.0f && projectedDist <= len)
				{
					float radSq = XMVectorGetX(XMVector3LengthSq(zIntersectPoint - (entPos + forward * projectedDist)));
					if (radSq <= widthSq)
					{
						tempDist = XMVectorGetX(XMVector3LengthSq(zIntersectPoint - rayOriginVec));
						if (tempDist < shortestDistance)
						{
							shortestDistance = tempDist;
							_selectedAxisID = 2;
						}
					}
				}
			}

			if (_selectedAxisID >= 0)
			{
				switch (_selectedAxisID)
				{
				case 0:
					_activeAxis = GetTransform()->GetRight(World);
					break;

				case 1:
					_activeAxis = GetTransform()->GetUp(World);
					break;

				case 2:
					_activeAxis = GetTransform()->GetForward(World);
					break;
				}

				XMVECTOR entPos = Load(_prevEntPos);
				XMVECTOR axis = Load(_activeAxis);

				XMVECTOR rayHitPos = rayOriginVec + (rayDirVec * sqrt(shortestDistance));
				float projectedDist = XMVectorGetX(XMVector3Dot(rayHitPos - entPos, axis));

				if (gridSnap)
					projectedDist = std::roundf(projectedDist / _posGridSize) * _posGridSize;

				if (_editType == Translate)
				{
					//if (gridSnap)
					//	projectedDist = std::roundf(projectedDist / _posGridSize) * _posGridSize;
				}
				else if (_editType == Scale)
				{
					_initialEntScale = selection->GetTransform()->GetScale(World);
					XMVECTOR initialScaleVec = Load(_initialEntScale);

					float initialScaleInAxis = XMVectorGetX(XMVector3Dot(initialScaleVec, axis));
					_scaleFactor = 1.0f / projectedDist;

					//if (gridSnap)
					//	_scaleFactor = std::roundf(_scaleFactor / _posGridSize) * _posGridSize;
				}

				Store(_prevSelectPos, entPos + (axis * projectedDist));
			}
			break;
		}

		case Rotate:
		{
			// Perform intersection test on axis-discs, set selected axis to normal axis of disc if hit.
			float innerRad = 1.28f * distanceScalar;
			float outerRad = 1.5f * distanceScalar;
			float innerRadSq = innerRad * innerRad;
			float outerRadSq = outerRad * outerRad;
				
			XMFLOAT3A r, u, f;
			GetTransform()->GetAxes(&r, &u, &f, World);

			XMVECTOR right = Load(r);
			XMVECTOR up = Load(u);
			XMVECTOR forward = Load(f);

			XMVECTOR origin = Load(GetTransform()->GetPosition(World));
			XMVECTOR rayLineVec = rayOriginVec + rayDirVec;
			XMVECTOR intersectionPoint;

			XMVECTOR xPlaneIntersection = XMPlaneIntersectLine(
				XMPlaneFromPointNormal(origin, right),
				rayOriginVec,
				rayLineVec
			);
			bool skipX = XMVectorGetX(XMVector3Dot(rayDirVec, xPlaneIntersection - rayOriginVec)) <= 0.0f;

			XMVECTOR yPlaneIntersection = XMPlaneIntersectLine(
				XMPlaneFromPointNormal(origin, up),
				rayOriginVec,
				rayLineVec
			);
			bool skipY = XMVectorGetX(XMVector3Dot(rayDirVec, yPlaneIntersection - rayOriginVec)) <= 0.0f;

			XMVECTOR zPlaneIntersection = XMPlaneIntersectLine(
				XMPlaneFromPointNormal(origin, forward),
				rayOriginVec,
				rayLineVec
			);
			bool skipZ = XMVectorGetX(XMVector3Dot(rayDirVec, zPlaneIntersection - rayOriginVec)) <= 0.0f;

			float shortestDistance = FLT_MAX;
			float tempDist = 0.0f;

			if (!skipX)
			{
				float radSq = XMVectorGetX(XMVector3LengthSq(xPlaneIntersection - origin));

				if (radSq >= innerRadSq && radSq <= outerRadSq)
				{
					tempDist = XMVectorGetX(XMVector3LengthSq(xPlaneIntersection - rayOriginVec));
					if (tempDist < shortestDistance)
					{
						intersectionPoint = xPlaneIntersection;
						shortestDistance = tempDist;
						_selectedAxisID = 0;
					}
				}
			}
			if (!skipY)
			{
				float radSq = XMVectorGetX(XMVector3LengthSq(yPlaneIntersection - origin));

				if (radSq >= innerRadSq && radSq <= outerRadSq)
				{
					tempDist = XMVectorGetX(XMVector3LengthSq(yPlaneIntersection - rayOriginVec));
					if (tempDist < shortestDistance)
					{
						intersectionPoint = yPlaneIntersection;
						shortestDistance = tempDist;
						_selectedAxisID = 1;
					}
				}
			}
			if (!skipZ)
			{
				float radSq = XMVectorGetX(XMVector3LengthSq(zPlaneIntersection - origin));

				if (radSq >= innerRadSq && radSq <= outerRadSq)
				{
					tempDist = XMVectorGetX(XMVector3LengthSq(zPlaneIntersection - rayOriginVec));
					if (tempDist < shortestDistance)
					{
						intersectionPoint = zPlaneIntersection;
						shortestDistance = tempDist;
						_selectedAxisID = 2;
					}
				}
			}

			if (_selectedAxisID >= 0)
			{
				_prevEntPos = selection->GetTransform()->GetPosition(World);
				_initialEntRot = selection->GetTransform()->GetRotation(World);

				switch (_selectedAxisID)
				{
				case 0:
					_activeAxis = GetTransform()->GetRight(World);
					break;

				case 1:
					_activeAxis = GetTransform()->GetUp(World);
					break;

				case 2:
					_activeAxis = GetTransform()->GetForward(World);
					break;
				}

				Store(_prevSelectPos, intersectionPoint);
			}
			break;
		}
		}
	}

	if (_selectedAxisID >= 0)
	{
		if (input.GetKey(KeyCode::M1) == KeyState::Released)
		{
			_selectedAxisID = -1;
			return true;
		}

		XMVECTOR entPos = Load(_prevEntPos);
		XMVECTOR axis = Load(_activeAxis);

		WindowSize windowSize = input.GetActiveWindowSize();

		XMFLOAT2A mousePos = { input.GetMouse().x, input.GetMouse().y };
		XMFLOAT2A screenSize = { (float)windowSize.width, (float)windowSize.height };

		XMFLOAT3A rayOrigin, rayDir;
		scene->GetViewCamera()->GetViewRay(mousePos, screenSize, rayOrigin, rayDir);

		XMVECTOR rayOriginVec = Load(rayOrigin);
		XMVECTOR rayDirVec = Load(rayDir);
		XMVECTOR rayLineVec = rayOriginVec + rayDirVec;

		switch (_editType)
		{
		case Translate:
		case Scale:
		{
			XMVECTOR cross = XMVector3Cross(entPos - rayOriginVec, axis);
			XMVECTOR plane = XMPlaneFromPoints(entPos, entPos + axis, entPos + cross);
			XMVECTOR intersectPoint = XMPlaneIntersectLine(plane, rayOriginVec, rayLineVec);

			float projectedDist = XMVectorGetX(XMVector3Dot(intersectPoint - entPos, axis));

			if (_editType == Translate)
			{
				if (gridSnap)
					projectedDist = std::roundf(projectedDist / _posGridSize) * _posGridSize;

				XMVECTOR newSelectPos = entPos + (axis * projectedDist);

				XMVECTOR selectDiff = newSelectPos - Load(_prevSelectPos);
				Store(_prevSelectPos, newSelectPos);

				XMVECTOR newEntPosVec = entPos + selectDiff;

				XMFLOAT3A newEntPos;
				Store(newEntPos, newEntPosVec);

				selection->GetTransform()->SetPosition(newEntPos, World);
			}
			else
			{
				XMVECTOR initialScaleVec = Load(_initialEntScale);
				float initialScaleInAxis = XMVectorGetX(XMVector3Dot(initialScaleVec, axis));

				float scaledProjDist = projectedDist * _scaleFactor;
				if (gridSnap)
					scaledProjDist = std::roundf(scaledProjDist / _posGridSize) * _posGridSize;

				float newEntScaleFloat = (scaledProjDist * initialScaleInAxis);
				//if (gridSnap)
				//	newEntScaleFloat = std::roundf(newEntScaleFloat / _posGridSize) * _posGridSize;

				XMVECTOR newEntScaleVec = (initialScaleVec - (axis * initialScaleInAxis)) + (axis * newEntScaleFloat);

				XMFLOAT3A newEntScale;
				Store(newEntScale, newEntScaleVec);

				selection->GetTransform()->SetScale(newEntScale, Local);
			}
			break;
		}

		case Rotate:
		{
			XMVECTOR plane = XMPlaneFromPointNormal(entPos, axis);
			XMVECTOR intersectPoint = XMPlaneIntersectLine(plane, rayOriginVec, rayLineVec);

			XMVECTOR prevDir = XMVector3Normalize(Load(_prevSelectPos) - entPos);
			XMVECTOR currDir = XMVector3Normalize(intersectPoint - entPos);
			Store(_prevSelectPos, intersectPoint);

			// Calculate signed angle between previous and current intersection point around axis.
			float dot = XMVectorGetX(XMVector3Dot(prevDir, currDir));
			float det = XMVectorGetX(XMVector3Dot(XMVector3Cross(prevDir, currDir), axis));
			float angle = atan2(det, dot);

			if (gridSnap)
				angle = std::roundf(angle / XMConvertToRadians(_rotGridSize)) * XMConvertToRadians(_rotGridSize);

			if (angle > -0.001 && angle < 0.001)
				Store(_prevSelectPos, entPos + prevDir);
			else
				selection->GetTransform()->RotateAxis(_activeAxis, angle, World);
			break;
		}
		}

		_prevEntPos = selection->GetTransform()->GetPosition(World);
	}

	return true;
}

void TransformGizmoBehaviour::SetGridSize(float size)
{
	_posGridSize = size;
}

void TransformGizmoBehaviour::SetGizmoScale(float scale)
{
	_gizmoScale = scale;
}
#endif