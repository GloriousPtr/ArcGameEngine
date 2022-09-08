#include "arcpch.h"
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-debug.h>

#ifdef ARC_DEBUG
#include <mono/metadata/debug-helpers.h>
#endif // ARC_DEBUG

#include "Arc/Scene/Entity.h"
#include "MonoUtils.h"
#include "GCManager.h"
#include "ScriptEngineRegistry.h"

namespace ArcEngine
{
	static const eastl::unordered_map<eastl::string, FieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single",		FieldType::Float },
		{ "System.Double",		FieldType::Double },
		{ "System.Boolean",		FieldType::Bool },
		{ "System.Char",		FieldType::Char },
		{ "System.SByte",		FieldType::Byte },
		{ "System.Int16",		FieldType::Short },
		{ "System.Int32",		FieldType::Int },
		{ "System.Int64",		FieldType::Long },
		{ "System.Byte",		FieldType::UByte },
		{ "System.UInt16",		FieldType::UShort },
		{ "System.UInt32",		FieldType::UInt },
		{ "System.UInt64",		FieldType::ULong },
		{ "System.String",		FieldType::String },
		
		{ "ArcEngine.Vector2",	FieldType::Vector2 },
		{ "ArcEngine.Vector3",	FieldType::Vector3 },
		{ "ArcEngine.Vector4",	FieldType::Vector4 },
		{ "ArcEngine.Color",	FieldType::Color },
	};

	struct ScriptEngineData
	{
		eastl::string CoreAssemblyPath = "../Sandbox/Assemblies/Arc-ScriptCore.dll";
		eastl::string AppAssemblyPath = "../Sandbox/Assemblies/Sandbox.dll";

		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoAssembly* AppAssembly = nullptr;
		MonoImage* CoreImage = nullptr;
		MonoImage* AppImage = nullptr;

		MonoClass* EntityClass = nullptr;
		MonoClass* SerializeFieldAttribute = nullptr;
		MonoClass* HideInPropertiesAttribute = nullptr;
		MonoClass* ShowInPropertiesAttribute = nullptr;
		MonoClass* HeaderAttribute = nullptr;
		MonoClass* TooltipAttribute = nullptr;
		MonoClass* RangeAttribute = nullptr;

		eastl::unordered_map<eastl::string, Ref<ScriptClass>> EntityClasses;
		eastl::unordered_map<UUID, eastl::unordered_map<eastl::string, eastl::unordered_map<eastl::string, ScriptFieldInstance>>> EntityFields;

		using EntityInstanceMap = eastl::unordered_map<UUID, eastl::unordered_map<eastl::string, ScriptInstance*>>;
		EntityInstanceMap EntityRuntimeInstances;
	};

	static Ref<ScriptEngineData> s_Data;

	Scene* ScriptEngine::s_CurrentScene = nullptr;

	void ScriptEngine::Init()
	{
		ARC_PROFILE_SCOPE();

		mono_set_assemblies_path("mono/lib");

		s_Data = CreateRef<ScriptEngineData>();

		static char* options[] =
		{
			"--soft-breakpoints",
			"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n"
		};
		mono_jit_parse_options(sizeof(options) / sizeof(char*), (char**)options);
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);

		s_Data->RootDomain = mono_jit_init("ArcJITRuntime");
		ARC_CORE_ASSERT(s_Data->RootDomain);
		mono_debug_domain_create(s_Data->RootDomain);
		mono_thread_set_main(mono_thread_current());

		GCManager::Init();

		ReloadAppDomain();
	}

	void ScriptEngine::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		s_Data->EntityClasses.clear();
		s_Data->EntityFields.clear();
		s_Data->EntityRuntimeInstances.clear();

		GCManager::Shutdown();

		mono_jit_cleanup(s_Data->RootDomain);
	}

	void ScriptEngine::LoadCoreAssembly()
	{
		ARC_PROFILE_SCOPE();

		s_Data->CoreAssembly = MonoUtils::LoadMonoAssembly(s_Data->CoreAssemblyPath.c_str());
		s_Data->CoreImage = mono_assembly_get_image(s_Data->CoreAssembly);

		s_Data->EntityClass = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "Entity");
		s_Data->SerializeFieldAttribute = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "SerializeField");
		s_Data->HideInPropertiesAttribute = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "HideInProperties");
		s_Data->ShowInPropertiesAttribute = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "ShowInProperties");
		s_Data->HeaderAttribute = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "Header");
		s_Data->TooltipAttribute = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "Tooltip");
		s_Data->RangeAttribute = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "Range");

		GCManager::CollectGarbage();
	}

	void ScriptEngine::LoadClientAssembly()
	{
		ARC_PROFILE_SCOPE();

		s_Data->AppAssembly = MonoUtils::LoadMonoAssembly(s_Data->AppAssemblyPath.c_str());
		s_Data->AppImage = mono_assembly_get_image(s_Data->AppAssembly);

		GCManager::CollectGarbage();

		s_Data->EntityClasses.clear();
		LoadAssemblyClasses(s_Data->AppAssembly);

		ScriptEngineRegistry::ClearTypes();
		ScriptEngineRegistry::RegisterInternalCalls();
		ScriptEngineRegistry::RegisterTypes();
	}

	void ScriptEngine::ReloadAppDomain()
	{
		if (s_Data->AppDomain)
		{
			mono_domain_set(s_Data->RootDomain, true);
			mono_domain_unload(s_Data->AppDomain);
			s_Data->AppDomain = nullptr;
		}

		//Compile app assembly
		{
			const char* command = "dotnet msbuild ../Sandbox/Sandbox.sln"
				" -nologo"																	// no microsoft branding in console
				" -noconlog"																// no console logs
				//" -t:rebuild"																// rebuild the project
				" -m"																		// multiprocess build
				" -flp1:Verbosity=minimal;logfile=AssemblyBuildErrors.log;errorsonly"		// dump errors in AssemblyBuildErrors.log file
				" -flp2:Verbosity=minimal;logfile=AssemblyBuildWarnings.log;warningsonly";	// dump warnings in AssemblyBuildWarnings.log file

			system(command);

			// Errors
			{
				FILE* errors = fopen("AssemblyBuildErrors.log", "r");

				char buffer[4096];
				if (errors != nullptr)
				{
					while (fgets(buffer, sizeof(buffer), errors))
					{
						if (buffer)
						{
							size_t newLine = strlen(buffer) - 1;
							buffer[newLine] = '\0';
							ARC_ERROR(buffer);
						}
					}

					fclose(errors);
				}
				else
				{
					ARC_CORE_ERROR("AssemblyBuildErrors.log not found!");
				}
			}

			// Warnings
			{
				FILE* warns = fopen("AssemblyBuildWarnings.log", "r");

				char buffer[1024];
				if (warns != nullptr)
				{
					while (fgets(buffer, sizeof(buffer), warns))
					{
						if (buffer)
						{
							size_t newLine = strlen(buffer) - 1;
							buffer[newLine] = '\0';
							ARC_WARN(buffer);
						}
					}

					fclose(warns);
				}
				else
				{
					ARC_CORE_ERROR("AssemblyBuildWarnings.log not found!");
				}
			}
		}

		s_Data->AppDomain = mono_domain_create_appdomain("ScriptRuntime", nullptr);
		ARC_CORE_ASSERT(s_Data->AppDomain);
		mono_domain_set(s_Data->AppDomain, true);
		mono_debug_domain_create(s_Data->AppDomain);

		LoadCoreAssembly();
		LoadClientAssembly();
	}

	void ArcEngine::ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		ARC_PROFILE_SCOPE();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

		for (int32_t i = 0; i < numTypes; ++i)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			eastl::string fullname = fmt::format("{}.{}", nameSpace, name).c_str();

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);
			if (!monoClass)
				continue;

			MonoClass* parentClass = mono_class_get_parent(monoClass);
			
			if (parentClass)
			{
				const char* parentName = mono_class_get_name(parentClass);
				const char* parentNamespace = mono_class_get_namespace(parentClass);

				bool isEntity = monoClass && strcmp(parentName, "Entity") == 0 && strcmp(parentNamespace, "ArcEngine") == 0;
				if (isEntity)
					s_Data->EntityClasses[fullname] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreImage;
	}

	MonoImage* ScriptEngine::GetAppAssemblyImage()
	{
		return s_Data->AppImage;
	}

	ScriptInstance* ScriptEngine::CreateInstance(Entity entity, const eastl::string& name)
	{
		const auto& scriptClass = s_Data->EntityClasses.at(name);
		UUID entityID = entity.GetUUID();
		ScriptInstance* instance = new ScriptInstance(scriptClass, entityID);
		s_Data->EntityRuntimeInstances[entityID][name] = instance;
		return instance;
	}

	bool ScriptEngine::HasInstance(Entity entity, const eastl::string& name)
	{
		UUID id = entity.GetUUID();
		return s_Data->EntityRuntimeInstances[id].find(name) != s_Data->EntityRuntimeInstances[id].end();
	}

	ScriptInstance* ScriptEngine::GetInstance(Entity entity, const eastl::string& name)
	{
		return s_Data->EntityRuntimeInstances.at(entity.GetUUID()).at(name);
	}

	bool ScriptEngine::HasClass(const eastl::string& className)
	{
		return s_Data->EntityClasses.find_as(className) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::RemoveInstance(Entity entity, const eastl::string& name)
	{
		delete s_Data->EntityRuntimeInstances.at(entity.GetUUID()).at(name);
		s_Data->EntityRuntimeInstances[entity.GetUUID()].erase(name);
	}

	MonoDomain* ScriptEngine::GetDomain()
	{
		return s_Data->AppDomain;
	}

	const eastl::vector<eastl::string>& ScriptEngine::GetFields(const char* className)
	{
		return s_Data->EntityClasses.at(className)->GetFields();
	}

	const eastl::unordered_map<eastl::string, ScriptField>& ScriptEngine::GetFieldMap(const char* className)
	{
		return s_Data->EntityClasses.at(className)->GetFieldsMap();
	}

	eastl::unordered_map<eastl::string, ScriptFieldInstance>& ScriptEngine::GetFieldInstanceMap(Entity entity, const char* className)
	{
		return s_Data->EntityFields[entity.GetUUID()][className];
	}

	eastl::unordered_map<eastl::string, Ref<ScriptClass>>& ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	////////////////////////////////////////////////////////////////////////
	// Script Class ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	ScriptClass::ScriptClass(MonoClass* monoClass)
		: m_MonoClass(monoClass)
	{
		LoadFields();
	}

	ScriptClass::ScriptClass(const eastl::string& classNamespace, const eastl::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(s_Data->AppImage, classNamespace.c_str(), className.c_str());
		LoadFields();
	}

	GCHandle ScriptClass::Instantiate()
	{
		MonoObject* object = mono_object_new(s_Data->AppDomain, m_MonoClass);
		if (object)
		{
			mono_runtime_object_init(object);
			return GCManager::CreateObjectReference(object, false);
		}

		return nullptr;
	}

	MonoMethod* ScriptClass::GetMethod(const char* methodName, uint32_t parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, methodName, parameterCount);
	}

	GCHandle ScriptClass::InvokeMethod(GCHandle gcHandle, MonoMethod* method, void** params)
	{
		MonoObject* reference = GCManager::GetReferencedObject(gcHandle);
		MonoObject* exception;
		mono_runtime_invoke(method, reference, params, &exception);
		if (exception != nullptr)
		{
			MonoString* monoString = mono_object_to_string(exception, nullptr);
			eastl::string ex = MonoUtils::MonoStringToUTF8(monoString);
			ARC_CRITICAL(ex.c_str());
		}
		return gcHandle;
	}

	enum class Accessibility : uint8_t
	{
		None = 0,
		Private = (1 << 0),
		Internal = (1 << 1),
		Protected = (1 << 2),
		Public = (1 << 3)
	};

	// Gets the accessibility level of the given field
	static uint8_t GetFieldAccessibility(MonoClassField* field)
	{
		uint8_t accessibility = (uint8_t)Accessibility::None;
		uint32_t accessFlag = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

		switch (accessFlag)
		{
		case MONO_FIELD_ATTR_PRIVATE:
		{
			accessibility = (uint8_t)Accessibility::Private;
			break;
		}
		case MONO_FIELD_ATTR_FAM_AND_ASSEM:
		{
			accessibility |= (uint8_t)Accessibility::Protected;
			accessibility |= (uint8_t)Accessibility::Internal;
			break;
		}
		case MONO_FIELD_ATTR_ASSEMBLY:
		{
			accessibility = (uint8_t)Accessibility::Internal;
			break;
		}
		case MONO_FIELD_ATTR_FAMILY:
		{
			accessibility = (uint8_t)Accessibility::Protected;
			break;
		}
		case MONO_FIELD_ATTR_FAM_OR_ASSEM:
		{
			accessibility |= (uint8_t)Accessibility::Private;
			accessibility |= (uint8_t)Accessibility::Protected;
			break;
		}
		case MONO_FIELD_ATTR_PUBLIC:
		{
			accessibility = (uint8_t)Accessibility::Public;
			break;
		}
		default:
			break;
		}

		return accessibility;
	}

	void ScriptClass::LoadFields()
	{
		m_Fields.clear();
		m_FieldsMap.clear();

		MonoObject* tempObject = mono_object_new(s_Data->AppDomain, m_MonoClass);
		GCHandle tempObjectHandle = GCManager::CreateObjectReference(tempObject, false);
		mono_runtime_object_init(tempObject);

		MonoClassField* monoField;
		void* ptr = nullptr;
		while ((monoField = mono_class_get_fields(m_MonoClass, &ptr)) != nullptr)
		{
			const char* fieldName = mono_field_get_name(monoField);

			MonoType* fieldType = mono_field_get_type(monoField);
			eastl::string typeName = mono_type_get_name(fieldType);

			FieldType type = FieldType::Unknown;
			if (s_ScriptFieldTypeMap.find(typeName) != s_ScriptFieldTypeMap.end())
				type = s_ScriptFieldTypeMap.at(typeName);

			if (type == FieldType::Unknown)
			{
				ARC_CORE_WARN("Unsupported Field Type Name: {}", typeName);

				/*
				MonoClass* m = mono_type_get_class(fieldType);
				if (m)
				{
					ARC_CORE_WARN("\tenum {}", mono_class_is_enum(m));
					MonoClassField* f;
					void* ptrr = nullptr;
					while ((f = mono_class_get_fields(m, &ptrr)) != nullptr)
					{
						ARC_CORE_WARN("\t\t {}", mono_field_get_name(f));
					}
				}

				*/
				continue;
			}

			uint8_t accessibilityFlag = GetFieldAccessibility(monoField);
			bool serializable = accessibilityFlag & (uint8_t)Accessibility::Public;
			bool hidden = !serializable;
			eastl::string header = "";
			eastl::string tooltip = "";
			float min = 0;
			float max = 0;

			MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(m_MonoClass, monoField);
			if (attr)
			{
				if (!serializable)
					serializable = mono_custom_attrs_has_attr(attr, s_Data->SerializeFieldAttribute);

				hidden = !serializable;

				if (mono_custom_attrs_has_attr(attr, s_Data->HideInPropertiesAttribute))
					hidden = true;
				else if (mono_custom_attrs_has_attr(attr, s_Data->ShowInPropertiesAttribute))
					hidden = false;

				if (mono_custom_attrs_has_attr(attr, s_Data->HeaderAttribute))
				{
					MonoObject* attributeObject = mono_custom_attrs_get_attr(attr, s_Data->HeaderAttribute);
					MonoClassField* messageField = mono_class_get_field_from_name(s_Data->HeaderAttribute, "Message");
					MonoObject* monoStr = mono_field_get_value_object(ScriptEngine::GetDomain(), messageField, attributeObject);
					header = MonoUtils::MonoStringToUTF8((MonoString*)monoStr);
				}

				if (mono_custom_attrs_has_attr(attr, s_Data->TooltipAttribute))
				{
					MonoObject* attributeObject = mono_custom_attrs_get_attr(attr, s_Data->TooltipAttribute);
					MonoClassField* messageField = mono_class_get_field_from_name(s_Data->TooltipAttribute, "Message");
					MonoObject* monoStr = mono_field_get_value_object(ScriptEngine::GetDomain(), messageField, attributeObject);
					tooltip = MonoUtils::MonoStringToUTF8((MonoString*)monoStr);
				}

				if (mono_custom_attrs_has_attr(attr, s_Data->RangeAttribute))
				{
					MonoObject* attributeObject = mono_custom_attrs_get_attr(attr, s_Data->RangeAttribute);
					MonoClassField* minField = mono_class_get_field_from_name(s_Data->RangeAttribute, "Min");
					MonoClassField* maxField = mono_class_get_field_from_name(s_Data->RangeAttribute, "Max");
					mono_field_get_value(attributeObject, minField, &min);
					mono_field_get_value(attributeObject, maxField, &max);
				}
			}

			auto& scriptField = m_FieldsMap[fieldName];
			scriptField.Name = fieldName;
			scriptField.Type = type;
			scriptField.Field = monoField;
			scriptField.Serializable = serializable;
			scriptField.Hidden = hidden;
			scriptField.Header = header;
			scriptField.Tooltip = tooltip;
			scriptField.Min = min;
			scriptField.Max = max;

			if (type == FieldType::String)
			{
				MonoString* monoStr = (MonoString*)mono_field_get_value_object(s_Data->AppDomain, monoField, tempObject);
				eastl::string str = MonoUtils::MonoStringToUTF8(monoStr);
				memcpy(scriptField.DefaultValue, str.data(), sizeof(scriptField.DefaultValue));
			}
			else
			{
				mono_field_get_value(tempObject, monoField, scriptField.DefaultValue);
			}

			m_Fields.push_back(fieldName);
		}

		GCManager::ReleaseObjectReference(tempObjectHandle);
	}

	////////////////////////////////////////////////////////////////////////
	// Script Instance /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, UUID entityID)
		: m_ScriptClass(scriptClass)
	{
		m_Handle = scriptClass->Instantiate();

		m_EntityClass = CreateRef<ScriptClass>(s_Data->EntityClass);
		m_Constructor = m_EntityClass->GetMethod(".ctor", 1);
		void* params = &entityID;
		m_EntityClass->InvokeMethod(m_Handle, m_Constructor, &params);
		
		eastl::string fullClassName = fmt::format("{}.{}", scriptClass->m_ClassNamespace.c_str(), scriptClass->m_ClassName.c_str()).c_str();
		auto& fieldsMap = s_Data->EntityFields[entityID][fullClassName];
		for (const auto& [fieldName, field] : scriptClass->m_FieldsMap)
		{
			if (fieldsMap.find(fieldName) != fieldsMap.end())
			{
				const ScriptFieldInstance& fieldInstance = fieldsMap.at(fieldName);
				SetFieldValueInternal(fieldName, fieldInstance.GetBuffer());
			}
		}

		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		m_OnDestroyMethod = scriptClass->GetMethod("OnDestroy", 0);

		m_OnCollisionEnter2DMethod = m_EntityClass->GetMethod("HandleOnCollisionEnter2D", 1);
		m_OnCollisionExit2DMethod = m_EntityClass->GetMethod("HandleOnCollisionExit2D", 1);
		m_OnSensorEnter2DMethod = m_EntityClass->GetMethod("HandleOnSensorEnter2D", 1);
		m_OnSensorExit2DMethod = m_EntityClass->GetMethod("HandleOnSensorExit2D", 1);
	}

	ScriptInstance::~ScriptInstance()
	{
		GCManager::ReleaseObjectReference(m_Handle);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Handle, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod)
		{
			void* params = &ts;
			m_ScriptClass->InvokeMethod(m_Handle, m_OnUpdateMethod, &params);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (m_OnDestroyMethod)
			m_ScriptClass->InvokeMethod(m_Handle, m_OnDestroyMethod);
	}

	void ScriptInstance::InvokeOnCollisionEnter2D(Collision2DData& other)
	{
		void* params = &other;
		m_EntityClass->InvokeMethod(m_Handle, m_OnCollisionEnter2DMethod, &params);
	}

	void ScriptInstance::InvokeOnCollisionExit2D(Collision2DData& other)
	{
		void* params = &other;
		m_EntityClass->InvokeMethod(m_Handle, m_OnCollisionExit2DMethod, &params);
	}

	void ScriptInstance::InvokeOnSensorEnter2D(Collision2DData& other)
	{
		void* params = &other;
		m_EntityClass->InvokeMethod(m_Handle, m_OnSensorEnter2DMethod, &params);
	}

	void ScriptInstance::InvokeOnSensorExit2D(Collision2DData& other)
	{
		void* params = &other;
		m_EntityClass->InvokeMethod(m_Handle, m_OnSensorExit2DMethod, &params);
	}

	void ScriptInstance::GetFieldValueInternal(const eastl::string& name, void* value) const
	{
		MonoClassField* classField = m_ScriptClass->m_FieldsMap.at(name).Field;
		mono_field_get_value(GCManager::GetReferencedObject(m_Handle), classField, value);
	}

	void ScriptInstance::SetFieldValueInternal(const eastl::string& name, const void* value)
	{
		const auto& field = m_ScriptClass->m_FieldsMap.at(name);
		MonoClassField* classField = field.Field;
		if (field.Type == FieldType::String)
		{
			MonoString* monoStr = mono_string_new(s_Data->AppDomain, (const char*)value);
			mono_field_set_value(GCManager::GetReferencedObject(m_Handle), classField, monoStr);
		}
		else
		{
			mono_field_set_value(GCManager::GetReferencedObject(m_Handle), classField, (void*)value);
		}
	}

	eastl::string ScriptInstance::GetFieldValueStringInternal(const eastl::string& name) const
	{
		MonoClassField* classField = m_ScriptClass->m_FieldsMap.at(name).Field;
		MonoString* monoStr = (MonoString*)mono_field_get_value_object(s_Data->AppDomain, classField, GCManager::GetReferencedObject(m_Handle));
		eastl::string str = MonoUtils::MonoStringToUTF8(monoStr);
		return str;
	}
}
