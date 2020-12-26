#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

namespace ArcEngine
{
	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;
	
	struct ProfileResult
	{
	    std::string Name;
	    
		FloatingPointMicroseconds Start;
		std::chrono::microseconds ElapsedTime;
	    std::thread::id ThreadID;
	};

	struct InstrumentationSession
	{
	    std::string Name;
	};

	class Instrumentor
	{
	private:
		std::mutex m_Mutex;
	    InstrumentationSession* m_CurrentSession;
	    std::ofstream m_OutputStream;
	public:
	    Instrumentor()
	        : m_CurrentSession(nullptr)
	    {
	    }

	    void BeginSession(const std::string& name, const std::string& filepath = "results.json")
	    {
			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession) 
			{
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				if (Log::GetCoreLogger()) 
				{ // Edge case: BeginSession() might be before Log::Init()
					ARC_CORE_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", name, m_CurrentSession->Name);
				}
				InternalEndSession();
			}
	    	
	        m_OutputStream.open(filepath);
	    	
	        if (m_OutputStream.is_open()) 
			{
				m_CurrentSession = new InstrumentationSession({name});
				WriteHeader();
			}
	    	else 
			{
				if (Log::GetCoreLogger()) 
				{ // Edge case: BeginSession() might be before Log::Init()
					ARC_CORE_ERROR("Instrumentor could not open results file '{0}'.", filepath);
				}
			}
	    }

	    void EndSession()
	    {
	        std::lock_guard lock(m_Mutex);
			InternalEndSession();
	    }

	    void WriteProfile(const ProfileResult& result)
	    {
	        std::stringstream json;

	        std::string name = result.Name;
	        std::replace(name.begin(), name.end(), '"', '\'');

	    	json << std::setprecision(3) << std::fixed;
	        json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
			json << "\"name\":\"" << name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.ThreadID << ",";
			json << "\"ts\":" << result.Start.count();
			json << "}";

	        std::lock_guard lock(m_Mutex);
			if (m_CurrentSession) 
			{
				m_OutputStream << json.str();
				m_OutputStream.flush();
			}
	    }

		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}

	private:
	    void WriteHeader()
	    {
	        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
	        m_OutputStream.flush();
	    }

	    void WriteFooter()
	    {
	        m_OutputStream << "]}";
	        m_OutputStream.flush();
	    }

		// Note: you must already own lock on m_Mutex before
		// calling InternalEndSession()
		void InternalEndSession()
		{
			if (m_CurrentSession) 
			{
				WriteFooter();
				m_OutputStream.close();
				delete m_CurrentSession;
				m_CurrentSession = nullptr;
			}
		}
	};

	class InstrumentationTimer
	{
	public:
	    InstrumentationTimer(const char* name)
	        : m_Name(name), m_Stopped(false)
	    {
	    	m_StartTimepoint = std::chrono::steady_clock::now();
	    }

	    ~InstrumentationTimer()
	    {
	        if (!m_Stopped)
	            Stop();
	    }

	    void Stop()
	    {
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

	        Instrumentor::Get().WriteProfile({ m_Name, highResStart, elapsedTime, std::this_thread::get_id() });

	        m_Stopped = true;
	    }
	private:
	    const char* m_Name;
	    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	    bool m_Stopped;
	};
}

#define ARC_PROFILE 0
#if ARC_PROFILE
	#define ARC_PROFILE_BEGIN_SESSION(name, filepath) ::ArcEngine::Instrumentor::Get().BeginSession(name, filepath)
	#define ARC_PROFILE_END_SESSION() ::ArcEngine::Instrumentor::Get().EndSession()
	#define ARC_PROFILE_SCOPE(name) ::ArcEngine::InstrumentationTimer timer##__LINE__(name)
	#define ARC_PROFILE_FUNCTION() ARC_PROFILE_SCOPE(ARC_FUNC_SIG)
#else
	#define ARC_PROFILE_BEGIN_SESSION(name, filepath)
	#define ARC_PROFILE_END_SESSION()
	#define ARC_PROFILE_SCOPE(name)
	#define ARC_PROFILE_FUNCTION()
#endif

