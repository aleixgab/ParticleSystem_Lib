#ifndef __COMPONENT_H__
#define __COMPONENT_H__

class GameObject;

enum ComponentType
{
	ComponentType_NONE,

	ComponentType_TRANSFORM,
	ComponentType_EMITTER,
};

class Component
{
public:
	Component(GameObject* gameObject, ComponentType type) : owner(gameObject) {}

	virtual ~Component() {}

	virtual void Update() {}

	inline void SetActive(bool isActive) { this->isActive = isActive; }

	virtual void Inspector() {}

public:
	ComponentType type = ComponentType_NONE;

	bool isActive = true;

	GameObject* owner = nullptr;

	bool toDelete = false;
};
#endif