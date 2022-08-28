#pragma once

#include <glm/glm.hpp>

struct ma_sound;

namespace ArcEngine
{
	enum class AttenuationModelType
	{
		None = 0,
		Inverse,
		Linear,
		Exponential
	};

	struct AudioSourceConfig
	{
		float VolumeMultiplier = 1.0f;
		float PitchMultiplier = 1.0f;
		bool PlayOnAwake = true;
		bool Looping = false;

		/*
		float LowPassFilter = -19999.0f;
		float HighPassFilter = 0.0f;
		float MasterReverbSend = 0.0f;
		*/

		bool Spatialization = false;
		AttenuationModelType AttenuationModel = AttenuationModelType::Inverse;
		float RollOff = 1.0f;
		float MinGain = 0.0f;
		float MaxGain = 1.0f;
		float MinDistance = 0.3f;
		float MaxDistance = 1000.0f;

		float ConeInnerAngle = 360.0f;
		float ConeOuterAngle = 360.0f;
		float ConeOuterGain = 0.0f;

		float DopplerFactor = 1.0f;
	};

	class AudioSource
	{
	public:
		AudioSource(const char* filepath);
		~AudioSource();

		const char* GetPath() { return m_Path.c_str(); }

		void Play();
		void Pause();
		void Stop();
		void SetConfig(const AudioSourceConfig& config, const glm::vec3& position, const glm::vec3& forward);

	private:
		eastl::string m_Path;
		Scope<ma_sound> m_Sound = nullptr;
	};
}
