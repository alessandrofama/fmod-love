extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <unordered_map>

struct Vector3 {

	union {
		struct {
			float x;
			float y;
			float z;
		};
	};

	inline Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

void To3DAttributes(Vector3 position, Vector3 forward, Vector3 up, FMOD_3D_ATTRIBUTES& outAttributes);

void ToFMODVector(Vector3 inVector, FMOD_VECTOR& outVector);

bool Init(const unsigned int& outputType, const unsigned int& realChannels, const unsigned int& virtualChannels,
	const unsigned int& studioInitFlags);

bool Update();

int LoadBank(const char* bankPath, int flags);

bool UnloadBank(const unsigned int& index);

bool SetNumListeners(const unsigned int& listeners);

bool SetListener3DPosition(const unsigned int& listener, float posX, float posY, float posZ, float dirX, float dirY, float dirZ, float oX, float oY, float oZ);

int CreateInstance(const char* eventPath);

bool StartInstance(const unsigned int& index);

bool StopInstance(const unsigned int& index, int stopMode);

bool ReleaseInstance(const unsigned int& index);

bool Set3DAttributes(const unsigned int& index, float posX, float posY, float posZ, float dirX, float dirY, float dirZ, float oX, float oY, float oZ);

bool PlayOneShot2D(const char* eventPath);

bool PlayOneShot3D(const char* eventPath, float posX, float posY, float posZ, float dirX, float dirY, float dirZ, float oX, float oY, float oZ);

bool SetInstanceVolume(const unsigned int& index, float volume);

bool IsPlaying(const unsigned int& index);

bool SetInstancePaused(const unsigned int& index, bool pause);

bool SetInstancePitch(const unsigned int& index, float pitch);

float GetInstancePitch(const unsigned int& index);

int GetTimelinePosition(const unsigned int& index);

bool SetTimelinePosition(const unsigned int& index, const unsigned int& position);

float GetInstanceRMS(const unsigned int& index);

float GetGlobalParameterByName(const char* parameterName);

bool SetGlobalParameterByName(const char* parameterName, float value, bool ignoreSeekSpeed);

float GetParameterByName(const unsigned int& index, const char* parameterName);

bool SetParameterByName(const unsigned int& index, const char* parameterName, float value, bool ignoreSeekSpeed);

int GetBus(const char* busPath);

float GetBusVolume(const unsigned int& index);

