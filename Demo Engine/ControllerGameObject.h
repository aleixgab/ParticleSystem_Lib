#ifndef __CONTROLLER_GAMEOBJECT_H__
#define __CONTROLLER_GAMEOBJECT_H__

#include "Controller.h"
#include "GameObject.h"
#include <list>

class ControllerGameObject : public Controller
{
public:

	ControllerGameObject(GameManager* mng);
	~ControllerGameObject();

	bool Start();
	bool Update(float dt);
	bool CleanUp();

	GameObject* CreateGameObject(glm::vec3 position, glm::quat rotation, glm::vec3 scale, const char* name = nullptr);
	void GetGameObjects(std::list<GameObject*>& gos) const;

public:
	std::list<GameObject*> gameObjectVec;
};

#endif