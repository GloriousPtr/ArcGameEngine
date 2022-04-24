#pragma once
#include "spdlog/sinks/base_sink.h"

namespace ArcEngine
{
	template<class Mutex>
	class ExternalConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		struct Message
		{
			std::string Buffer;
			LogLevel Level;

			Message(std::string& message, LogLevel level)
				: Buffer(message), Level(level)
			{
			}
		};

		explicit ExternalConsoleSink(bool forceFlush = false, uint8_t bufferCapacity = 10)
			: m_MessageBufferCapacity(forceFlush ? 1 : bufferCapacity), m_MessageBuffer(std::vector<Ref<Message>>(forceFlush ? 1 : bufferCapacity))
		{
		}
		ExternalConsoleSink(const ExternalConsoleSink&) = delete;
		ExternalConsoleSink& operator=(const ExternalConsoleSink&) = delete;
		virtual ~ExternalConsoleSink() = default;

		static void SetConsoleSink_HandleFlush(std::function<void(std::string, LogLevel)> func)
		{
			OnFlush = func;
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			if (OnFlush == nullptr)
			{
				flush_();
				return;
			}

			spdlog::memory_buf_t formatted;
			base_sink<std::mutex>::formatter_->format(msg, formatted);
			*(m_MessageBuffer.begin() + m_MessagesBuffered) = CreateRef<Message>(fmt::to_string(formatted), GetMessageLevel(msg.level));

			if (++m_MessagesBuffered == m_MessageBufferCapacity)
				flush_();
		}

		void flush_() override
		{
			if (OnFlush == nullptr)
				return;

			for (Ref<Message>& msg : m_MessageBuffer)
				OnFlush(msg->Buffer, msg->Level);

			m_MessagesBuffered = 0;
		}
	private:
		static LogLevel GetMessageLevel(const spdlog::level::level_enum level)
		{
			switch (level)
			{
				case spdlog::level::level_enum::trace:			return LogLevel::Trace;
				case spdlog::level::level_enum::debug:			return LogLevel::Debug;
				case spdlog::level::level_enum::info:			return LogLevel::Info;
				case spdlog::level::level_enum::warn:			return LogLevel::Warn;
				case spdlog::level::level_enum::err:			return LogLevel::Error;
				case spdlog::level::level_enum::critical:		return LogLevel::Critical;
				case spdlog::level::level_enum::off:			return LogLevel::Off;
			}
			return LogLevel::Trace;
		}
	private:
		uint8_t m_MessagesBuffered = 0;
		uint8_t m_MessageBufferCapacity;
		std::vector<Ref<Message>> m_MessageBuffer;

		static std::function<void(std::string, LogLevel)> OnFlush;
	};
}

#include "spdlog/details/null_mutex.h"
#include <mutex>
namespace ArcEngine
{
	using ExternalConsoleSink_mt = ExternalConsoleSink<std::mutex>;                  // multi-threaded
	using ExternalConsoleSink_st = ExternalConsoleSink<spdlog::details::null_mutex>; // single threaded
}
