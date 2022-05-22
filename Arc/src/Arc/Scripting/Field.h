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

		Field(const eastl::string& name, FieldType type, void* monoClassField)
			: Name(name), Type(type), m_Field((MonoClassField*)monoClassField)
		{
		}

		template<typename T>
		T GetValue(GCHandle handle) const
		{
			T value;
			GetValue_Impl(handle, &value);
			return value;
		}

		template<typename T>
		void SetValue(GCHandle handle, T value) const
		{
			SetValue_Impl(handle, &value);
		}

		eastl::string GetValueString(GCHandle handle);
		void SetValueString(GCHandle handle, eastl::string& str);

		static FieldType GetFieldType(MonoType* monoType);

	private:
		void GetValue_Impl(GCHandle handle, void* outValue) const;
		void SetValue_Impl(GCHandle handle, void* value) const;

		static FieldType GetFieldTypeFromValueType(MonoType* monoType);

		MonoClassField* m_Field;
	};
}
