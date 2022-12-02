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
		char* Data = nullptr;

		void Set(EditorContextType type, const void* data, size_t size)
		{
			delete[] Data;

			if (size != 0)
			{
				Type = type;
				m_Size = size;
				Data = new char[m_Size];
				memcpy(Data, data, m_Size);
			}
			else
			{
				Type = EditorContextType::None;
				Data = nullptr;
			}
		}

		void Reset()
		{
			Set(EditorContextType::None, nullptr, 0);
		}

		bool IsValid(EditorContextType type) const { return type == Type && Data != nullptr; }
		operator bool() const { return Type != EditorContextType::None && Data != nullptr; }

	private:
		size_t m_Size = 0;
	};
}
