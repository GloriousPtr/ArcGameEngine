#pragma once

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

		float ConeInnerAngle = glm::radians(360.0f);
		float ConeOuterAngle = glm::radians(360.0f);
		float ConeOuterGain = 0.0f;

		float DopplerFactor = 1.0f;
	};

	class AudioSource
	{
	public:
		explicit AudioSource(const char* filepath);
		~AudioSource();

		AudioSource(const AudioSource& other) = delete;
		AudioSource(AudioSource&& other) = delete;

		[[nodiscard]] const char* GetPath() const { return m_Path.c_str(); }

		void Play() const;
		void Pause() const;
		void UnPause() const;
		void Stop() const;
		[[nodiscard]] bool IsPlaying() const;

		void SetConfig(const AudioSourceConfig& config);

		void SetVolume(float volume) const;
		void SetPitch(float pitch) const;
		void SetLooping(bool state) const;
		void SetSpatialization(bool state);
		void SetAttenuationModel(AttenuationModelType type) const;
		void SetRollOff(float rollOff) const;
		void SetMinGain(float minGain) const;
		void SetMaxGain(float maxGain) const;
		void SetMinDistance(float minDistance) const;
		void SetMaxDistance(float maxDistance) const;
		void SetCone(float innerAngle, float outerAngle, float outerGain) const;
		void SetDopplerFactor(float factor) const;

		void SetPosition(const glm::vec3& position) const;
		void SetDirection(const glm::vec3& forward) const;
		void SetVelocity(const glm::vec3& velocity) const;

	private:
		eastl::string m_Path;
		Scope<ma_sound> m_Sound;
		bool m_Spatialization = false;
	};
}
