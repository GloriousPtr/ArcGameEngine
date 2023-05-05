#include "arcpch.h"
#include "AudioSource.h"

#include <miniaudio.h>

#include "AudioEngine.h"

namespace ArcEngine
{
	AudioSource::AudioSource(const char* filepath)
		: m_Path(filepath)
	{
		ARC_PROFILE_SCOPE();

		m_Sound = CreateScope<ma_sound>();

		const ma_result result = ma_sound_init_from_file(static_cast<ma_engine*>(AudioEngine::GetEngine()), filepath, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, nullptr, m_Sound.get());
		if (result != MA_SUCCESS)
			ARC_CORE_ERROR("Failed to initialize sound: {}", filepath);
	}

	AudioSource::~AudioSource()
	{
		ARC_PROFILE_SCOPE();

		ma_sound_uninit(m_Sound.get());
		m_Sound = nullptr;
	}

	void AudioSource::Play() const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_seek_to_pcm_frame(m_Sound.get(), 0);
		ma_sound_start(m_Sound.get());
	}

	void AudioSource::Pause() const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_stop(m_Sound.get());
	}

	void AudioSource::UnPause() const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_start(m_Sound.get());
	}

	void AudioSource::Stop() const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_stop(m_Sound.get());
		ma_sound_seek_to_pcm_frame(m_Sound.get(), 0);
	}

	bool AudioSource::IsPlaying() const
	{
		ARC_PROFILE_SCOPE();

		return ma_sound_is_playing(m_Sound.get());
	}

	static ma_attenuation_model GetAttenuationModel(const AttenuationModelType model)
	{
		ARC_PROFILE_SCOPE();

		switch (model)
		{
			case AttenuationModelType::None:		return ma_attenuation_model_none;
			case AttenuationModelType::Inverse:		return ma_attenuation_model_inverse;
			case AttenuationModelType::Linear:		return ma_attenuation_model_linear;
			case AttenuationModelType::Exponential: return ma_attenuation_model_exponential;
		}

		return ma_attenuation_model_none;
	}

	void AudioSource::SetConfig(const AudioSourceConfig& config)
	{
		ARC_PROFILE_SCOPE();

		ma_sound* sound = m_Sound.get();
		ma_sound_set_volume(sound, config.VolumeMultiplier);
		ma_sound_set_pitch(sound, config.PitchMultiplier);
		ma_sound_set_looping(sound, config.Looping);

		if (m_Spatialization != config.Spatialization)
		{
			m_Spatialization = config.Spatialization;
			ma_sound_set_spatialization_enabled(sound, config.Spatialization);
		}

		if (config.Spatialization)
		{
			ma_sound_set_attenuation_model(sound, GetAttenuationModel(config.AttenuationModel));
			ma_sound_set_rolloff(sound, config.RollOff);
			ma_sound_set_min_gain(sound, config.MinGain);
			ma_sound_set_max_gain(sound, config.MaxGain);
			ma_sound_set_min_distance(sound, config.MinDistance);
			ma_sound_set_max_distance(sound, config.MaxDistance);

			ma_sound_set_cone(sound, config.ConeInnerAngle, config.ConeOuterAngle, config.ConeOuterGain);
			ma_sound_set_doppler_factor(sound, glm::max(config.DopplerFactor, 0.0f));
		}
		else
		{
			ma_sound_set_attenuation_model(sound, ma_attenuation_model_none);
		}
	}

	void AudioSource::SetVolume(const float volume) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_volume(m_Sound.get(), volume);
	}

	void AudioSource::SetPitch(const float pitch) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_pitch(m_Sound.get(), pitch);
	}

	void AudioSource::SetLooping(const bool state) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_looping(m_Sound.get(), state);
	}

	void AudioSource::SetSpatialization(const bool state)
	{
		ARC_PROFILE_SCOPE();

		m_Spatialization = state;
		ma_sound_set_spatialization_enabled(m_Sound.get(), state);
	}

	void AudioSource::SetAttenuationModel(const AttenuationModelType type) const
	{
		ARC_PROFILE_SCOPE();

		if (m_Spatialization)
			ma_sound_set_attenuation_model(m_Sound.get(), GetAttenuationModel(type));
		else
			ma_sound_set_attenuation_model(m_Sound.get(), GetAttenuationModel(AttenuationModelType::None));
	}

	void AudioSource::SetRollOff(const float rollOff) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_rolloff(m_Sound.get(), rollOff);
	}

	void AudioSource::SetMinGain(const float minGain) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_min_gain(m_Sound.get(), minGain);
	}

	void AudioSource::SetMaxGain(const float maxGain) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_max_gain(m_Sound.get(), maxGain);
	}

	void AudioSource::SetMinDistance(const float minDistance) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_min_distance(m_Sound.get(), minDistance);
	}

	void AudioSource::SetMaxDistance(const float maxDistance) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_max_distance(m_Sound.get(), maxDistance);
	}

	void AudioSource::SetCone(const float innerAngle, const float outerAngle, const float outerGain) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_cone(m_Sound.get(), innerAngle, outerAngle, outerGain);
	}

	void AudioSource::SetDopplerFactor(const float factor) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_doppler_factor(m_Sound.get(), glm::max(factor, 0.0f));
	}

	void AudioSource::SetPosition(const glm::vec3& position) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_position(m_Sound.get(), position.x, position.y, position.z);
	}

	void AudioSource::SetDirection(const glm::vec3& forward) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_direction(m_Sound.get(), forward.x, forward.y, forward.z);
	}

	void AudioSource::SetVelocity(const glm::vec3& velocity) const
	{
		ARC_PROFILE_SCOPE();

		ma_sound_set_velocity(m_Sound.get(), velocity.x, velocity.y, velocity.z);
	}
}
