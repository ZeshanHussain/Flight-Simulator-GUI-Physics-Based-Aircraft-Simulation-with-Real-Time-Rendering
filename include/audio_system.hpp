#pragma once
#include <string>
#include <map>
#include <vector>

// Forward declare miniaudio types
struct ma_engine;
struct ma_sound;
struct ma_decoder;

enum class SoundType {
    ENGINE,
    STALL_WARNING,
    TERRAIN_500,
    TERRAIN_400,
    TERRAIN_300,
    TERRAIN_200,
    TERRAIN_100,
    TERRAIN_50,
    TERRAIN_40,
    TERRAIN_30,
    TERRAIN_20,
    TERRAIN_10,
    WIND_AMBIENT,
    GEAR_WARNING
};

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();
    
    bool initialize();
    void shutdown();
    
    // Load a sound file
    bool loadSound(SoundType type, const std::string& filepath);
    
    // Generate synthetic sounds (for when we don't have audio files)
    void generateSyntheticSounds();
    
    // Play sound (looping or one-shot)
    void playSound(SoundType type, bool loop = false);
    void stopSound(SoundType type);
    void stopAllSounds();
    
    // Set volume for a specific sound (0.0 to 1.0)
    void setVolume(SoundType type, float volume);
    
    // Set pitch/frequency (1.0 = normal, 2.0 = double speed)
    void setPitch(SoundType type, float pitch);
    
    // Check if sound is playing
    bool isPlaying(SoundType type);
    
    // Update function (call each frame)
    void update(double throttle, double airspeed, double altitude, bool isStalling);
    
private:
    struct Sound {
        void* soundPtr;  // Will be ma_sound*
        bool loaded;
        bool playing;
        bool loop;
        float volume;
        float pitch;
    };
    
    void* enginePtr;  // Will be ma_engine*
    void* devicePtr;  // Will be ma_device* 
    bool initialized;
    
    std::map<SoundType, Sound> sounds;
    
    // State tracking for alerts
    bool stallWarningActive;
    double lastTerrainCallout;
    double terrainCalloutCooldown;
    
    // Play a beep tone
    void playBeep(float frequency, float duration, float volume);
};
