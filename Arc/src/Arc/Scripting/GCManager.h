#pragma once

typedef struct _MonoObject MonoObject;

namespace ArcEngine
{
	using GCHandle = uint32_t;

	class GCManager
	{
	public:
		static void Init();
		static void Shutdown();

		static void CollectGarbage(bool blockUntilFinalized = true);

		[[nodiscard]] static GCHandle CreateObjectReference(MonoObject* managedObject, bool weakReference, bool pinned = false, bool track = true);
		[[nodiscard]] static MonoObject* GetReferencedObject(GCHandle handle);
		static void ReleaseObjectReference(GCHandle handle);
	};
}
