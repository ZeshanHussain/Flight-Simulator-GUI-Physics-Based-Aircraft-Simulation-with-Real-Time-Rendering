#include "audio_system.hpp"
#include <cmath>
#include <iostream>
#include <cstring>
#include <cstdlib>

// Only define implementation once in this file
#define MINIAUDIO_IMPLEMENTATION
#include "../external/miniaudio.h"

AudioSystem::AudioSystem() 
    : enginePtr(nullptr), devicePtr(nullptr), initialized(false), 
      stallWarningActive(false), lastTerrainCallout(-10.0), 
      terrainCalloutCooldown(3.0) {
}

AudioSystem::~AudioSystem() {
    shutdown();
}

bool AudioSystem::initialize() {
    // Allocate and initialize engine
    enginePtr = malloc(sizeof(ma_engine));
    ma_engine* engine = (ma_engine*)enginePtr;
    
    ma_result result = ma_engine_init(NULL, engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine: " << result << std::endl;
        free(enginePtr);
        enginePtr = nullptr;
        return false;
    }
    
    // Start the engine
    result = ma_engine_start(engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to start audio engine" << std::endl;
        ma_engine_uninit(engine);
        free(enginePtr);
        enginePtr = nullptr;
        return false;
    }
    
    initialized = true;
    std::cout << "✓ Audio system initialized successfully" << std::endl;
    std::cout << "  You should hear beeps for stall warnings and terrain alerts" << std::endl;
    
    // Test beep
    playBeep(440.0f, 0.1f, 0.3f);
    
    return true;
}

void AudioSystem::shutdown() {
    if (initialized && enginePtr) {
        ma_engine* engine = (ma_engine*)enginePtr;
        
        // Stop engine
        ma_engine_stop(engine);
        
        // Uninit engine
        ma_engine_uninit(engine);
        free(enginePtr);
        enginePtr = nullptr;
        
        initialized = false;
        std::cout << "Audio system shut down" << std::endl;
    }
}

void AudioSystem::playBeep(float frequency, float duration, float volume) {
    if (!initialized || !enginePtr) return;
    
    ma_engine* engine = (ma_engine*)enginePtr;
    
    // Create a waveform configuration
    ma_waveform_config waveformConfig = ma_waveform_config_init(
        ma_format_f32,
        2,  // stereo
        ma_engine_get_sample_rate(engine),
        ma_waveform_type_sine,
        0.5,  // amplitude
        frequency
    );
    
    // Allocate waveform
    ma_waveform* waveform = (ma_waveform*)malloc(sizeof(ma_waveform));
    ma_result result = ma_waveform_init(&waveformConfig, waveform);
    
    if (result != MA_SUCCESS) {
        free(waveform);
        return;
    }
    
    // Create a sound from the waveform
    ma_sound* sound = (ma_sound*)malloc(sizeof(ma_sound));
    result = ma_sound_init_from_data_source(engine, waveform, 0, NULL, sound);
    
    if (result != MA_SUCCESS) {
        ma_waveform_uninit(waveform);
        free(waveform);
        free(sound);
        return;
    }
    
    // Set volume and start
    ma_sound_set_volume(sound, volume);
    ma_sound_start(sound);
    
    // Note: In a real implementation, you'd need to track these and clean them up
    // For now, this creates a fire-and-forget beep
}

void AudioSystem::generateSyntheticSounds() {
    std::cout << "Using real-time audio beeps for warnings" << std::endl;
}

bool AudioSystem::loadSound(SoundType type, const std::string& filepath) {
    if (!initialized || !enginePtr) return false;
    
    Sound& snd = sounds[type];
    
    // Allocate sound
    snd.soundPtr = malloc(sizeof(ma_sound));
    ma_sound* sound = (ma_sound*)snd.soundPtr;
    
    ma_result result = ma_sound_init_from_file((ma_engine*)enginePtr, filepath.c_str(), 
                                               MA_SOUND_FLAG_DECODE, NULL, NULL, 
                                               sound);
    
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load sound: " << filepath << std::endl;
        free(snd.soundPtr);
        snd.soundPtr = nullptr;
        return false;
    }
    
    snd.loaded = true;
    snd.playing = false;
    snd.volume = 1.0f;
    snd.pitch = 1.0f;
    
    return true;
}

void AudioSystem::playSound(SoundType type, bool loop) {
    if (!initialized) return;
    
    auto it = sounds.find(type);
    if (it == sounds.end() || !it->second.loaded || !it->second.soundPtr) {
        return;
    }
    
    Sound& snd = it->second;
    ma_sound* sound = (ma_sound*)snd.soundPtr;
    
    if (!snd.playing) {
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
        ma_sound_set_volume(sound, snd.volume);
        ma_sound_start(sound);
        snd.playing = true;
        snd.loop = loop;
    }
}

void AudioSystem::stopSound(SoundType type) {
    if (!initialized) return;
    
    auto it = sounds.find(type);
    if (it == sounds.end() || !it->second.loaded || !it->second.soundPtr) return;
    
    Sound& snd = it->second;
    
    if (snd.playing) {
        ma_sound_stop((ma_sound*)snd.soundPtr);
        snd.playing = false;
    }
}

void AudioSystem::stopAllSounds() {
    for (auto& pair : sounds) {
        stopSound(pair.first);
    }
}

void AudioSystem::setVolume(SoundType type, float volume) {
    if (!initialized) return;
    
    auto it = sounds.find(type);
    if (it == sounds.end()) return;
    
    Sound& snd = it->second;
    snd.volume = std::max(0.0f, std::min(1.0f, volume));
    
    if (snd.loaded && snd.playing && snd.soundPtr) {
        ma_sound_set_volume((ma_sound*)snd.soundPtr, snd.volume);
    }
}

void AudioSystem::setPitch(SoundType type, float pitch) {
    if (!initialized) return;
    
    auto it = sounds.find(type);
    if (it == sounds.end()) return;
    
    Sound& snd = it->second;
    snd.pitch = std::max(0.1f, std::min(4.0f, pitch));
    
    if (snd.loaded && snd.playing && snd.soundPtr) {
        ma_sound_set_pitch((ma_sound*)snd.soundPtr, snd.pitch);
    }
}

bool AudioSystem::isPlaying(SoundType type) {
    auto it = sounds.find(type);
    if (it == sounds.end() || !it->second.soundPtr) return false;
    
    return it->second.playing && ma_sound_is_playing((ma_sound*)it->second.soundPtr);
}

void AudioSystem::update(double throttle, double airspeed, double altitude, bool isStalling) {
    if (!initialized) return;
    
    static double time = 0.0;
    time += 0.016; // Assume ~60 FPS
    
    // ============================================
    // ENGINE SOUND - Varies with throttle
    // ============================================
    static double lastEngineUpdate = 0.0;
    if (time - lastEngineUpdate > 2.0 && throttle > 0.1) {
        // Play a low rumble for engine (varies with throttle)
        float engineFreq = 80.0f + (throttle * 120.0f); // 80-200 Hz
        float engineVol = 0.15f + (throttle * 0.15f);   // 0.15-0.3 volume
        playBeep(engineFreq, 0.2f, engineVol);
        
        lastEngineUpdate = time;
        std::cout << "🔊 Engine: " << (int)(throttle * 100) << "% power" << std::endl;
    }
    
    // ============================================
    // STALL WARNING - Activated at low airspeed
    // ============================================
    double stallSpeed = 40.0; // m/s (~78 knots)
    double stallBuffer = 5.0;  // m/s buffer zone
    
    if (airspeed < stallSpeed + stallBuffer && !isStalling) {
        if (!stallWarningActive) {
            std::cout << "⚠️  STALL WARNING - Low airspeed!" << std::endl;
            std::cout << "    Airspeed: " << (airspeed * 1.94384) << " kts" << std::endl;
            stallWarningActive = true;
            
            // Play initial stall warning beep
            playBeep(800.0f, 0.3f, 0.5f);
        }
        
        // Continuous beeping
        static double lastStallBeep = 0.0;
        if (time - lastStallBeep > 0.5) {
            playBeep(800.0f, 0.2f, 0.4f);
            std::cout << "🔴 BEEP BEEP BEEP" << std::endl;
            lastStallBeep = time;
        }
    } else if (airspeed > stallSpeed + stallBuffer + 5.0) {
        if (stallWarningActive) {
            stallWarningActive = false;
            std::cout << "✓ Airspeed recovered" << std::endl;
        }
    }
    
    // ============================================
    // TERRAIN WARNINGS - Altitude callouts
    // ============================================
    double altitudeFeet = altitude * 3.28084;
    
    // Only give warnings if descending
    static double lastAltitude = altitudeFeet;
    bool descending = (altitudeFeet < lastAltitude - 10.0);
    lastAltitude = altitudeFeet;
    
    if (descending && (time - lastTerrainCallout) > terrainCalloutCooldown) {
        const char* calloutText = nullptr;
        float frequency = 600.0f;
        bool shouldCallout = false;
        
        if (altitudeFeet < 510 && altitudeFeet > 490) {
            calloutText = "500";
            shouldCallout = true;
        } else if (altitudeFeet < 410 && altitudeFeet > 390) {
            calloutText = "400";
            shouldCallout = true;
        } else if (altitudeFeet < 310 && altitudeFeet > 290) {
            calloutText = "300";
            shouldCallout = true;
        } else if (altitudeFeet < 210 && altitudeFeet > 190) {
            calloutText = "200";
            frequency = 700.0f;
            shouldCallout = true;
        } else if (altitudeFeet < 110 && altitudeFeet > 90) {
            calloutText = "100";
            frequency = 800.0f;
            shouldCallout = true;
            terrainCalloutCooldown = 1.0;
        } else if (altitudeFeet < 55 && altitudeFeet > 45) {
            calloutText = "50";
            frequency = 900.0f;
            shouldCallout = true;
            terrainCalloutCooldown = 0.5;
        } else if (altitudeFeet < 45 && altitudeFeet > 35) {
            calloutText = "40";
            frequency = 950.0f;
            shouldCallout = true;
            terrainCalloutCooldown = 0.5;
        } else if (altitudeFeet < 35 && altitudeFeet > 25) {
            calloutText = "30";
            frequency = 1000.0f;
            shouldCallout = true;
            terrainCalloutCooldown = 0.4;
        } else if (altitudeFeet < 25 && altitudeFeet > 15) {
            calloutText = "20";
            frequency = 1100.0f;
            shouldCallout = true;
            terrainCalloutCooldown = 0.3;
        } else if (altitudeFeet < 15 && altitudeFeet > 5) {
            calloutText = "10";
            frequency = 1200.0f;
            shouldCallout = true;
            terrainCalloutCooldown = 0.2;
        }
        
        if (shouldCallout) {
            std::cout << "📢 TERRAIN: " << calloutText << " feet" << std::endl;
            
            // Play warning beep with increasing frequency as we get lower
            playBeep(frequency, 0.3f, 0.6f);
            
            lastTerrainCallout = time;
        }
    }
    
    // Reset cooldown if climbing
    if (!descending && altitudeFeet > 200) {
        terrainCalloutCooldown = 3.0;
    }
}
