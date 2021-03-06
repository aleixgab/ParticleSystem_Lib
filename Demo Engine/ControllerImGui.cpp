#include "ControllerImGui.h"
#include "ControllerWindow.h"
#include "GameManager.h" 
#include "GameObject.h"

#include "imgui/imgui.h"
#include "3rdPart/includes/imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <ParticleSystem.h>

ControllerImGui::ControllerImGui(GameManager* mng) : Controller(mng)
{
}

ControllerImGui::~ControllerImGui()
{
}

bool ControllerImGui::Start()
{
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(Mng->window->window, true);
	ImGui_ImplOpenGL3_Init("#version 130");



	return true;
}

bool ControllerImGui::Update(float dt)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	Hierarchy();
	ObjInspector();
	PlayStop();
	
	TimeGraph();

	if (popParticle)
	{
		ImGui::OpenPopup("Select Max Particles");
	}

	if (ImGui::BeginPopupModal("Select Max Particles", (bool*)0, ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("Number aproximatly of particles that will be at the same time");
		ImGui::DragInt("##MaxPart",&maxParticles);
		if (ImGui::Button("Create", ImVec2(100.0f, 25.0f)))
		{
			Mng->scene->currGO->AddComponentEmitter(Mng->particle, maxParticles);
			popParticle = false;
		}
		ImGui::EndPopup();
	}
	return true;
}

void ControllerImGui::TimeGraph()
{
	if (glfwGetKey(Mng->window->window, GLFW_KEY_F2) == GLFW_PRESS)
		hasFps = !hasFps;
	if (hasFps)
	{
		if (ImGui::Begin("Times", NULL))
		{
			// Framerate
			char title[20];
			std::vector<float> framerateTrack = Mng->GetFps();
			sprintf_s(title, IM_ARRAYSIZE(title), "Framerate %.1f", framerateTrack.back());
			ImGui::PlotHistogram("##framerate", &framerateTrack.front(), framerateTrack.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));

			// Ms
			std::vector<float> msTrack = Mng->GetMsec();
			sprintf_s(title, IM_ARRAYSIZE(title), "Milliseconds %.1f", msTrack.back());
			ImGui::PlotHistogram("##milliseconds", &msTrack.front(), msTrack.size(), 0, title, 0.0f, 40.0f, ImVec2(310, 100));
		}
		ImGui::End();
	}
}

void ControllerImGui::Hierarchy()
{
	bool alwaysOpen = true;

	if (ImGui::Begin("Hiererchy", &alwaysOpen))
	{
		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("New Game Object"))
			{
				std::string str = "Random Object ";
				str.append(std::to_string(rand() % 100));
				Mng->gameObject->CreateGameObject(glm::vec3(0.0f), glm::quat(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f), str.data());
			}

			ImGui::MenuItem("Close");
			ImGui::EndMenu();
		}
		ImGui::Separator();


		ImGui::Text("Current Game Objects:");
		ImGui::Columns(1, "currGO");
		ImGui::Separator();
		for (std::list<GameObject*>::iterator iterator = Mng->gameObject->gameObjectVec.begin(); iterator != Mng->gameObject->gameObjectVec.end(); ++iterator)
		{
			if (ImGui::Selectable((*iterator)->name.c_str()))
			{
				Mng->scene->currGO = *iterator;
			}
		}
	}
	ImGui::End();
}

void ControllerImGui::ObjInspector()
{
	bool alwaysOpen = true;
	if (ImGui::Begin("Inspector", &alwaysOpen, ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{


		GameObject* currObject = Mng->scene->currGO;

		if (currObject != nullptr)
		{
			char name[50];
			strcpy_s(name, 50, currObject->name.c_str());
			if (ImGui::InputText("##Name", name, 50, ImGuiInputTextFlags_EnterReturnsTrue))
				currObject->name = name;

			ImGui::Checkbox("Active", &currObject->isActive);

			for (std::vector<Component*>::iterator comp = currObject->components.begin(); comp != currObject->components.end(); ++comp)
			{
				(*comp)->Inspector();
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Add new component"))
			{
				if (ImGui::MenuItem("Particle System", "", nullptr, !currObject->HasEmitter()))
					popParticle = true;
				ImGui::MenuItem("Cancel");
				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::Button("Delete GameObject", ImVec2(150, 25)))
			{
				currObject->Delete();
				Mng->scene->currGO = nullptr;
			}
		}
		else
		{
			ImGui::TextWrapped("Select a GameObject to view its components");
		}
	}
	ImGui::End();
}

void ControllerImGui::PlayStop()
{
	bool alwaysOpen = true;
	if (Mng->scene->currGO)
	{
		ComponentEmitter* component = Mng->scene->currGO->GetComponentEmitter();
		if (component)
		{
			if (ImGui::Begin("PlayStop", &alwaysOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
			{
				if (ImGui::Button("Play", { 50,25 }))
				{
					Part::StartEmmitter(component->emitter);
				}
				ImGui::SameLine();
				if (ImGui::Button("Pause", { 50,25 }))
				{
					Part::PauseEmmitter(component->emitter);
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop", { 50,25 }))
				{
					Part::StopEmitter(component->emitter);
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop All", { 50,25 }))
				{
					Part::StopAllEmitters();
				}
			}
			ImGui::End();
		}
	}
}

void ControllerImGui::Draw()
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ControllerImGui::CleanUp()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return true;
}
