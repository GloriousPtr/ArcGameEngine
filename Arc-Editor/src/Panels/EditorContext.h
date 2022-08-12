#pragma once

namespace ArcEngine
{
	enum class EditorContextType
	{
		None = 0,
		Entity,
		File
	};

	struct EditorContext
	{
		EditorContextType Type = EditorContextType::None;
		void* Data = nullptr;
		size_t Size = 0;

		EditorContext() = delete;

		void Set(EditorContextType type, void* data, size_t size)
		{
			if (Size != 0)
				delete[Size] Data;

			Type = type;

			if (size != 0)
			{
				Data = new char[size];
				memcpy(Data, data, size);
			}
		}
	};
}
