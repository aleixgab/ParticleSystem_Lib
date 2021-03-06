#include <Emitter.h>
#include <Timer.h>
#include <glad/glad.h>
#include <PlaneImporter.h>
#include <string>

ParticleEmitter::ParticleEmitter(float* emitterPos, int maxParticles) : globalObjPos(emitterPos[0])
{
	colorMap.insert(std::pair<float, PartVec4>(0.0f, PartVec4(1.0f)));
	plane = new PlaneImporter();
	rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());

	if (maxParticles <= 0)
		maxParticles = 1;

	particles.resize(maxParticles);
	particleLife.resize(maxParticles);
	particleActiveBool.resize(maxParticles);

	plane->SetDynamicValues(maxParticles);

	//Start Values
	particleValues.speed = PartVec2(3.0f, 3.0f);
	particleValues.size = PartVec2(1.0f, 1.0f);
	particleValues.life = PartVec2(5.0f, 5.0f);

	emitterValues.boxShapeSize = PartVec3(1.0f);
}

PARTICLELIB_API void ParticleEmitter::AddColor(float* colorRGBA, const float position)
{
	colorMap.insert(std::pair<float, PartVec4>(position, PartVec4(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3])));
}

PARTICLELIB_API bool ParticleEmitter::EditColor(float* colorRGBA, const float position)
{
	bool ret = false;
	std::map<float, PartVec4>::iterator iter = colorMap.find(position);
	if (iter != colorMap.end())
	{
		(*iter).second = PartVec4(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);
		ret = true;
	}
	return ret;
}

PARTICLELIB_API bool ParticleEmitter::GetColor(float* colorRGBA, const float position)
{
	bool ret = false;
	std::map<float, PartVec4>::iterator iter = colorMap.find(position);
	if (iter != colorMap.end())
	{
		colorRGBA[0] = (*iter).second.x;
		colorRGBA[1] = (*iter).second.y;
		colorRGBA[2] = (*iter).second.z;
		colorRGBA[3] = (*iter).second.w;
		ret = true;
	}
	return ret;
}

PARTICLELIB_API void ParticleEmitter::GetAllPositions(std::list<float>& positions)
{
	for (std::map<float, PartVec4>::iterator iter = colorMap.begin(); iter != colorMap.end(); ++iter)
	{
		positions.push_back((*iter).first);
	}
}

PARTICLELIB_API void ParticleEmitter::GetAllColors(std::list<float*>& color)
{
	for (std::map<float, PartVec4>::iterator iter = colorMap.begin(); iter != colorMap.end(); ++iter)
	{
		color.push_back(&(*iter).second.x);
	}
}

PARTICLELIB_API bool ParticleEmitter::EraseColor(const float position)
{
	bool ret = false;
	std::map<float, PartVec4>::iterator iter = colorMap.find(position);
	if (iter != colorMap.end())
	{
		colorMap.erase(iter);
		ret = true;
	}
	return ret;
}

ParticleEmitter::~ParticleEmitter()
{
	StopEmitter();
	delete plane;
}

PARTICLELIB_API void ParticleEmitter::SetParticleValues(ParticleValues values)
{
	particleValues.life = values.life;
	particleValues.speed = values.speed;
	particleValues.gravity = values.gravity;
	particleValues.acceleration = values.acceleration;
	particleValues.size = values.size;
	particleValues.sizeOverTime =values.sizeOverTime;
	particleValues.rotation = values.rotation;
	particleValues.angularAcceleration = values.angularAcceleration;
	particleValues.angularVelocity = values.angularVelocity;

	particleValues.textureID = values.textureID;
	particleValues.textureRows = values.textureRows;
	particleValues.textureColumns = values.textureColumns;
	particleValues.animationSpeed = values.animationSpeed;
	particleValues.dieOnFinishAnim = values.dieOnFinishAnim;
	particleValues.isParticleAnimated = values.isParticleAnimated;
	particleValues.useTexture = values.useTexture;

	particleValues.activeMulticolor = values.activeMulticolor;

	particleValues.particlesEmition = values.particlesEmition;
}

PARTICLELIB_API ParticleValues ParticleEmitter::GetParticleValues() const
{
	return particleValues;
}

PARTICLELIB_API void ParticleEmitter::SetEmitterValues(EmitterValues values)
{
	emitterValues.shapeEmitter = values.shapeEmitter;
	emitterValues.burstShapeEmitter = values.burstShapeEmitter;
	emitterValues.boxShapeSize = values.boxShapeSize;
	emitterValues.sphereShapeRad = values.sphereShapeRad;
	emitterValues.coneShapeHeight =	values.coneShapeHeight;
	emitterValues.coneShapeRad = values.coneShapeRad;
	emitterValues.isBurst =	values.isBurst;
	emitterValues.burstSeconds = values.burstSeconds;
	emitterValues.minBurst = values.minBurst;
	emitterValues.maxBurst = values.maxBurst;
}

PARTICLELIB_API EmitterValues ParticleEmitter::GetEmitterValues() const
{
	return emitterValues;
}

void ParticleEmitter::Update(float dt)
{
	if (runningTime == TimerState::StatePlayed)
	{
		if (particleValues.particlesEmition > 0)
		{
			float time = emitterTimer.GetTime() / 1000.0f;
			if (time > secParticleCreation)
			{
				int particlesToCreate = (time / (1.0f / particleValues.particlesEmition));
				CreateParticles(particlesToCreate, emitterValues.shapeEmitter);

				secParticleCreation = (1.0f / particleValues.particlesEmition);

				emitterTimer.Play();
			}
		}
		if (emitterValues.isBurst)
		{
			float time = burstTimer.GetTime() / 1000.0f;
			if (time > emitterValues.burstSeconds && !onceBurst)
			{
				CreateParticles(GetRandomNum(emitterValues.minBurst, emitterValues.maxBurst), emitterValues.burstShapeEmitter);

				if (emitterValues.burstSeconds <= 0)
				{
					onceBurst = true;
					burstTimer.Stop();
				}
				else
					burstTimer.Play();
			}
		}

		float initialActive = particleActive;
		for (int i = 0; i < initialActive; ++i)
		{
			if (particleActiveBool[i])
			{
				if (particleLife[i].x < particleLife[i].y)
					particleLife[i].x += dt;
				else
				{
					//Deactivate the particle
					particleActive--;
					particles[i] = particles[particleActive];
					particleLife[i] = particleLife[particleActive];

					particleActiveBool[particleActive] = false;

					changeBuffers = true;
				}
			}
		}

		if (changeBuffers)
			SetNewBuffers();
	}
}

PARTICLELIB_API void ParticleEmitter::ChangeMaxParticles(int maxParticles)
{
	if (maxParticles <= 0)
		maxParticles = 1;

	particles.resize(maxParticles);
	particleLife.resize(maxParticles);
	particleActiveBool.resize(maxParticles);

	plane->SetDynamicValues(maxParticles);
}

PARTICLELIB_API void ParticleEmitter::SetGlobalPos(float* globalPos)
{
	globalObjPos.x = globalPos[0];
	globalObjPos.y = globalPos[1];
	globalObjPos.z = globalPos[2];
}

void ParticleEmitter::SetNewBuffers()
{
	//Life
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO_dynamic[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec2), &particleLife[0]);

	////Position
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO_dynamic[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(Particle), &particles[0]);
}

void ParticleEmitter::CreateParticles(int numParticles, ShapeEmitter emitter)
{
	for (int i = 0; i < numParticles; ++i)
	{
		//int particleId = 0;
		if (particleActive < particles.size())
		{
			//Create the particle in the correctly slot in the pool
			SetParticleValues(GetRandomPos(emitter));
			particleActive++;
		}
		else
			break;
	}
	changeBuffers = true;
}

void ParticleEmitter::SetParticleValues(PartVec3 pos)
{
	//Save all the initial values 
	particleLife[particleActive] = PartVec2(0.0f, GetRandomNum(particleValues.life.x, particleValues.life.y));

	particles[particleActive].position = pos;
	particles[particleActive].direction	= initialParticleDirection;
	particles[particleActive].gravity = particleValues.gravity;
	particles[particleActive].speed	= GetRandomNum(particleValues.speed.x, particleValues.speed.y);
	particles[particleActive].acceleration = GetRandomNum(particleValues.acceleration.x, particleValues.acceleration.y);
	particles[particleActive].angleRot = GetRandomNum(particleValues.rotation.x, particleValues.rotation.y);
	particles[particleActive].angleVel = GetRandomNum(particleValues.angularVelocity.x, particleValues.angularVelocity.y);
	particles[particleActive].angleAccel = GetRandomNum(particleValues.angularAcceleration.x, particleValues.angularAcceleration.y);
	particles[particleActive].size = GetRandomNum(particleValues.size.x, particleValues.size.y);
	particles[particleActive].sizeTime = GetRandomNum(particleValues.sizeOverTime.x, particleValues.sizeOverTime.y);

	particleActiveBool[particleActive] = true;
}

void ParticleEmitter::Draw(unsigned int shaderUuid)
{
	if (runningTime == TimerState::StateStopped)
		return;

	glUniform1i(glGetUniformLocation(shaderUuid, "colorSize"), particleValues.activeMulticolor ? colorMap.size() : 1);
	glUniform1i(glGetUniformLocation(shaderUuid, "useTexture"), particleValues.useTexture);

	glUniform1i(glGetUniformLocation(shaderUuid, "isAnimated"), particleValues.isParticleAnimated);
	glUniform1i(glGetUniformLocation(shaderUuid, "textRows"), particleValues.textureRows);
	glUniform1i(glGetUniformLocation(shaderUuid, "textColumns"), particleValues.textureColumns);
	glUniform1f(glGetUniformLocation(shaderUuid, "animatedTime"), particleValues.animationSpeed);

	int cont = 0;
	for (std::map<float, PartVec4>::iterator iter = colorMap.begin(); iter != colorMap.end(); ++iter, ++cont)
	{
		std::string name = "colors[" + std::to_string(cont) + "]";
		glUniform4fv(glGetUniformLocation(shaderUuid, name.c_str()), 4, &(*iter).second.x);

		name = "positions[" + std::to_string(cont) + "]";
		glUniform1fv(glGetUniformLocation(shaderUuid, name.c_str()), 1, &(*iter).first);

		if (!particleValues.activeMulticolor)
			break;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particleValues.textureID);

	//Bind VAO
	glBindVertexArray(plane->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO_dynamic[0]);
	//Life
	glVertexAttribDivisor(0, 1);
	glEnableVertexAttribArray(0);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec2), &particleLife[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO_dynamic[1]);
	//Position
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);

	//Direction
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)sizeof(PartVec3));

	//Speed
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 2));

	//Acceleration
	glVertexAttribDivisor(4, 1);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 2 + sizeof(float)));

	//Gravity - Acceleration 3D
	glVertexAttribDivisor(5, 1);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 2 + sizeof(float) * 2));

	//Rotation
	glVertexAttribDivisor(6, 1);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 3 + sizeof(float) * 2));

	//Rotation Velocity
	glVertexAttribDivisor(7, 1);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 3 + sizeof(float) * 3));

	//Rotation Acceleration
	glVertexAttribDivisor(8, 1);
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 3 + sizeof(float) * 4));

	//Size
	glVertexAttribDivisor(9, 1);
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 3 + sizeof(float) * 5));

	//Size Over Time
	glVertexAttribDivisor(10, 1);
	glEnableVertexAttribArray(10);
	glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(PartVec3) * 3 + sizeof(float) * 6));

	glDrawArraysInstanced(GL_POINTS, 0, 1, particleActive);

	glBindVertexArray(0);
}

bool ParticleEmitter::SaveCameraDistance(float* cameraPos)
{
	bool ret = false;
	if (cameraPos)
	{
		float x = cameraPos[0] - globalObjPos.x;
		float y = cameraPos[1] - globalObjPos.y;
		float z = cameraPos[2] - globalObjPos.z;

		cameraDist = x * x + y * y + z * z;
		ret = true;
	}
	return ret;
}

void ParticleEmitter::StartEmitter()
{
	runningTime = TimerState::StatePlayed;
	onceBurst = false;
	emitterTimer.Play();
	burstTimer.Play();
}

void ParticleEmitter::StopEmitter()
{
	for (int i = 0; i < particleActive; ++i)
	{
		if (particleActiveBool[i])
		{
			particleActiveBool[i] = false;
		}
	}

	runningTime = TimerState::StateStopped;
	secParticleCreation = 0.0f;

	particleActive = 0;
	emitterTimer.Stop();
	burstTimer.Stop();
}

void ParticleEmitter::PauseEmitter()
{
	runningTime = TimerState::StatePaused;
}

PartVec3 ParticleEmitter::GetRandomPos(ShapeEmitter emitter)
{
	PartVec3 randomPos = PartVec3(0.0f);

	switch (emitter)
	{
	case BoxShape:
		//Box Size
		randomPos.x =GetRandomNum(-emitterValues.boxShapeSize.x / 2, emitterValues.boxShapeSize.x / 2);
		randomPos.y = GetRandomNum(-emitterValues.boxShapeSize.y / 2, emitterValues.boxShapeSize.y / 2);
		randomPos.z = GetRandomNum(-emitterValues.boxShapeSize.z / 2, emitterValues.boxShapeSize.z / 2);

		initialParticleDirection = PartVec3(0.0f, 1.0f, 0.0f);
		break;
	case SphereShape:
	case SphereShapeCenter:
	case SphereShapeBorder:
		//Sphere rad
		randomPos.x = GetRandomNum(-1.0f, 1.0f);
		randomPos.y = GetRandomNum(-1.0f, 1.0f);
		randomPos.z = GetRandomNum(-1.0f, 1.0f);

		randomPos = randomPos.Normalize();
		initialParticleDirection = randomPos;

		if (emitter == SphereShape)
			randomPos *= GetRandomNum(0.0f, emitterValues.sphereShapeRad);
		else if (emitter == SphereShapeCenter)
			randomPos = PartVec3(0.0f);
		else if (emitter == SphereShapeBorder)
			randomPos = initialParticleDirection * emitterValues.sphereShapeRad;
		break;
	case ConeShape:
		//The position is always 0. We only change the direction
	{
		PartVec3 destination;
		destination.x = GetRandomNum(-emitterValues.coneShapeRad, emitterValues.coneShapeRad);
		destination.y = emitterValues.coneShapeHeight;
		destination.z = GetRandomNum(-emitterValues.coneShapeRad, emitterValues.coneShapeRad);

		initialParticleDirection = destination.Normalize();

	}
	break;
	default:
		break;
	}


	return randomPos + globalObjPos;
}

float ParticleEmitter::GetRandomNum(float min, float max)
{
	if (min < max)
		return (max - min) * (float)rng() / (float)rng.max() + min;
	else
		return min;
}