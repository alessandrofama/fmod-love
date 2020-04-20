extern "C" {
#include "lua/inc/lua.h"
#include "lua/inc/lauxlib.h"
}

#include "fmod/inc/fmod_studio.hpp"
#include "fmod/inc/fmod.hpp"
#include <unordered_map>

struct Vector3 {
	float x;
	float y;
	float z;
};

FMOD::Studio::System* studioSystem = nullptr;
FMOD::System* coreSystem = nullptr;

std::unordered_map<std::size_t, FMOD::Studio::EventInstance*> instanceList;
std::size_t nInstances = 0;

std::unordered_map<std::size_t, FMOD::Studio::Bank*> bankList;
std::size_t nBanks = 0;

std::unordered_map<std::size_t, FMOD::Studio::Bus*> busList;
std::size_t nBusses = 0;

std::unordered_map<std::size_t, FMOD::Studio::VCA*> vcaList;
std::size_t nVCAs = 0;

static int LuaIntDefault(lua_State* L, int i, int def);

FMOD_3D_ATTRIBUTES To3DAttributes(const Vector3& vec, const Vector3& forward,
	const Vector3& up);

FMOD_VECTOR ToFMODVector(const Vector3& position);

static int LuaIntDefault(lua_State* L, int i, int def)
{
	return (lua_gettop(L) >= i && !lua_isnil(L, i)) ? luaL_checkint(L, i) : def;
}

FMOD_3D_ATTRIBUTES To3DAttributes(const Vector3& vec, const Vector3& forward,
	const Vector3& up)
{
	FMOD_3D_ATTRIBUTES attributes;
	attributes.forward = ToFMODVector(forward);
	attributes.up = ToFMODVector(up);
	attributes.position = ToFMODVector(vec);

	return attributes;
}

FMOD_VECTOR ToFMODVector(const Vector3& position)
{
	FMOD_VECTOR fVec;
	fVec.x = position.x;
	fVec.y = position.y;
	fVec.z = position.z;
	return fVec;
}

bool FMODInit(int outputType, int realChannels, int virtualChannels,
	int studioInitFlags)
{
	auto result = FMOD::Studio::System::create(&studioSystem);

	if (result != FMOD_OK) {
		return 0;
	}

	result = studioSystem->getCoreSystem(&coreSystem);

	if (result != FMOD_OK) {
		return 0;
	}

	result = coreSystem->setOutput((FMOD_OUTPUTTYPE)outputType);

	if (result != FMOD_OK) {
		return 0;
	}

	result = coreSystem->setSoftwareChannels(realChannels);

	if (result != FMOD_OK) {
		return 0;
	}

	result = studioSystem->initialize(virtualChannels,
		(FMOD_STUDIO_INITFLAGS)studioInitFlags,
		FMOD_INIT_NORMAL, NULL);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool Update()
{
	auto result = studioSystem->update();

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

int LoadBank(const char* bankPath, int flags)
{
	FMOD::Studio::Bank* bank;
	auto result = studioSystem->loadBankFile(
		bankPath, (FMOD_STUDIO_LOAD_BANK_FLAGS)flags, &bank);

	if (result != FMOD_OK) {
		return -1;
	}

	if (bank) {
		bankList.emplace(nBanks++, bank);
		return static_cast<int>(nBanks - 1);
	}
	else
		return -1;
}

bool UnloadBank(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (bankList.count(i) == 0) {
		return 0;
	}

	auto result = bankList[i]->unload();

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool SetNumListeners(int listeners)
{
	auto result = studioSystem->setNumListeners(listeners);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool SetListener3DPosition(int listener, float posX, float posY, float posZ,
	float dirX, float dirY, float dirZ, float oX,
	float oY, float oZ)
{
	Vector3 pos = { posX, posY, posZ };
	Vector3 forward = { dirX, dirY, dirZ };
	Vector3 up = { oX, oY, oZ };

	FMOD_3D_ATTRIBUTES attributes = To3DAttributes(pos, forward, up);

	auto result = studioSystem->setListenerAttributes(listener, &attributes);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

int CreateInstance(const char* eventPath)
{
	FMOD::Studio::EventDescription* eventDescription = nullptr;
	auto result = studioSystem->getEvent(eventPath, &eventDescription);

	if (result != FMOD_OK) {
		return -1;
	}

	if (eventDescription) {
		FMOD::Studio::EventInstance* eventInstance = nullptr;
		auto result = eventDescription->createInstance(&eventInstance);
		if (result == FMOD_OK) {
			instanceList.emplace(nInstances++, eventInstance);
			return static_cast<int>(nInstances - 1);
		}
		else
			return -1;
	}
	else
		return -1;
}

bool StartInstance(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}
	auto result = instanceList[i]->start();

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool StopInstance(int index, int stopMode)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 1) {
		auto result = instanceList[i]->stop((FMOD_STUDIO_STOP_MODE)(stopMode));

		if (result != FMOD_OK) {
			return 0;
		}
	}

	return 1;
}

bool ReleaseInstance(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 1) {
		auto result = instanceList[i]->release();

		if (result != FMOD_OK) {
			return 0;
		}

		instanceList.erase(i);
	}

	return 1;
}

bool Set3DAttributes(int index, float posX, float posY, float posZ, float dirX,
	float dirY, float dirZ, float oX, float oY, float oZ)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	Vector3 pos = { posX, posY, posZ };
	Vector3 forward = { dirX, dirY, dirZ };
	Vector3 up = { oX, oY, oZ };

	FMOD_3D_ATTRIBUTES attributes;
	attributes = To3DAttributes(pos, forward, up);

	auto result = instanceList[i]->set3DAttributes(&attributes);
	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool PlayOneShot2D(const char* eventPath)
{
	FMOD::Studio::EventDescription* eventDescription = NULL;
	auto result = studioSystem->getEvent(eventPath, &eventDescription);

	if (result != FMOD_OK) {
		return 0;
	}

	if (eventDescription) {
		FMOD::Studio::EventInstance* eventInstance = NULL;
		auto result = eventDescription->createInstance(&eventInstance);

		if (result == FMOD_OK && eventInstance) {
			eventInstance->start();
			eventInstance->release();
			return 1;
		}
		else
			return 0;
	}
	else
		return 0;
}

bool PlayOneShot3D(const char* eventPath, float posX, float posY, float posZ,
	float dirX, float dirY, float dirZ, float oX, float oY,
	float oZ)
{
	FMOD::Studio::EventDescription* eventDescription = NULL;
	auto result = studioSystem->getEvent(eventPath, &eventDescription);

	if (result != FMOD_OK) {
		return 0;
	}

	if (eventDescription) {
		bool is3D;
		eventDescription->is3D(&is3D);

		if (!is3D) {
			return 0;
		}

		FMOD::Studio::EventInstance* eventInstance = NULL;
		result = eventDescription->createInstance(&eventInstance);

		if (result != FMOD_OK) {
			return 0;
		}

		Vector3 pos = { posX, posY, posZ };
		Vector3 forward = { dirX, dirY, dirZ };
		Vector3 up = { oX, oY, oZ };
		FMOD_3D_ATTRIBUTES attributes = To3DAttributes(pos, forward, up);

		eventInstance->set3DAttributes(&attributes);
		eventInstance->start();
		eventInstance->release();

		return 1;
	}
	else
		return 0;
}

bool SetInstanceVolume(int index, float volume)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	auto result = instanceList[i]->setVolume(volume);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool IsPlaying(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	FMOD_STUDIO_PLAYBACK_STATE pS = FMOD_STUDIO_PLAYBACK_STOPPED;

	auto result = instanceList[i]->getPlaybackState(&pS);

	if (result != FMOD_OK) {
		return 0;
	}

	if (pS == FMOD_STUDIO_PLAYBACK_PLAYING) {
		return 1;
	}
	else
		return 0;
}

bool SetInstancePaused(int index, bool pause)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	auto result = instanceList[i]->setPaused(pause);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

bool SetInstancePitch(int index, float pitch)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	auto result = instanceList[i]->setPitch(pitch);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

float GetInstancePitch(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return -1;
	}

	float pitch = 0;
	float finalPitch = 0;

	auto result = instanceList[i]->getPitch(&pitch, &finalPitch);

	if (result != FMOD_OK) {
		return -1;
	}

	return finalPitch;
}

int GetTimelinePosition(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return -1;
	}

	int position = 0;

	auto result = instanceList[i]->getTimelinePosition(&position);

	if (result != FMOD_OK) {
		return -1;
	}

	return position;
}

bool SetTimelinePosition(int index, int position)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	auto result = instanceList[i]->setTimelinePosition(position);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

float GetInstanceRMS(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return -1;
	}

	FMOD::ChannelGroup* ChanGroup = nullptr;
	instanceList[i]->getChannelGroup(&ChanGroup);
	if (ChanGroup) {
		FMOD::DSP* ChanDSP = nullptr;
		ChanGroup->getDSP(0, &ChanDSP);
		if (ChanDSP) {
			ChanDSP->setMeteringEnabled(false, true);
			FMOD_DSP_METERING_INFO Info = {};
			ChanDSP->getMeteringInfo(nullptr, &Info);

			float rms = 0.f;

			for (int i = 0; i < Info.numchannels; i++) {
				rms += Info.rmslevel[i] * Info.rmslevel[i];
			}

			rms = std::sqrt(rms / (float)Info.numchannels);

			float dB = rms > 0 ? 20.0f * std::log10(rms * std::sqrt(2.0f)) : -80.0f;
			if (dB > 10.0f)
				dB = 10.0f;
			return dB;
		}
		else
			return -1;
	}
	else
		return -1;
}

float GetGlobalParameterByName(const char* parameterName)
{
	float value, finalValue;
	auto result = studioSystem->getParameterByName(parameterName, &value, &finalValue);

	if (result != FMOD_OK) {
		return -1;
	}

	return finalValue;
}

bool SetGlobalParameterByName(const char* parameterName, float value,
	bool ignoreSeekSpeed)
{
	auto result = studioSystem->setParameterByName(parameterName, value, ignoreSeekSpeed);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

float GetParameterByName(int index, const char* parameterName)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return -1;
	}

	float value, finalValue;

	auto result = instanceList[i]->getParameterByName(parameterName, &value, &finalValue);

	if (result != FMOD_OK) {
		return -1;
	}

	return finalValue;
}

bool SetParameterByName(int index, const char* parameterName, float value,
	bool ignoreSeekSpeed)
{
	std::size_t i = (std::size_t)round(index);

	if (instanceList.count(i) == 0) {
		return 0;
	}

	auto result = instanceList[i]->setParameterByName(parameterName, value,
		ignoreSeekSpeed);

	if (result != FMOD_OK) {
		return 0;
	}

	return 1;
}

int GetBus(const char* busPath)
{
	FMOD::Studio::Bus* bus = nullptr;
	auto result = studioSystem->getBus(busPath, &bus);
	if (result == FMOD_OK) {
		busList.emplace(nBusses++, bus);
		return static_cast<int>(nBusses - 1);
	}
	else
		return -1;
}

float GetBusVolume(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (busList.count(i) == 0) {
		return -1;
	}

	float volume, finalVolume;

	auto result = busList[i]->getVolume(&volume, &finalVolume);

	if (result != FMOD_OK) {
		return -1;
	}
	else
		return finalVolume;
}

bool SetBusVolume(int index, float volume)
{
	std::size_t i = (std::size_t)round(index);

	if (busList.count(i) == 0) {
		return 0;
	}

	auto result = busList[i]->setVolume(volume);

	if (result != FMOD_OK) {
		return 0;
	}
	else
		return 1;
}

int GetVCA(const char* vcaPath)
{
	FMOD::Studio::VCA* vca = nullptr;
	auto result = studioSystem->getVCA(vcaPath, &vca);
	if (result == FMOD_OK) {
		vcaList.emplace(nVCAs++, vca);
		return static_cast<int>(nVCAs - 1);
	}
	else
		return -1;
}

float GetVCAVolume(int index)
{
	std::size_t i = (std::size_t)round(index);

	if (vcaList.count(i) == 0) {
		return -1;
	}

	float volume, finalVolume;

	auto result = vcaList[i]->getVolume(&volume, &finalVolume);

	if (result != FMOD_OK) {
		return -1;
	}
	else
		return finalVolume;
}

bool SetVCAVolume(int index, float volume)
{
	std::size_t i = (std::size_t)round(index);

	if (vcaList.count(i) == 0) {
		return 0;
	}

	auto result = vcaList[i]->setVolume(volume);

	if (result != FMOD_OK) {
		return 0;
	}
	else
		return 1;
}

static int love_fmod_init(lua_State* L)
{
	lua_pushboolean(L,
		FMODInit(LuaIntDefault(L, 1, 0), LuaIntDefault(L, 2, 32),
			LuaIntDefault(L, 3, 128), LuaIntDefault(L, 4, 1)));
	return 1;
}

static int love_fmod_update(lua_State* L)
{
	bool result = Update();
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_load_bank(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	int flags = static_cast<int>(lua_tointeger(L, 2));
	int index = LoadBank(input, flags);
	lua_Integer i = static_cast<lua_Integer>(index);
	lua_pushinteger(L, i);
	return 1;
}

static int love_fmod_unload_bank(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	bool result = UnloadBank(index);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_setnumlisteners(lua_State* L)
{
	int listeners = static_cast<int>(lua_tointeger(L, 1));
	bool result = SetNumListeners(listeners);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_set_listener3d_position(lua_State* L)
{
	int listener = static_cast<int>(lua_tointeger(L, 1));
	float posX = static_cast<float>(lua_tonumber(L, 2));
	float posY = static_cast<float>(lua_tonumber(L, 3));
	float posZ = static_cast<float>(lua_tonumber(L, 4));
	float dirX = static_cast<float>(lua_tonumber(L, 5));
	float dirY = static_cast<float>(lua_tonumber(L, 6));
	float dirZ = static_cast<float>(lua_tonumber(L, 7));
	float oX = static_cast<float>(lua_tonumber(L, 8));
	float oY = static_cast<float>(lua_tonumber(L, 9));
	float oZ = static_cast<float>(lua_tonumber(L, 10));

	bool result = SetListener3DPosition(listener, posX, posY, posZ, dirX, dirY,
		dirZ, oX, oY, oZ);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_create_instance(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	int index = CreateInstance(input);
	lua_pushinteger(L, index);
	return 1;
}

static int love_fmod_start_instance(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	bool result = StartInstance(index);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_stop_instance(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	int stopMode = static_cast<int>(lua_tointeger(L, 2));
	bool result = StopInstance(index, stopMode);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_release_instance(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	bool result = ReleaseInstance(index);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_set3d_attributes(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float posX = static_cast<float>(lua_tonumber(L, 2));
	float posY = static_cast<float>(lua_tonumber(L, 3));
	float posZ = static_cast<float>(lua_tonumber(L, 4));
	float dirX = static_cast<float>(lua_tonumber(L, 5));
	float dirY = static_cast<float>(lua_tonumber(L, 6));
	float dirZ = static_cast<float>(lua_tonumber(L, 7));
	float oX = static_cast<float>(lua_tonumber(L, 8));
	float oY = static_cast<float>(lua_tonumber(L, 9));
	float oZ = static_cast<float>(lua_tonumber(L, 10));

	bool result = Set3DAttributes(index, posX, posY, posZ, dirX, dirY, dirZ, oX, oY, oZ);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_playoneshot2d(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	bool result = PlayOneShot2D(input);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_playoneshot3d(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	float posX = static_cast<float>(lua_tonumber(L, 2));
	float posY = static_cast<float>(lua_tonumber(L, 3));
	float posZ = static_cast<float>(lua_tonumber(L, 4));
	float dirX = static_cast<float>(lua_tonumber(L, 5));
	float dirY = static_cast<float>(lua_tonumber(L, 6));
	float dirZ = static_cast<float>(lua_tonumber(L, 7));
	float oX = static_cast<float>(lua_tonumber(L, 8));
	float oY = static_cast<float>(lua_tonumber(L, 9));
	float oZ = static_cast<float>(lua_tonumber(L, 10));

	bool result = PlayOneShot3D(input, posX, posY, posZ, dirX, dirY, dirZ, oX, oY, oZ);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_set_instance_volume(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float volume = static_cast<float>(lua_tonumber(L, 2));
	bool result = SetInstanceVolume(index, volume);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_is_playing(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	bool result = IsPlaying(index);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_set_instance_paused(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	bool paused = lua_toboolean(L, 2);
	bool result = SetInstancePaused(index, paused);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_get_instance_pitch(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float pitch = GetInstancePitch(index);
	lua_Number result = static_cast<lua_Number>(pitch);
	lua_pushnumber(L, result);
	return 1;
}

static int love_fmod_set_instance_pitch(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float pitch = static_cast<float>(lua_tonumber(L, 2));
	bool result = SetInstancePitch(index, pitch);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_get_timeline_position(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	int position = GetTimelinePosition(index);
	lua_pushinteger(L, position);
	return 1;
}

static int love_fmod_set_timeline_position(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	int position = static_cast<int>(lua_tointeger(L, 2));
	bool result = SetTimelinePosition(index, position);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_get_instance_rms(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float rms = GetInstanceRMS(index);
	lua_Number result = static_cast<lua_Number>(rms);
	lua_pushnumber(L, result);
	return 1;
}

static int love_fmod_get_global_parameter_by_name(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	float parameterValue = GetGlobalParameterByName(input);
	lua_Number result = static_cast<lua_Number>(parameterValue);
	lua_pushnumber(L, result);
	return 1;
}

static int love_fmod_set_global_parameter_by_name(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	float parameterValue = static_cast<float>(lua_tonumber(L, 2));
	bool ignoreSeekSpeed = lua_toboolean(L, 3);
	bool result = SetGlobalParameterByName(input, parameterValue, ignoreSeekSpeed);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_get_parameter_by_name(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	const char* input = lua_tostring(L, 2);
	float parameterValue = GetParameterByName(index, input);
	lua_Number result = static_cast<lua_Number>(parameterValue);
	lua_pushnumber(L, result);
	return 1;
}

static int love_fmod_set_parameter_by_name(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	const char* input = lua_tostring(L, 2);
	float parameterValue = static_cast<float>(lua_tonumber(L, 3));
	bool ignoreSeekSpeed = lua_toboolean(L, 4);
	bool result = SetParameterByName(index, input, parameterValue, ignoreSeekSpeed);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_get_bus(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	int result = GetBus(input);
	lua_pushinteger(L, result);
	return 1;
}

static int love_fmod_get_bus_volume(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float volume = GetBusVolume(index);
	lua_Number result = static_cast<lua_Number>(volume);
	lua_pushnumber(L, result);
	return 1;
}

static int love_fmod_set_bus_volume(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float volume = static_cast<float>(lua_tonumber(L, 2));
	bool result = SetBusVolume(index, volume);
	lua_pushboolean(L, result);
	return 1;
}

static int love_fmod_get_vca(lua_State* L)
{
	const char* input = lua_tostring(L, 1);
	int result = GetVCA(input);
	lua_pushinteger(L, result);
	return 1;
}

static int love_fmod_get_vca_volume(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float volume = GetVCAVolume(index);
	lua_Number result = static_cast<lua_Number>(volume);
	lua_pushnumber(L, result);
	return 1;
}

static int love_fmod_set_vca_volume(lua_State* L)
{
	int index = static_cast<int>(lua_tointeger(L, 1));
	float volume = static_cast<float>(lua_tonumber(L, 2));
	bool result = SetVCAVolume(index, volume);
	lua_pushboolean(L, result);
	return 1;
}

static const struct luaL_reg love_fmod_methods[] = {
	{ "init", love_fmod_init },
	{ "update", love_fmod_update },
	{ "loadBank", love_fmod_load_bank },
	{ "unloadBank", love_fmod_unload_bank },
	{ "setNumListeners", love_fmod_setnumlisteners },
	{ "setListener3DPosition", love_fmod_set_listener3d_position },
	{ "createInstance", love_fmod_create_instance },
	{ "startInstance", love_fmod_start_instance },
	{ "stopInstance", love_fmod_stop_instance },
	{ "releaseInstance", love_fmod_release_instance },
	{ "set3DAttributes", love_fmod_set3d_attributes },
	{ "playOneShot2D", love_fmod_playoneshot2d },
	{ "playOneShot3D", love_fmod_playoneshot3d },
	{ "setInstanceVolume", love_fmod_set_instance_volume },
	{ "isPlaying", love_fmod_is_playing },
	{ "setInstancePaused", love_fmod_set_instance_paused },
	{ "getInstancePitch", love_fmod_get_instance_pitch },
	{ "setInstancePitch", love_fmod_set_instance_pitch },
	{ "getTimelinePosition", love_fmod_get_timeline_position },
	{ "setTimelinePosition", love_fmod_set_timeline_position },
	{ "getInstanceRms", love_fmod_get_instance_rms },
	{ "getGlobalParameterByName", love_fmod_get_global_parameter_by_name },
	{ "setGlobalParameterByName", love_fmod_set_global_parameter_by_name },
	{ "getParameterByName", love_fmod_get_parameter_by_name },
	{ "setParameterByName", love_fmod_set_parameter_by_name },
	{ "getBus", love_fmod_get_bus },
	{ "getBusVolume", love_fmod_get_bus_volume },
	{ "setBusVolume", love_fmod_set_bus_volume },
	{ "getVCA", love_fmod_get_vca },
	{ "getVCAVolume", love_fmod_get_vca_volume },
	{ "setVCAVolume", love_fmod_set_vca_volume },
	{ NULL, NULL }
};

extern "C" {
	__declspec(dllexport) int luaopen_fmod_love(lua_State* L)
	{
		luaL_openlib(L, "fmod_love", love_fmod_methods, 0);
		return 1;
	}
}