#pragma once

#include <mutex>

#include <EASTL/string.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace ArcEngine
{
	template<class Mutex>
	class ExternalConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		struct Message
		{
			eastl::string Buffer;
			Log::Level Level;

			Message(eastl::string& message, Log::Level level)
				: Buffer(message), Level(level)
			{
			}
		};

		explicit ExternalConsoleSink(bool forceFlush = false, uint8_t bufferCapacity = 10)
			: m_MessageBufferCapacity(forceFlush ? 1 : bufferCapacity), m_MessageBuffer(eastl::vector<Ref<Message>>(forceFlush ? 1 : bufferCapacity))
		{
		}
		ExternalConsoleSink(const ExternalConsoleSink&) = delete;
		ExternalConsoleSink& operator=(const ExternalConsoleSink&) = delete;
		virtual ~ExternalConsoleSink() = default;

		static void SetConsoleSink_HandleFlush(eastl::function<void(eastl::string, Log::Level)> func)
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
			*(m_MessageBuffer.begin() + m_MessagesBuffered) = CreateRef<Message>(eastl::string(fmt::to_string(formatted).c_str()), GetMessageLevel(msg.level));

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
		static Log::Level GetMessageLevel(const spdlog::level::level_enum level)
		{
			switch (level)
			{
				case spdlog::level::level_enum::trace:			return Log::Level::Trace;
				case spdlog::level::level_enum::debug:			return Log::Level::Debug;
				case spdlog::level::level_enum::info:			return Log::Level::Info;
				case spdlog::level::level_enum::warn:			return Log::Level::Warn;
				case spdlog::level::level_enum::err:			return Log::Level::Error;
				case spdlog::level::level_enum::critical:		return Log::Level::Critical;
				default:										return Log::Level::Trace;
			}
			return Log::Level::Trace;
		}
	private:
		uint8_t m_MessagesBuffered = 0;
		uint8_t m_MessageBufferCapacity;
		eastl::vector<Ref<Message>> m_MessageBuffer;

		static eastl::function<void(eastl::string, Log::Level)> OnFlush;
	};
}

namespace ArcEngine
{
	using ExternalConsoleSink_mt = ExternalConsoleSink<std::mutex>;                  // multi-threaded
	using ExternalConsoleSink_st = ExternalConsoleSink<spdlog::details::null_mutex>; // single threaded
}
