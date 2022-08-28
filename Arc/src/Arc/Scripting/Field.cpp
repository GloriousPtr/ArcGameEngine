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
			case MONO_TYPE_R8:			return FieldType::Double;
			case MONO_TYPE_I1:			return FieldType::SByte;
			case MONO_TYPE_U1:			return FieldType::Byte;
			case MONO_TYPE_I2:			return FieldType::Short;
			case MONO_TYPE_U2:			return FieldType::UShort;
			case MONO_TYPE_I4:			return FieldType::Int;
			case MONO_TYPE_U4:			return FieldType::UInt;
			case MONO_TYPE_I8:			return FieldType::Long;
			case MONO_TYPE_U8:			return FieldType::ULong;
			case MONO_TYPE_STRING:		return FieldType::String;
			case MONO_TYPE_VALUETYPE:	return GetFieldTypeFromValueType(monoType);
		}

		return FieldType::Unknown;
	}

	Field::FieldType Field::GetFieldTypeFromValueType(MonoType* monoType)
	{
		const char* name = mono_type_get_name(monoType);

		if (strcmp(name, "ArcEngine.Vector2") == 0) return FieldType::Vec2;
		if (strcmp(name, "ArcEngine.Vector3") == 0) return FieldType::Vec3;
		if (strcmp(name, "ArcEngine.Vector4") == 0) return FieldType::Vec4;
		if (strcmp(name, "ArcEngine.Color") == 0) return FieldType::Color;

		return FieldType::Unknown;
	}

	static size_t GetSizeFromType(const Field::FieldType type)
	{
		switch (type)
		{
		case Field::FieldType::Bool:			return sizeof(bool);
		case Field::FieldType::Float:			return sizeof(float);
		case Field::FieldType::Double:			return sizeof(double);
		case Field::FieldType::SByte:			return sizeof(int8_t);
		case Field::FieldType::Byte:			return sizeof(uint8_t);
		case Field::FieldType::Short:			return sizeof(int16_t);
		case Field::FieldType::UShort:			return sizeof(uint16_t);
		case Field::FieldType::Int:				return sizeof(int32_t);
		case Field::FieldType::UInt:			return sizeof(uint32_t);
		case Field::FieldType::Long:			return sizeof(int64_t);
		case Field::FieldType::ULong:			return sizeof(uint64_t);
		case Field::FieldType::String:			return 256;
		case Field::FieldType::Vec2:			return sizeof(glm::vec2);
		case Field::FieldType::Vec3:			return sizeof(glm::vec3);
		case Field::FieldType::Vec4:			return sizeof(glm::vec4);
		case Field::FieldType::Color:			return sizeof(glm::vec4);
		}

		return 0;
	}

	Field::Field(const eastl::string& name, FieldType type, void* monoClassField, GCHandle handle)
		: Name(name), Type(type), m_Field((MonoClassField*)monoClassField), m_Handle(handle)
	{
		size_t size = GetSizeFromType(type);
		if (type == FieldType::String)
		{
			eastl::string str = GetManagedValueString();
			if (size <= str.size())
				size = str.size();
			m_Data.resize(size);
			memcpy(m_Data.data(), str.data(), size);
		}
		else
		{
			m_Data.resize(size);
			GetManagedValueInternal(m_Data.data());
		}
	}

	Field::~Field()
	{
		m_Data.clear();
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

		const size_t size = str.size();
		if (m_Data.capacity() <= size)
		{
			const size_t newSize = size + 256;
			ARC_CORE_TRACE("Increasing size to {}", newSize);
			m_Data.resize(newSize);
		}

		memcpy(m_Data.data(), str.data(), size);
		MonoString* monoStr = MonoUtils::UTF8ToMonoString(str);
		mono_field_set_value(GCManager::GetReferencedObject(m_Handle), m_Field, monoStr);
	}
}
