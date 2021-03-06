#include "Application.h"
#include "Component.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
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
	if (debug_frustum && go->isEnabled())
	{
		App->renderer3D->RenderFrustum(frustum, Green);	
	}

}

void ComponentCamera::UpdateTransform()
{
	if (go != nullptr)
	{
		float3 camera_scale(0.0f, frustum.verticalFov, frustum.horizontalFov);

		camera_transformation = (ComponentTransform*)go->GetComponent(Component::TRANSFORM);
		camera_transformation->SetScale(camera_scale);

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
		if (ImGui::CollapsingHeader("ID Component"))
		{
			ImGui::Text("ID Component: %d", Component::GetID());
		}

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

		ImGui::SameLine();
		bool culling_enabled = culling;
		if (ImGui::Checkbox("Culling",&culling_enabled))
		{
			if (culling_enabled)
			{
				culling = true;
			}
			else
			{
				culling = false;
			}
		}

			ImGui::Text("Near plane");
			float new_near = frustum.nearPlaneDistance;
			if (ImGui::SliderFloat("##near", &new_near,1.0f,4999.0f));
			{
				SetNearDistance(new_near);
			}

			ImGui::Text("Far plane");
			float new_far = frustum.farPlaneDistance;
			if (ImGui::SliderFloat("##far", &new_far, 1.0f, 5000.0f));
			{
				SetFarDistance(new_far);
			}

			ImGui::Text("FOV");
			float fov = field_of_view;
			if (ImGui::SliderFloat("##fov", &fov, 1.0f, 150.0f));
			{
				SetFieldOfView(fov);
			}
	}
}

void ComponentCamera::ToSave(Json & file_data) const
{
	Json data;
	data.AddInt("type", type);
	data.AddInt("ID Component", id);
	data.AddBool("enabled", enabled);

	data.AddBool("Culling", culling);
	data.AddBool("Debug Frustum", debug_frustum);
	data.AddFloatArray("Frustum Pos", frustum.pos.ptr());
	data.AddFloatArray("Frustum front", frustum.front.ptr());
	data.AddFloatArray("Frustum up", frustum.up.ptr());
	data.AddFloat("Near plane", frustum.nearPlaneDistance);
	data.AddFloat("Far plane", frustum.farPlaneDistance);
	data.AddFloat("FOV", field_of_view);
	data.AddFloat("Aspect ratio", aspect_ratio);

	file_data.AddArrayData(data);

}

void ComponentCamera::ToLoad(Json & file_data)
{
	id = file_data.GetInt("ID Component");
	enabled = file_data.GetBool("enabled");

	culling = file_data.GetBool("Culling");

	debug_frustum = file_data.GetBool("Debug Frustum");
	frustum.pos = file_data.GetFloat3("Frustum Pos");
	frustum.front = file_data.GetFloat3("Frustum front");
	frustum.up = file_data.GetFloat3("Frustum up");
	frustum.nearPlaneDistance = file_data.GetFloat("Near plane");
	frustum.farPlaneDistance = file_data.GetFloat("Far plane");
	field_of_view = file_data.GetFloat("FOV");
	aspect_ratio = file_data.GetFloat("Aspect Ratio");

	UpdateTransform();
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

LineSegment ComponentCamera::CastRay()
{
	float2 mouse = float2(App->input->GetMouseX(), App->input->GetMouseY());

	float normX = (mouse.x * 2.0f / SCREEN_WIDTH) - 1.0f;
	float normY = 1.0f - mouse.y * 2.0f / SCREEN_HEIGHT;

	LineSegment raycast = App->editor->main_camera_component->frustum.UnProjectLineSegment(normX,normY);

	return raycast;
}

void ComponentCamera::LookAt(const float3 & position)
{
	float3 direction = position - frustum.pos;
	float3x3 m = float3x3::LookAt(frustum.front, direction.Normalized(), frustum.up, float3::unitY);

	frustum.front = m.MulDir(frustum.front).Normalized();
	frustum.up = m.MulDir(frustum.up).Normalized();
}

bool ComponentCamera::ContainsAABB(const AABB & ref_box) const
{
	float3 corners[8];
	ref_box.GetCornerPoints(corners);

	Plane planes[6];
	frustum.GetPlanes(planes);

	for (int n_planes = 0; n_planes < 6; ++n_planes)
	{
		int in_count = 8;

		for (int i = 0; i < 8; ++i)
		{
			if (planes[n_planes].IsOnPositiveSide(corners[i]))
			{
				--in_count;
			}
		}

		if (in_count == 0)
		{
			return false;
			break;
		}
	}
	return true;
}
