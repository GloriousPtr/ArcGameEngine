#pragma once

typedef struct _MonoType MonoType;
typedef struct _MonoClassField MonoClassField;

namespace ArcEngine
{
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

		std::string Name;
		FieldType Type;

		Field(const std::string& name, FieldType type, void* monoClassField)
			: Name(name), Type(type), m_Field((MonoClassField*)monoClassField)
		{
		}

		template<typename T>
		T GetValue(void* instance) const
		{
			T value;
			GetValue_Impl(instance, &value);
			return value;
		}

		template<typename T>
		void SetValue(void* instance, T value) const
		{
			SetValue_Impl(instance, &value);
		}

		static FieldType GetFieldType(MonoType* monoType);

	private:
		void GetValue_Impl(void* instance, void* outValue) const;
		void SetValue_Impl(void* instance, void* value) const;

		static FieldType GetFieldTypeFromValueType(MonoType* monoType);

		MonoClassField* m_Field;
	};
}
