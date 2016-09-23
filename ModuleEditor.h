#pragma once
#include "Module.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "InfoWindows.h"
#include <list>

class FPSwindow;
class HardwareWindow;

class ModuleEditor : public Module
{
public:
	ModuleEditor(Application* app, bool start_enabled = true);
	~ModuleEditor();

	bool Start();
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	update_status UpdateEditor();

private:
	void CreatePoints(float2 min_max, int n);
	void CreateBoundingBox();
	void Render();

	//---- Imgui stuff
	void AboutMenu();
	void InfoMenu();
	void ShowFPSwindow();
	void ShowHardwareWindow();

private:
	bool box_render;
	int n_points;
	Cube_Prim bounding_box;
	float2 range;	
	list<Sphere_Prim> points;

	//-------Info stuff
	list<InfoWindows*> info_window;
	FPSwindow* fps_win = nullptr;
	HardwareWindow* hd_win = nullptr;



};

