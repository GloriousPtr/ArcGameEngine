#pragma once

typedef struct _MonoObject MonoObject;

namespace ArcEngine
{
	using GCHandle = void*;

	class GCManager
	{
	public:
		static void Init();
		static void Shutdown();

		static void CollectGarbage(bool blockUntilFinalized = true);

		static GCHandle CreateObjectReference(MonoObject* managedObject, bool weakReference, bool pinned = false, bool track = true);
		static MonoObject* GetReferencedObject(GCHandle handle);
		static void ReleaseObjectReference(GCHandle handle);
	};
}
