#pragma once

#include <mutex>
#include "Arc/Core/Log.h"

#include <EASTL/functional.h>
#include <EASTL/string.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace ArcEngine
{
	class ExternalConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		struct Message
		{
			eastl::string Buffer;
			eastl::string CallerPath;
			eastl::string CallerFunction;
			int32_t CallerLine;
			Log::Level Level;

			Message(eastl::string_view message, eastl::string_view callerPath, eastl::string_view callerFunction, int32_t callerLine, Log::Level level)
				: Buffer(message), CallerPath(callerPath), CallerFunction(callerFunction), CallerLine(callerLine), Level(level)
			{
			}
		};

		explicit ExternalConsoleSink(bool forceFlush = false, uint8_t bufferCapacity = 10)
			: m_MessageBufferCapacity(forceFlush ? 1 : bufferCapacity), m_MessageBuffer(eastl::vector<Ref<Message>>(forceFlush ? 1 : bufferCapacity))
		{
		}
		ExternalConsoleSink(const ExternalConsoleSink&) = delete;
		ExternalConsoleSink& operator=(const ExternalConsoleSink&) = delete;
		~ExternalConsoleSink() override = default;

		static void SetConsoleSink_HandleFlush(eastl::function<void(eastl::string, eastl::string, eastl::string, int32_t, Log::Level)> func)
		{
			ARC_PROFILE_SCOPE();

			OnFlush = func;
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			ARC_PROFILE_SCOPE();

			if (OnFlush == nullptr)
			{
				flush_();
				return;
			}

			spdlog::memory_buf_t formatted;
			base_sink<std::mutex>::formatter_->format(msg, formatted);
			*(m_MessageBuffer.begin() + m_MessagesBuffered) = CreateRef<Message>(fmt::to_string(formatted).c_str(), msg.source.filename, msg.source.funcname, msg.source.line, GetMessageLevel(msg.level));

			if (++m_MessagesBuffered == m_MessageBufferCapacity)
				flush_();
		}

		void flush_() override
		{
			ARC_PROFILE_SCOPE();

			if (OnFlush == nullptr)
				return;

			for (const Ref<Message>& msg : m_MessageBuffer)
				OnFlush(msg->Buffer, msg->CallerPath, msg->CallerFunction, msg->CallerLine, msg->Level);

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
			}
			return Log::Level::Trace;
		}
	private:
		uint8_t m_MessagesBuffered = 0;
		uint8_t m_MessageBufferCapacity;
		eastl::vector<Ref<Message>> m_MessageBuffer;

		static eastl::function<void(eastl::string, eastl::string, eastl::string, int32_t, Log::Level)> OnFlush;
	};
}
