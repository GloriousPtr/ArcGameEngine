#pragma once

#include <functional>

namespace ArcEngine
{
	class JobSystem
	{
	public:
		/// <summary>
		/// Create the internal resources such as worker threads, etc. Call it once when initializing the application.
		/// </summary>
		static void Init();
		
		/// <summary>
		/// Add a job to execute asynchronously. Any idle thread will execute this job.
		///		func		: job to execute.
		/// </summary>
		static void Execute(const eastl::function<void()>& job);

		/// <summary>
		/// Divide a job into multiple jobs and execute in parallel.
		///		jobCount	: how many jobs to generate for this task.
		///		groupSize	: how many jobs to execute per thread.
		///		func		: job to execute.
		/// </summary>
		static void ExecuteParallel(uint32_t jobCount, uint32_t groupSize, const eastl::function<void(uint32_t jobIndex, uint32_t groupIndex)>& job);

		/// <summary>
		/// Check if any threads are working.
		/// </summary>
		static bool IsBusy();

		/// <summary>
		/// Wait until all threads become idle.
		/// </summary>
		static void Wait();
	};
}
