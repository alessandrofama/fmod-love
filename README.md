# fmod-love
FMOD Studio API module for LÖVE

This module adds support for the FMOD Studio API 2.01.00 (x64) in LÖVE (x64). Works with Windows for now. Pull requests are welcome.

## Contents 

[Installation](#installation)

[Building](#bulding)

[Usage](#installation)

[Examples](#examples)

## Installation

Download the <b>fmod_love.dll</b> file and add it to the directory containing the main.lua file. Download the FMOD Studio API (2.01.00 / x64) and also add <b>fmodstudio.dll</b> and <b>fmod.dll</b> to your game folder.

## Building
Add 
- fmod.dll
- fmod_vc.lib
- fmodstudio.dll
- fmodstudio_vc.lib 
from the API installation to the fmod lib folder and it should build.

## Usage

### System functions
```
fmod = require("fmod_love")
```
to load the .dll file.

```
fmod.init(outputType, realChannels, virtualChannels,
	studioInitFlags)
```
will create and initialize the Studio System. <a href="https://fmod.com/resources/documentation-api?version=2.0&page=core-api-system.html#fmod_outputtype">`outputType`</a> expects an integer, 0 should be fine in most cases. Change Real and Virtual Channels to your liking. Setting studioInitFlags to 1 will enable live update (check <a href="https://www.fmod.com/resources/documentation-api?version=2.1&page=studio-api-system.html#fmod_studio_initflags">docs</a>)

Returns `false` if failed, `true` if succeded.

```
fmod.update()
```
To update the Studio System (call it in love.update).
Returns `false` if failed, `true` if succeded.

### Listener 

#### Setting num. of listeners
```
fmod.setNumListeners(numOfListeners)
```
Returns `false` if failed, `true` if succeded.

#### Setting listener position
```
fmod.setListener3DPosition(listenerIndex, posX, posY, posZ, dirX, dirY, dirZ, oX, oY, oZ)
```
If you have one listener the `listenerIndex` is 0. `dirX` `dirY` `dirZ` is the forward vector, `oX` `oY` `oZ` the up vector.
Returns `false` if failed, `true` if succeded.

### Banks

#### Loading a bank
```
fmod.loadBank(bankPath, flags)
```
`bankPath` is the path to the bank file. `flags` (int) can be:

- FMOD_STUDIO_LOAD_BANK_NORMAL (0)
- FMOD_STUDIO_LOAD_BANK_NONBLOCKING (1)
- FMOD_STUDIO_LOAD_BANK_DECOMPRESS_SAMPLES (2)
- FMOD_STUDIO_LOAD_BANK_UNENCRYPTED (4)

Will return an index value to the bank.
Returns `-1` if failed.
Use the returned value to unload the bank if necessary:

```
fmod.unloadBank(index)
```
Returns `false` if failed, `true` if succeded.

### EventInstances

#### Create an instance
```
fmod.createInstance(eventPath)
```
Returns an index value to the EventInstance. 
Returns `-1` if failed. Use the index value to start the instance:

#### Starting an instance
```
fmod.startIntance(index)
```
Returns `false` if failed, `true` if succeded.

#### Stopping an instance
```
fmod.stopInstance(index, stopMode)
```
`stopMode`  can be:

- FMOD_STUDIO_STOP_ALLOWFADEOUT (0)
- FMOD_STUDIO_STOP_IMMEDIATE (1)

Returns `false` if failed, `true` if succeded.

#### Releasing an instance
```
fmod.releaseInstance(index)
```
Returns `false` if failed, `true` if succeded. This will remove the instance from the index.

#### Setting the 3D Attributes on an instance
```
fmod.set3DAttributes(index, posX, posY, posZ, dirX, dirY, dirZ, oX, oY, oZ)
```

`dirX` `dirY` `dirZ` is the forward vector, `oX` `oY` `oZ` the up vector. 
Returns `false` if failed, `true` if succeded.

#### Playing a simple 2D event (no spatializer)
```
fmod.playOneShot2D(eventPath)
```
Will automatically call EventInstance::release after starting. No need to take care of the instance.
Returns `false` if failed, `true` if succeded.

#### Playing a simple 3D event 
```
fmod.playOneShot3D(eventPath, posX, posY, posZ, dirX, dirY, dirZ, oX, oY, oZ)
```
It will automatically call EventInstance::release after playing. `dirX` `dirY` `dirZ` is the forward vector, `oX` `oY` `oZ` the up vector.
Returns `false` if failed, `true` if succeded.

#### Setting the volume of an instance
```
fmod.setInstanceVolume(index, volume)
```
Returns `false` if failed, `true` if succeded.

#### Check if an instance is playing
```
fmod.isPlaying(index)
```
Returns `false` if failed or not playing, `true` if playing.

#### Set the paused state of an instance
```
fmod.setInstancePaused(index, pauseState)
```
`pauseState` should be `true` or `false`.

#### Set the pitch of an instance
```
fmod.setInstancePitch(index, pitch)
```
Returns `false` if failed, `true` if succeded.

#### Get the timeline position of an instance
```
fmod.getTimelinePosition(index)
```
Returns the timeline position (int).
Returns `-1` if failed.

#### Set the timeline position of an instance
```
fmod.setTimelinePosition(index, position)
```

Returns `false` if failed, `true` if succeded.

#### Get the RMS value of a playing instance
```
fmod.getInstanceRms(index)
```
Returns the RMS value of an instance. 
Returns `-1` if failed.

### Parameters

#### Get a global parameter value by name 
```
fmod.getGlobalParameterByName(parameterName)
```
Takes the parameter name and returns the parameter value. 
Returns `-1` if failed.

#### Set a global parameter value by name 
```
fmod.setGlobalParameterByName(parameterName, value, ignoreSeekSpeed)
```
ignoreSeekSpeed can be true or false.
Returns `false` if failed, `true` if succeded.

#### Get a local parameter value by name 
```
fmod.getParameterByName(instanceIndex, parameterName)
```
Takes the instance index and the parameter name and returns its value.
Returns `-1` if failed.

#### Set a local parameter value by name 
```
fmod.setParameterByName(instanceIndex, parameterName, value, ignoreSeekSpeed)
```
Returns `false` if failed, `true` if succeded.

### Busses

#### Getting a bus
```
fmod.getBus(busPath)
```
Takes the bus path and returns an index to that bus. Use it to get or set the bus volume.
Returns `-1` if failed.

#### Get the bus volume
```
fmod.getBusVolume(index)
```
Returns the bus volume.
Returns `-1` if failed.

#### Set the bus volume
```
fmod.setBusVolume(index, volume)
```
Returns `false` if failed, `true` if succeded.

### VCAs

#### Getting a VCA
```
fmod.getVCA(vcaPath)
```
Takes the VCA path and returns an index value to that VCA. Use it to get or set the VCA volume.
Returns `-1` if failed.

#### Get the VCA volume
```
fmod.getVCAVolume(index)
```
Returns the VCA volume.
Returns `-1` if failed.

#### Set the VCA volume
```
fmod.setVCAVolume(index, volume)
```
Returns `false` if failed, `true` if succeded.

## Examples

### Initialising the Studio System and loading banks
```
fmod = require("fmod_love")
function love.load()
    initResult = fmod.init(0, 32, 128, 1)
    bankIndex1 = fmod.loadBank("Desktop/Master.bank", 0)
    bankIndex2 = fmod.loadBank("Desktop/Master.strings.bank", 0)
end
```

### Playing a 3d sound by manually managing instances
```
instance = fmod.createInstance("event:/OneShot")
fmod.set3DAttributes(instance, love.mouse.getX(), love.mouse.getY(), 0, 0,
                         -1, 0, 0, 0, 1)
fmod.startInstance(instance)
fmod.releaseInstance(instance)
```

### Playing a 2d sound with playOneShot2D
```
fmod.playOneShot2D("event:/Player/OneShot")
```
