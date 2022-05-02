#include "arcpch.h"
#include  "Field.h"

#include <mono/jit/jit.h>

#include "GCManager.h"
#include "MonoUtils.h"
#include "ScriptEngine.h"

namespace ArcEngine
{
	Field::FieldType Field::GetFieldType(MonoType* monoType)
	{
		OPTICK_EVENT();

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

	Field::FieldType Field::GetFieldTypeFromValueType(MonoType* monoType)
	{
		const char* name = mono_type_get_name(monoType);

		if (strcmp(name, "ArcEngine.Vector2") == 0) return FieldType::Vec2;
		if (strcmp(name, "ArcEngine.Vector3") == 0) return FieldType::Vec3;
		if (strcmp(name, "ArcEngine.Vector4") == 0) return FieldType::Vec4;

		return FieldType::None;
	}

	void Field::GetValue_Impl(GCHandle handle, void* outValue) const
	{
		OPTICK_EVENT();

		mono_field_get_value(GCManager::GetReferencedObject(handle), m_Field, outValue);
	}

	void Field::SetValue_Impl(GCHandle handle, void* value) const
	{
		OPTICK_EVENT();

		mono_field_set_value(GCManager::GetReferencedObject(handle), m_Field, value);
	}

	std::string Field::GetValueString(GCHandle handle)
	{
		OPTICK_EVENT();

		MonoObject* monoStr = mono_field_get_value_object(ScriptEngine::GetDomain(), m_Field, GCManager::GetReferencedObject(handle));
		return MonoUtils::MonoStringToUTF8((MonoString*)monoStr);
	}

	void Field::SetValueString(GCHandle handle, std::string& str)
	{
		OPTICK_EVENT();

		MonoString* monoStr = MonoUtils::UTF8ToMonoString(str);
		mono_field_set_value(GCManager::GetReferencedObject(handle), m_Field, monoStr);
	}
}
