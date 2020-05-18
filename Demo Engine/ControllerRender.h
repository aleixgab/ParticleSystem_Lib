#ifndef __CONTROLLER_RENDER_H__
#define __CONTROLLER_RENDER_H__

#include "Controller.h"
#include "Shader.h"
#include <list>

class TextureImporter;
class PlaneImporter;
class GameObject;

class ControllerRender : public Controller
{
public:

	ControllerRender(GameManager* mng);
	~ControllerRender();

	bool Start();
	bool Update(float dt);
	bool CleanUp();

	std::list<TextureImporter*> textures;
private:
	Shader basicShader;
	Shader particleShader;

	PlaneImporter* ground;
};

#endif