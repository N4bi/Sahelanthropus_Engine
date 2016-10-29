#include "Application.h"
#include "Component.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "Imgui\imgui.h"

ComponentCamera::ComponentCamera(Component::Types type) : Component(type)
{
	type = CAMERA;

	frustum.type = FrustumType::PerspectiveFrustum;
	frustum.pos = float3::zero;
	frustum.front = float3::unitZ;
	frustum.up = float3::unitY;
	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 1000.0f;
	frustum.verticalFov = DegToRad(field_of_view);

	SetAspectRatio(aspect_ratio);

}

ComponentCamera::~ComponentCamera()
{

}

void ComponentCamera::Update(float dt)
{
	if (debug_frustum)
	{
		App->renderer3D->RenderFrustum(frustum, Green);	
	}
}

void ComponentCamera::UpdateTransform()
{
	if (go != nullptr)
	{
		camera_transformation = (ComponentTransform*)go->GetComponent(Component::TRANSFORM);
		camera_transformation->SetScale(float3(0.0f, frustum.verticalFov,frustum.horizontalFov));

		float4x4 transformation = camera_transformation->GetWorldTransformationMatrix();
		frustum.pos = transformation.TranslatePart();
		frustum.front = transformation.WorldZ();
		frustum.up = transformation.WorldY();
	}	
}

void ComponentCamera::ShowOnEditor()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		bool is_enabled = debug_frustum;
		if (ImGui::Checkbox("Debug", &is_enabled))
		{
			if (is_enabled)
			{
				debug_frustum = true;
			}
			else
			{
				debug_frustum = false;
			}
		}

			ImGui::Text("Near plane");
			float new_near = frustum.nearPlaneDistance;
			if (ImGui::DragFloat("##near", &new_near,1.0f,1.0f,5000.0f));
			{
				SetNearDistance(new_near);
			}

			ImGui::Text("Far plane");
			float new_far = frustum.farPlaneDistance;
			if (ImGui::DragFloat("##far", &new_far, 1.0f, 1.0f, 5000.0f));
			{
				SetFarDistance(new_far);
			}

			ImGui::Text("FOV");
			float fov = field_of_view;
			if (ImGui::DragFloat("##fov", &fov, 1.0f, 1.0f, 150.0f));
			{
				SetFieldOfView(fov);
			}
	}
}


Frustum ComponentCamera::GetFrustum() const
{
	return frustum;
}

float ComponentCamera::GetNearDistance() const
{
	return frustum.nearPlaneDistance;
}

float ComponentCamera::GetFarDistance() const
{
	return frustum.farPlaneDistance;
}

float ComponentCamera::GetFieldOfView() const
{
	return DegToRad(frustum.verticalFov);
}

float ComponentCamera::GetAspectRatio() const
{
	return frustum.AspectRatio();
}

void ComponentCamera::SetNearDistance(float distance)
{
	if (distance > 0 && distance < frustum.farPlaneDistance)
	{
		frustum.nearPlaneDistance = distance;
	}
}

void ComponentCamera::SetFarDistance(float distance)
{
	if (distance > frustum.nearPlaneDistance)
	{
		frustum.farPlaneDistance = distance;
	}
}

void ComponentCamera::SetFieldOfView(float fov)
{
	aspect_ratio = frustum.AspectRatio();

	field_of_view = fov;
	frustum.verticalFov = DegToRad(fov);
	SetAspectRatio(aspect_ratio);
}

void ComponentCamera::SetAspectRatio(float aspect_ratio)
{
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov * 0.5f) * aspect_ratio);
}

float * ComponentCamera::GetViewMatrix()
{
	
	float4x4 m = frustum.ViewMatrix();
	m.Transpose();

	return (float*)m.v;
}

float * ComponentCamera::GetProjectionMatrix()
{
	
	float4x4 m = frustum.ProjectionMatrix();
	m.Transpose();

	return (float*) m.v;
}

void ComponentCamera::LookAt(const float3 & position)
{
	float3 direction = position - frustum.pos;
	float3x3 m = float3x3::LookAt(frustum.front, direction.Normalized(), frustum.up, float3::unitY);

	frustum.front = m.MulDir(frustum.front).Normalized();
	frustum.up = m.MulDir(frustum.up).Normalized();
}
