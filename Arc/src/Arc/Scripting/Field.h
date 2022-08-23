#pragma once

typedef struct _MonoType MonoType;
typedef struct _MonoClassField MonoClassField;

namespace ArcEngine
{
	using GCHandle = void*;

	struct Field
	{
		enum class FieldType
		{
			None = 0,
			Bool,
			Float,
			Int,
			UnsignedInt,
			String,

			Vec2,
			Vec3,
			Vec4
		};

		eastl::string Name;
		FieldType Type;

		Field(const eastl::string& name, FieldType type, void* monoClassField, GCHandle handle);
		~Field();

		void* GetUnmanagedValue()
		{
			return m_Data;
		}

		template<typename T>
		T GetManagedValue() const
		{
			ARC_PROFILE_SCOPE();

			T value;
			GetManagedValueInternal(&value);
			return value;
		}

		void SetValue(void* value) const
		{
			memcpy(m_Data, value, m_Size);
			SetManagedValue(value);
		}

		eastl::string GetManagedValueString();
		void SetValueString(eastl::string& str);
		size_t GetSize() { return m_Size; }

		static FieldType GetFieldType(MonoType* monoType);

	private:

		void GetManagedValueInternal(void* outValue) const;

		void SetManagedValue(void* value) const;

		static FieldType GetFieldTypeFromValueType(MonoType* monoType);

		GCHandle m_Handle;
		MonoClassField* m_Field;
		void* m_Data;
		size_t m_Size;
	};
}
