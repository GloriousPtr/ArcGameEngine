#include "arcpch.h"
#include "JobSystem.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace ArcEngine
{
	// Fixed size very simple thread safe ring buffer
	template <typename T, size_t capacity>
	class ThreadSafeRingBuffer
	{
	public:
		// Push an item to the end if there is free space
		//  Returns true if successful
		//  Returns false if there is not enough space
		inline bool push_back(const T& item)
		{
			bool result = false;
			lock.lock();
			size_t next = (head + 1) % capacity;
			if (next != tail)
			{
				data[head] = item;
				head = next;
				result = true;
			}
			lock.unlock();
			return result;
		}

		// Get an item if there are any
		//  Returns true if successful
		//  Returns false if there are no items
		inline bool pop_front(T& item)
		{
			bool result = false;
			lock.lock();
			if (tail != head)
			{
				item = data[tail];
				tail = (tail + 1) % capacity;
				result = true;
			}
			lock.unlock();
			return result;
		}

	private:
		T data[capacity];
		size_t head = 0;
		size_t tail = 0;
		std::mutex lock; // this just works better than a spinlock here (on windows)
	};

	static uint32_t numThreads = 0;
	ThreadSafeRingBuffer<std::function<void()>, 256> jobPool;
	std::condition_variable wakeCondition;
	std::mutex wakeMutex;
	uint64_t currentLabel = 0;
	std::atomic<uint64_t> finishedLabel;

	void JobSystem::Init()
	{
		finishedLabel.store(0);
		auto numCores = std::thread::hardware_concurrency();
		numThreads = std::max(1u, numCores);

		for (uint32_t threadID = 0; threadID < numThreads; ++threadID)
		{
			std::thread worker([threadID]() {
				std::wstring name = std::format(L"Worker {}", threadID);
				ARC_PROFILE_THREAD(name.c_str());
				std::function<void()> job;

				while (true)
				{
					if (jobPool.pop_front(job))
					{
						job();
						finishedLabel.fetch_add(1);
					}
					else
					{
						std::unique_lock<std::mutex> lock(wakeMutex);
						wakeCondition.wait(lock);
					}
				}
			});

			worker.detach();
		}
	}

	inline void Poll()
	{
		wakeCondition.notify_one();
		std::this_thread::yield();
	}

	void JobSystem::Execute(const std::function<void()>& job)
	{
		currentLabel += 1;
		while (!jobPool.push_back(job))
			Poll();
		wakeCondition.notify_one();
	}

	void JobSystem::ExecuteParallel(uint32_t jobCount, uint32_t groupSize, const std::function<void(uint32_t jobIndex, uint32_t groupIndex)>& job)
	{
		if (jobCount == 0 || groupSize == 0)
			return;

		const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;
		currentLabel += groupCount;
		for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			auto jobGroup = [jobCount, groupSize, job, groupIndex]() {
				const uint32_t groupJobOffset = groupIndex * groupSize;
				const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);
				for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
					job(i, groupIndex);
			};

			while (!jobPool.push_back(jobGroup))
				Poll();
			wakeCondition.notify_one();
		}
	}

	bool JobSystem::IsBusy()
	{
		return finishedLabel.load() < currentLabel;
	}

	void JobSystem::Wait()
	{
		while (IsBusy())
			Poll();
	}
}
