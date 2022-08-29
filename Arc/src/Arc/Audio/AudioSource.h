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
		void UnPause();
		void Stop();
		bool IsPlaying();

		void SetConfig(const AudioSourceConfig& config);

		void SetVolume(const float volume);
		void SetPitch(const float pitch);
		void SetLooping(const bool state);
		void SetSpatialization(const bool state);
		void SetAttenuationModel(const AttenuationModelType type);
		void SetRollOff(const float rollOff);
		void SetMinGain(const float minGain);
		void SetMaxGain(const float maxGain);
		void SetMinDistance(const float minDistance);
		void SetMaxDistance(const float maxDistance);
		void SetCone(const float innerAngle, const float outerAngle, const float outerGain);
		void SetDopplerFactor(const float factor);

		void SetPosition(const glm::vec3& position);
		void SetDirection(const glm::vec3& forward);
		void SetVelocity(const glm::vec3& velocity);

	private:
		eastl::string m_Path;
		Scope<ma_sound> m_Sound = nullptr;
		bool m_Spatialization = false;
	};
}
