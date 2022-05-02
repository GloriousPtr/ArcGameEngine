#include "arcpch.h"
#include  "Field.h"

#include <mono/jit/jit.h>

namespace ArcEngine
{
	Field::FieldType Field::GetFieldType(MonoType* monoType)
	{
		int type = mono_type_get_type(monoType);
		switch (type)
		{
			case MONO_TYPE_BOOLEAN:		return FieldType::Bool;
			case MONO_TYPE_R4:			return FieldType::Float;
			case MONO_TYPE_I4:			return FieldType::Int;
			case MONO_TYPE_U4:			return FieldType::UnsignedInt;
			case MONO_TYPE_STRING:		return FieldType::String;
			case MONO_TYPE_VALUETYPE:	return GetFieldTypeFromValueType(monoType);
		}

		return FieldType::None;
	}

	void Field::GetValue_Impl(void* instance, void* outValue) const
	{
		mono_field_get_value((MonoObject*)instance, m_Field, outValue);
	}

	void Field::SetValue_Impl(void* instance, void* value) const
	{
		mono_field_set_value((MonoObject*)instance, m_Field, value);
	}

	Field::FieldType Field::GetFieldTypeFromValueType(MonoType* monoType)
	{
		const char* name = mono_type_get_name(monoType);

		if (strcmp(name, "ArcEngine.Vector2") == 0) return FieldType::Vec2;
		if (strcmp(name, "ArcEngine.Vector3") == 0) return FieldType::Vec3;
		if (strcmp(name, "ArcEngine.Vector4") == 0) return FieldType::Vec4;

		return FieldType::None;
	}
}
