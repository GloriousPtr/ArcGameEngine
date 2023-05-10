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
		void Set(EditorContextType type, const void* data, size_t size)
		{
			m_Data.clear();

			if (data && size != 0)
			{
				m_Type = type;
				m_Size = size;
				m_Data.resize(m_Size);
				memcpy(m_Data.data(), data, m_Size);
			}
			else
			{
				m_Type = EditorContextType::None;
				m_Data.clear();
			}
		}

		void Reset()
		{
			Set(EditorContextType::None, nullptr, 0);
		}

		EditorContext() = default;
		~EditorContext()
		{
			m_Data.clear();
		}

		EditorContext(const EditorContext& other)
		{
			Set(other.m_Type, other.m_Data.data(), other.m_Size);
		}

		EditorContext& operator=(const EditorContext& other)
		{
			Set(other.m_Type, other.m_Data.data(), other.m_Size);
			return *this;
		}

		EditorContext(EditorContext&& other) = delete;
		EditorContext operator=(EditorContext&& other) = delete;

		[[nodiscard]] EditorContextType GetType() const { return m_Type; }

		template<typename T>
		[[nodiscard]] const T* As() const { ARC_CORE_ASSERT(!m_Data.empty(), "EditorContext Data is null"); return reinterpret_cast<const T*>(m_Data.data()); }

		[[nodiscard]] bool IsValid(EditorContextType type) const { return type == m_Type && !m_Data.empty(); }
		operator bool() const { return m_Type != EditorContextType::None && !m_Data.empty(); }
		bool operator==(const EditorContext& other) const { return m_Type == other.m_Type && m_Data.data() == other.m_Data.data(); }

	private:
		EditorContextType m_Type = EditorContextType::None;
		std::vector<uint8_t> m_Data;
		size_t m_Size = 0;
	};
}
