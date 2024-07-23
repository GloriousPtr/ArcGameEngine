#include "arcpch.h"
#include "QueueSystem.h"

namespace ArcEngine
{
	bool DoNextWorkQueueEntry(WorkQueue* q);

	DWORD WINAPI ThreadProccc(LPVOID lpParam)
	{
		Win32ThreadInfo* threadInfo = (Win32ThreadInfo*)lpParam;
		for (;;)
		{
			if (DoNextWorkQueueEntry(threadInfo->Queue))
			{
				WaitForSingleObjectEx(threadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
			}
		}
	}

	#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))
	static Win32ThreadInfo threadInfos[6];
	void QueueSystem::Init([[]]WorkQueue* queue)
	{
		constexpr uint32_t InitialCount = 0;
		constexpr uint32_t threadCount = ArrayCount(threadInfos);
		queue->SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, threadCount, 0, 0, SEMAPHORE_ALL_ACCESS);

		for (uint32_t threadIndex = 0; threadIndex < threadCount; threadIndex++)
		{
			Win32ThreadInfo* info = &threadInfos[threadIndex];
			info->Queue = queue;

			DWORD threadId;
			HANDLE threadHandle = CreateThread(0, 0, ThreadProccc, info, 0, &threadId);

			WCHAR buffer[256];
			wsprintf(buffer, L"Queue Thread %u", threadIndex);
			SetThreadDescription(threadHandle, buffer);
			CloseHandle(threadHandle);
		}
	}

	void QueueSystem::AddEntry(WorkQueue* queue, WorkQueueCallback* callback, void* data)
	{
		// TODO: add InterlockedCompareExchange
		uint32_t originalNextEntryToWrite = queue->NextEntryToWrite;
		uint32_t newNextEntryToWrite = (originalNextEntryToWrite + 1) % ArrayCount(queue->Entries);
		assert(newNextEntryToWrite != queue->NextEntryToRead);
		WorkQueueEntry* entry = queue->Entries + originalNextEntryToWrite;
		entry->Callback = callback;
		entry->Data = data;
		InterlockedIncrement((LONG volatile *)&queue->CompletionGoal);
		_WriteBarrier();
		_mm_sfence();
		queue->NextEntryToWrite = newNextEntryToWrite;
		ReleaseSemaphore(queue->SemaphoreHandle, 1, 0);
	}

	bool DoNextWorkQueueEntry(WorkQueue* queue)
	{
		bool shouldSleep = false;

		uint32_t originalNextEntryToRead = queue->NextEntryToRead;
		uint32_t newNextEntryToRead = (originalNextEntryToRead + 1) % ArrayCount(queue->Entries);
		if (originalNextEntryToRead != queue->NextEntryToWrite)
		{
			uint32_t index = InterlockedCompareExchange((LONG volatile *)&queue->NextEntryToRead,
												   newNextEntryToRead,
												   originalNextEntryToRead);
			if (index == originalNextEntryToRead)
			{
				WorkQueueEntry entry = queue->Entries[index];
				entry.Callback(queue, entry.Data);
				InterlockedIncrement((LONG volatile *)&queue->CompletionCount);
			}
		}
		else
		{
			shouldSleep = true;
		}
		return shouldSleep;
	}

	void QueueSystem::CompleteAllWork(WorkQueue* queue)
	{
		while (queue->CompletionGoal != queue->CompletionCount)
		{
			DoNextWorkQueueEntry(queue);
		}

		queue->CompletionGoal = 0;
		queue->CompletionCount = 0;
	}
}
