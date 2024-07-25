#pragma once

namespace ArcEngine
{
	struct WorkQueue;
	typedef void WorkQueueCallback(WorkQueue* queue, void* data);
	struct WorkQueueEntry
	{
		WorkQueueCallback* Callback;
		void* Data;
	};

	struct WorkQueue
	{
		uint32_t volatile CompletionGoal;
		uint32_t volatile CompletionCount;

		uint32_t volatile NextEntryToWrite;
		uint32_t volatile NextEntryToRead;
		HANDLE SemaphoreHandle;
	
		WorkQueueEntry Entries[256];
	};

	struct Win32ThreadInfo
	{
		WorkQueue* Queue;
		wchar_t Name[32];
	};

	class QueueSystem
	{
	public:
		static void Init(WorkQueue* queue);
		static void AddEntry(WorkQueue* queue, WorkQueueCallback* callback, void* data);
		static void CompleteAllWork(WorkQueue* queue);
	};
}
