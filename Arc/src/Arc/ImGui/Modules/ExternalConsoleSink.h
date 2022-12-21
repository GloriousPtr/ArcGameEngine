#pragma once

#include <mutex>
#include "Arc/Core/Log.h"

#include <functional>
#include <string>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace ArcEngine
{
	class ExternalConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		struct Message
		{
			std::string Buffer;
			std::string CallerPath;
			std::string CallerFunction;
			int32_t CallerLine;
			Log::Level Level;

			Message(std::string_view message, std::string_view callerPath, std::string_view callerFunction, int32_t callerLine, Log::Level level)
				: Buffer(message), CallerPath(callerPath), CallerFunction(callerFunction), CallerLine(callerLine), Level(level)
			{
			}
		};

		explicit ExternalConsoleSink(bool forceFlush = false, uint8_t bufferCapacity = 10)
			: m_MessageBufferCapacity(forceFlush ? 1 : bufferCapacity), m_MessageBuffer(std::vector<Ref<Message>>(forceFlush ? 1 : bufferCapacity))
		{
		}
		ExternalConsoleSink(const ExternalConsoleSink&) = delete;
		ExternalConsoleSink& operator=(const ExternalConsoleSink&) = delete;
		~ExternalConsoleSink() override = default;

		static void SetConsoleSink_HandleFlush(std::function<void(std::string, std::string, std::string, int32_t, Log::Level)> func)
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
			std::string filename = msg.source.filename ? msg.source.filename : "";
			std::string funcname = msg.source.funcname ? msg.source.funcname : "";
			*(m_MessageBuffer.begin() + m_MessagesBuffered) = CreateRef<Message>(fmt::to_string(formatted), filename, funcname, msg.source.line, GetMessageLevel(msg.level));

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
		std::vector<Ref<Message>> m_MessageBuffer;

		static std::function<void(std::string, std::string, std::string, int32_t, Log::Level)> OnFlush;
	};
}
