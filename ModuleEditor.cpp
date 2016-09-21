#include "Application.h"
#include "ModuleEditor.h"
#include "Primitive.h"
#include "Imgui\imgui.h"
#include "FPSwindow.h"
#include "InfoWindows.h"
#include "MathGeoLib\include\Algorithm\Random\LCG.h"


ModuleEditor::ModuleEditor(Application* app, bool start_enabled) : Module(app, start_enabled)
{	
	
}

ModuleEditor::~ModuleEditor()
{}

// Load assets
bool ModuleEditor::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	n_points = 0;
	range.x = 0.0f;
	range.y = 0.0f;
	box_render = false;

	info_window.push_back(fps_win = new FPSwindow());


	return ret;
}

// Load assets
bool ModuleEditor::CleanUp()
{
	LOG("Unloading Intro scene");

	list<InfoWindows*>::iterator it = info_window.begin();
	while (it != info_window.end())
	{
		delete (*it);
		++it;
	}

	return true;
}

// Update: draw background
update_status ModuleEditor::Update(float dt)
{
	update_status ret = UpdateEditor();

	Render();
	


	return ret;
}

update_status ModuleEditor::PostUpdate(float dt)
{

	return UPDATE_CONTINUE;
}

void ModuleEditor::CreatePoints(float2 min_max, int n)
{

	LCG random;

	for (int i = 0; i < n; i++)
	{
		vec position;
		position.x = random.Int(min_max.x, min_max.y);
		position.y = random.Int(min_max.x, min_max.y);
		position.z = random.Int(min_max.x, min_max.y);

		Sphere_Prim s;
		s.radius = 0.05f;
		s.color.b = 255.0f;
		s.SetPos(position.x, position.y, position.z);
		points.push_back(s);
		
	}
}

void ModuleEditor::CreateBoundingBox()
{
	AABB box;
	list<Sphere_Prim>::iterator it = points.begin();

	box.minPoint.x = box.maxPoint.x = (*it).GetPos().x;
	box.minPoint.y = box.maxPoint.y = (*it).GetPos().y;
	box.minPoint.z = box.maxPoint.z = (*it).GetPos().z;

	while (it != points.end())
	{
		vec position = (*it).GetPos();

		if (position.x < box.minPoint.x)
			box.minPoint.x = position.x;

		if (position.x > box.maxPoint.x)
			box.maxPoint.x = position.x;

		if (position.y < box.minPoint.y)
			box.minPoint.y = position.y;

		if (position.y > box.maxPoint.y)
			box.maxPoint.y = position.y;

		if (position.z < box.minPoint.z)
			box.minPoint.z = position.z;

		if (position.z > box.maxPoint.z)
			box.maxPoint.z = position.z;

		++it;		
	}

	vec position2;

	position2.x = (box.maxPoint.x - box.minPoint.x) / 2.0f + box.minPoint.x;
	position2.y = (box.maxPoint.y - box.minPoint.y) / 2.0f + box.minPoint.y;
	position2.z = (box.maxPoint.z - box.minPoint.z) / 2.0f + box.minPoint.z;

	bounding_box.SetPos(position2.x, position2.y, position2.z);

	vec size;

	size.x = box.Size().x;
	size.y = box.Size().y;
	size.z = box.Size().z;

	bounding_box.size = size;

	bounding_box.wire = false;

	box_render = true;

}


void ModuleEditor::Render()
{
	list<Sphere_Prim>::iterator p = points.begin();
	while (p != points.end())
	{
		(*p).Render();
		++p;
	}

	if (box_render)
	{
		bounding_box.Render();
	}
}

update_status ModuleEditor::UpdateEditor()
{
	update_status ret = UPDATE_CONTINUE;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Configuration"))
		{
			InfoMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			AboutMenu();
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Close"))
		{
			return UPDATE_STOP;
		}
		ImGui::EndMainMenuBar();
	}

	bool open = false;
	if (!ImGui::Begin("Window", &open))
	{
		ImGui::End();
		return ret;
	}

	if (ImGui::CollapsingHeader("Random points"))
	{
		ImGui::InputFloat2("min range/max range", range.ptr());
		ImGui::InputInt("Number of points", &n_points);
		if (ImGui::Button("Create points"))
		{
			CreatePoints(range, n_points);
		}
		if (ImGui::Button("Bounding box"))
		{
			CreateBoundingBox();
		}
	}
	ImGui::End();

	list<InfoWindows*>::iterator it = info_window.begin();
	while (it != info_window.end())
	{
		(*it)->Render();
		++it;
	}

	return ret;
}
void ModuleEditor::AboutMenu()
{
	ImGui::BulletText("Sahelanthropus Engine\n"
						"3D Engine created for the subject with the same name"
		);
	ImGui::BulletText("Author: Ivan Perez Latorre");
	ImGui::BulletText(
		"Libraries: \n"

			"- Bullet\n"
			"- SDL 2.0.4\n"
			"- MathGeoLib\n"
			"- ImGui\n"
			"- OpenGL 4.5.0\n"
			"- Glew 2.0.0"

	);
}

void ModuleEditor::InfoMenu()
{
	if (ImGui::BeginMenu("Info"))
	{
		if (ImGui::MenuItem("FPS info"))
		{
			ShowFPSwindow();
		}

		ImGui::EndMenu();
	}
}

void ModuleEditor::ShowFPSwindow()
{
	fps_win->SetActive(true);
}