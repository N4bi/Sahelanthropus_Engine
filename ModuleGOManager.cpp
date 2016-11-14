#include "Application.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "Component.h"
#include "RayCast.h"
#include "Imgui\imgui.h"


using namespace std;

ModuleGOManager::ModuleGOManager(Application * app, const char* name, bool start_enabled) : Module(app, name, start_enabled)
{

}

ModuleGOManager::~ModuleGOManager()
{

}

bool ModuleGOManager::Init(Json& config)
{
	bool ret = true;
	LOG("Init Game Object Manager");

	root = new GameObject(nullptr, "root");
	root->AddComponent(Component::TRANSFORM);

	return ret;
}

update_status ModuleGOManager::PreUpdate(float dt)
{
	//Delete game objects in the vector to delete 
	vector<GameObject*>::iterator it = to_delete.begin();
	while (it != to_delete.end())
	{
		delete(*it);
		++it;
	}

	to_delete.clear();

	if (root)
	{
		DoPreUpdate(dt, root);
	}


	return UPDATE_CONTINUE;
}

update_status ModuleGOManager::Update(float dt)
{

	if (root)
	{
		UpdateChilds(dt, root);
	}
	HierarchyInfo();
	EditorWindow();

	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		Ray raycast = App->editor->main_camera_component->DoRay(float2(App->input->GetMouseX(), App->input->GetMouseY()));
		game_object_on_editor = DoRaycast(raycast);
	}

	return UPDATE_CONTINUE;
}

bool ModuleGOManager::CleanUp()
{
	bool ret = true;

	delete root;

	game_object_on_editor = nullptr;
	root = nullptr;

	return ret;
}

GameObject* ModuleGOManager::CreateGameObject(GameObject* parent, const char* name)
{
	GameObject* ret = new GameObject(parent, name);
	if (parent == nullptr)
	{
		parent = root;
	}

	parent->childs.push_back(ret);

	return ret;
}

void ModuleGOManager::DeleteGameObject(GameObject * go)
{

	if (go != nullptr)
	{
		if (go->GetParent() != nullptr)
		{
			go->GetParent()->DeleteChilds(go);
	
		}
		go->DeleteAllChildren();
		to_delete.push_back(go);
	}
}

void ModuleGOManager::HierarchyInfo()
{
	ImGui::Begin("Hierarchy");

	ShowGameObjectsOnEditor(root->GetChilds());

	ImGui::End();
}


void ModuleGOManager::ShowGameObjectsOnEditor(const vector<GameObject*>* childs)
{
	vector<GameObject*>::const_iterator it = (*childs).begin();
	while (it != (*childs).end())
	{
		if ((*it) == game_object_on_editor)
		{
			ImGuiTreeNodeFlags_Framed;
		}
		
		if ((*it)->childs.size() > 0)
		{
			if (ImGui::TreeNodeEx((*it)->name_object.data(), ImGuiTreeNodeFlags_Framed))
			{
				if (ImGui::IsItemClicked())
				{
					game_object_on_editor = (*it);
				}
				ShowGameObjectsOnEditor((*it)->GetChilds());
				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::TreeNodeEx((*it)->name_object.data(), ImGuiTreeNodeFlags_Leaf))
			{
				if (ImGui::IsItemClicked())
				{
					game_object_on_editor = (*it);
				}
				ImGui::TreePop();
			}
		}
		++it;
	}
}

void ModuleGOManager::EditorWindow()
{
	ImGui::Begin("Editor");

	if (game_object_on_editor)
	{
		bool is_enabled = game_object_on_editor->isEnabled();
		if (ImGui::Checkbox(game_object_on_editor->name_object._Myptr(),&is_enabled))
		{
			if (is_enabled)
			{
				game_object_on_editor->Enable();
			}
			else
			{
				game_object_on_editor->Disable();
			}
		}

		ImGui::SameLine();
		
		bool wire_enabled = App->renderer3D->wireframe;
		if (ImGui::Checkbox("Wireframe", &wire_enabled))
		{
			if (wire_enabled)
			{
				App->renderer3D->wireframe = true;
			}
			else
			{
				App->renderer3D->wireframe = false;
			}
		}

		ImGui::SameLine();
		ImGui::TextColored(IMGUI_GREEN,"ID object: ");
		ImGui::SameLine();
		ImGui::Text("%d", game_object_on_editor->GetID());

		const vector<Component*>* components = game_object_on_editor->GetComponents();
		for (vector<Component*>::const_iterator component = (*components).begin(); component != (*components).end(); ++component)
		{
			(*component)->ShowOnEditor();
		}

	}

	ImGui::End();
}

GameObject * ModuleGOManager::DoRaycast(Ray & raycast)
{
	GameObject* ret = nullptr;
	list<GameObject*> objects_hit;

	CollectHits(root, raycast, objects_hit);

	list<GameObject*>::iterator it = objects_hit.begin();
	RayCast hit_point;

	for (list<GameObject*>::iterator it = objects_hit.begin(); it != objects_hit.end(); it++)
	{
		if ((*it)->DoRaycast(raycast, hit_point))
		{
			ret = (*it);
			break;
		}
	}

	return ret;
}

void ModuleGOManager::CollectHits(GameObject * go, Ray & raycast, list<GameObject*>& hits)
{	
	ComponentMesh* cmp_mesh = (ComponentMesh*) go->GetComponent(Component::MESH);

	if (go->bb != nullptr)
	{
		if (raycast.Intersects(*go->bb))
		{
			hits.push_back(go);
		}

		vector<GameObject*>::iterator it = go->childs.begin();
		while (it != go->childs.end())
		{
			CollectHits((*it), raycast, hits);
			++it;
		}
	}
}


void ModuleGOManager::SaveGameObjectsOnScene() const
{
	Json data;
	data.AddArray("Game Objects");

	root->Save(data);

	char* buff;
	size_t size = data.Save(&buff);

	App->fs->Save("Scene.json", buff, size);
	delete[] buff;
}

GameObject * ModuleGOManager::LoadGameObjectsOnScene(Json & game_objects)
{
	const char* name = game_objects.GetString("Name");
	int id = game_objects.GetInt("ID Game Object");
	int parent_id = game_objects.GetInt("ID Parent");
	bool enabled = game_objects.GetBool("enabled");

	//Search the parent of the game object
	GameObject* parent = nullptr;
	if (parent_id != 0 && root != nullptr)
	{
		parent = SearchGameObjectsByID(root, parent_id);
	}

	//Create the childs 
	GameObject* child = new GameObject(parent, name, id, enabled);

	if (parent != nullptr)
	{
		parent->childs.push_back(child);
	}

	//Attach the components
	Json component_data;
	int component_array_size = game_objects.GetArraySize("Components");
	for (uint i = 0; i < component_array_size; i++)
	{
		component_data = game_objects.GetArray("Components", i);
		int type = component_data.GetInt("type");

		Component* cmp_go = child->AddComponent((Component::Types)(type));
		cmp_go->ToLoad(component_data);

	}

	return child;
}

GameObject * ModuleGOManager::SearchGameObjectsByID(GameObject * first_go, int id) const
{
	GameObject* ret = nullptr;
	if (first_go != nullptr)
	{
		if (first_go->id == id)
		{
			ret = first_go;
		}
		else
		{
			const std::vector<GameObject*>* game_objects = first_go->GetChilds();
			vector<GameObject*>::const_iterator it = game_objects->begin();

			while (it != game_objects->end())
			{
				ret = SearchGameObjectsByID((*it), id);				
				if (ret != nullptr)
				{
					break;
				}
				++it;
			}
		}
	}

	return ret;
}

void ModuleGOManager::LoadScene(const char * directory)
{
	char* buff;
	uint size = App->fs->Load(directory, &buff);
	
	Json scene(buff);
	Json root;

	root = scene.GetArray("Game Objects",0);
	uint scene_size = scene.GetArraySize("Game Objects");

	//DeleteScene();

	for (uint i = 0; i < scene_size; i++)
	{
		if (i == 0)
		{
			this->root = LoadGameObjectsOnScene(scene.GetArray("Game Objects", i));
		}
		else
		{
			LoadGameObjectsOnScene(scene.GetArray("Game Objects", i));
		}	
	}

	delete[] buff;

}

void ModuleGOManager::DeleteScene()
{
	DeleteGameObject(root);
	game_object_on_editor = nullptr;
	root = nullptr;
}

GameObject* ModuleGOManager::GetRoot() const
{
	return root;
}

void ModuleGOManager::DoPreUpdate(float dt	,GameObject * go)
{
	if (root != go)
	{
		go->PreUpdate(dt);
	}
	vector<GameObject*>::const_iterator it = go->childs.begin();
	while (it != go->childs.end())
	{
		DoPreUpdate(dt, (*it));
		++it;
	}
}

void ModuleGOManager::UpdateChilds(float dt, GameObject * go)
{
	if (root != go)
	{
		go->Update(dt);
	}

	vector<GameObject*>::const_iterator it = go->childs.begin();
	while (it != go->childs.end())
	{
		UpdateChilds(dt, (*it));
		++it;
	}
}



