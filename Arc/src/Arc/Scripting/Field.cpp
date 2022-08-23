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
		ARC_PROFILE_SCOPE();

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

	static size_t GetSizeFromType(const Field::FieldType type)
	{
		switch (type)
		{
		case Field::FieldType::Bool:			return sizeof(bool);
		case Field::FieldType::Float:			return sizeof(float);
		case Field::FieldType::Int:				return sizeof(int32_t);
		case Field::FieldType::UnsignedInt:		return sizeof(uint32_t);
		case Field::FieldType::String:			return 1024;
		case Field::FieldType::Vec2:			return sizeof(glm::vec2);
		case Field::FieldType::Vec3:			return sizeof(glm::vec3);
		case Field::FieldType::Vec4:			return sizeof(glm::vec4);
		}

		return 0;
	}

	Field::Field(const eastl::string& name, FieldType type, void* monoClassField, GCHandle handle)
		: Name(name), Type(type), m_Field((MonoClassField*)monoClassField), m_Handle(handle)
	{
		m_Size = GetSizeFromType(type);
		m_Data = new char[m_Size];
	}

	Field::~Field()
	{
		delete[m_Size] m_Data;
	}

	void Field::GetManagedValueInternal(void* outValue) const
	{
		ARC_PROFILE_SCOPE();
		mono_field_get_value(GCManager::GetReferencedObject(m_Handle), m_Field, outValue);
	}

	void Field::SetManagedValue(void* value) const
	{
		ARC_PROFILE_SCOPE();
		mono_field_set_value(GCManager::GetReferencedObject(m_Handle), m_Field, value);
	}

	eastl::string Field::GetManagedValueString()
	{
		ARC_PROFILE_SCOPE();

		MonoObject* monoStr = mono_field_get_value_object(ScriptEngine::GetDomain(), m_Field, GCManager::GetReferencedObject(m_Handle));
		return MonoUtils::MonoStringToUTF8((MonoString*)monoStr);
	}

	void Field::SetValueString(eastl::string& str)
	{
		ARC_PROFILE_SCOPE();

		memcpy(m_Data, str.data(), m_Size);
		MonoString* monoStr = MonoUtils::UTF8ToMonoString(str);
		mono_field_set_value(GCManager::GetReferencedObject(m_Handle), m_Field, monoStr);
	}
}
