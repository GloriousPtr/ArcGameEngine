#include "arcpch.h"
#include "AudioSource.h"

#include "miniaudio.h"

#include "AudioEngine.h"

namespace ArcEngine
{
	AudioSource::AudioSource(const char* filepath)
		: m_Path(filepath)
	{
		m_Sound = CreateScope<ma_sound>();

		ma_result result = ma_sound_init_from_file((ma_engine*)AudioEngine::GetEngine(), filepath, 0, nullptr, nullptr, m_Sound.get());
		if (result != MA_SUCCESS)
			ARC_CORE_ERROR("Failed to initialize sound: {}", filepath);
	}

	AudioSource::~AudioSource()
	{
		ma_sound_uninit(m_Sound.get());
		m_Sound = nullptr;
	}

	void AudioSource::Play()
	{
		ma_sound_start(m_Sound.get());
	}

	void AudioSource::Pause()
	{
		ma_sound_stop(m_Sound.get());
	}

	void AudioSource::Stop()
	{
		ma_sound_stop(m_Sound.get());
		ma_sound_seek_to_pcm_frame(m_Sound.get(), 0);
	}

	static ma_attenuation_model GetAttenuationModel(const AttenuationModelType model)
	{
		switch (model)
		{
			case AttenuationModelType::None:		return ma_attenuation_model_none;
			case AttenuationModelType::Inverse:		return ma_attenuation_model_inverse;
			case AttenuationModelType::Linear:		return ma_attenuation_model_linear;
			case AttenuationModelType::Exponential: return ma_attenuation_model_exponential;
		}

		return ma_attenuation_model_none;
	}

	void AudioSource::SetConfig(const AudioSourceConfig& config, const glm::vec3& position, const glm::vec3& forward)
	{
		ma_sound* sound = m_Sound.get();
		ma_sound_set_volume(sound, config.VolumeMultiplier);
		ma_sound_set_pitch(sound, config.PitchMultiplier);
		ma_sound_set_looping(sound, config.Looping);

		ma_sound_set_spatialization_enabled(sound, config.Spatialization);

		if (config.Spatialization)
		{
			ma_sound_set_attenuation_model(sound, GetAttenuationModel(config.AttenuationModel));
			ma_sound_set_rolloff(sound, config.RollOff);
			ma_sound_set_min_gain(sound, config.MinGain);
			ma_sound_set_max_gain(sound, config.MaxGain);
			ma_sound_set_min_distance(sound, config.MinDistance);
			ma_sound_set_max_distance(sound, config.MaxDistance);

			ma_sound_set_cone(sound, glm::radians(config.ConeInnerAngle), glm::radians(config.ConeOuterAngle), config.ConeOuterGain);
			ma_sound_set_doppler_factor(sound, glm::max(config.DopplerFactor, 0.0f));

			ma_sound_set_position(sound, position.x, position.y, position.z);
			ma_sound_set_direction(sound, forward.x, forward.y, forward.z);
		}
		else
		{
			ma_sound_set_attenuation_model(sound, ma_attenuation_model_none);
		}
	}
}
