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
			Unknown = 0,
			Bool,
			Float,
			Double,
			SByte,
			Byte,
			Short,
			UShort,
			Int,
			UInt,
			Long,
			ULong,
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
			return (void*)m_Data.data();
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
			memcpy((void*)m_Data.data(), value, m_Data.size());
			SetManagedValue(value);
		}

		eastl::string GetManagedValueString();
		void SetValueString(eastl::string& str);
		size_t GetSize() { return m_Data.size(); }

		static FieldType GetFieldType(MonoType* monoType);

	private:

		void GetManagedValueInternal(void* outValue) const;

		void SetManagedValue(void* value) const;

		static FieldType GetFieldTypeFromValueType(MonoType* monoType);

		GCHandle m_Handle;
		MonoClassField* m_Field;
		eastl::vector<char> m_Data;
	};
}
