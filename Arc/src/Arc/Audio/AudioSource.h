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
		explicit AudioSource(const char* filepath);
		~AudioSource();

		AudioSource(const AudioSource& other) = default;
		AudioSource(AudioSource&& other) = default;

		const char* GetPath() const { return m_Path.c_str(); }

		void Play() const;
		void Pause() const;
		void UnPause() const;
		void Stop() const;
		bool IsPlaying() const;

		void SetConfig(const AudioSourceConfig& config);

		void SetVolume(const float volume) const;
		void SetPitch(const float pitch) const;
		void SetLooping(const bool state) const;
		void SetSpatialization(const bool state);
		void SetAttenuationModel(const AttenuationModelType type) const;
		void SetRollOff(const float rollOff) const;
		void SetMinGain(const float minGain) const;
		void SetMaxGain(const float maxGain) const;
		void SetMinDistance(const float minDistance) const;
		void SetMaxDistance(const float maxDistance) const;
		void SetCone(const float innerAngle, const float outerAngle, const float outerGain) const;
		void SetDopplerFactor(const float factor) const;

		void SetPosition(const glm::vec3& position) const;
		void SetDirection(const glm::vec3& forward) const;
		void SetVelocity(const glm::vec3& velocity) const;

	private:
		eastl::string m_Path;
		Scope<ma_sound> m_Sound = nullptr;
		bool m_Spatialization = false;
	};
}
