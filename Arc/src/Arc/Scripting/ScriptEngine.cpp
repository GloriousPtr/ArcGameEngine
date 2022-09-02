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

#include "MonoUtils.h"
#include "GCManager.h"
#include "ScriptEngineRegistry.h"

namespace ArcEngine
{
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

		using EntityInstanceMap = eastl::unordered_map<UUID, eastl::unordered_map<eastl::string, ScriptInstance*>>;
		EntityInstanceMap EntityInstances;
		EntityInstanceMap EntityRuntimeInstances;
		EntityInstanceMap* CurrentEntityInstanceMap = nullptr;
	};

	static Ref<ScriptEngineData> s_Data;

	Scene* ScriptEngine::s_CurrentScene = nullptr;

	void ScriptEngine::Init()
	{
		ARC_PROFILE_SCOPE();

		mono_set_assemblies_path("mono/lib");

		s_Data = CreateRef<ScriptEngineData>();

		static const char* options[] =
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

		for (auto [id, script] : s_Data->EntityInstances)
		{
			for (auto& it = script.begin(); it != script.end(); ++it)
			{
				delete it->second;
			}
		}
		s_Data->EntityClasses.clear();

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
		// Clean old instances
		for (auto [id, script] : s_Data->EntityInstances)
		{
			for (auto& it = script.begin(); it != script.end(); ++it)
			{
				GCManager::ReleaseObjectReference(it->second->GetHandle());
			}
		}

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

		// Recreate instances
		eastl::unordered_map<UUID, eastl::unordered_map<eastl::string, ScriptInstance*>> entityInstances;
		for (auto [id, script] : s_Data->EntityInstances)
		{
			for (auto [name, scriptInstance] : script)
			{
				auto& scriptClass = s_Data->EntityClasses.at(name);
				if (s_Data->EntityInstances[id][name] != nullptr)
				{
					entityInstances[id][name] = s_Data->EntityInstances[id][name];
					entityInstances[id][name]->Invalidate(scriptClass, id);
				}
				else
				{
					entityInstances[id][name] = new ScriptInstance(scriptClass, id);
				}
			}
		}
		s_Data->EntityInstances = entityInstances;

		s_Data->CurrentEntityInstanceMap = &s_Data->EntityInstances;
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
				
				ARC_CORE_TRACE(fullname.c_str());
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

	void ScriptEngine::OnRuntimeBegin()
	{
		s_Data->CurrentEntityInstanceMap = &s_Data->EntityRuntimeInstances;

		for (auto [id, script] : s_Data->EntityInstances)
		{
			for (auto [name, scriptInstance] : script)
			{
				ScriptInstance* copy = new ScriptInstance(scriptInstance, id);
				s_Data->EntityRuntimeInstances[id][name] = copy;
			}
		}
	}

	void ScriptEngine::OnRuntimeEnd()
	{
		s_Data->CurrentEntityInstanceMap = &s_Data->EntityInstances;
		for (auto [id, script] : s_Data->EntityRuntimeInstances)
		{
			for (auto& it = script.begin(); it != script.end(); ++it)
				delete it->second;
		}
		s_Data->EntityRuntimeInstances.clear();
	}

	ScriptInstance* ScriptEngine::CreateInstance(Entity entity, const eastl::string& name)
	{
		auto& scriptClass = s_Data->EntityClasses.at(name);
		UUID entityID = entity.GetUUID();
		ScriptInstance* instance = new ScriptInstance(scriptClass, entityID);
		(*s_Data->CurrentEntityInstanceMap)[entityID][name] = instance;
		return (*s_Data->CurrentEntityInstanceMap)[entityID][name];
	}

	bool ScriptEngine::HasInstance(Entity entity, const eastl::string& name)
	{
		UUID id = entity.GetUUID();
		return (*s_Data->CurrentEntityInstanceMap)[id].find(name) != (*s_Data->CurrentEntityInstanceMap)[id].end();
	}

	ScriptInstance* ScriptEngine::GetInstance(Entity entity, const eastl::string& name)
	{
		return (*s_Data->CurrentEntityInstanceMap)[entity.GetUUID()].at(name);
	}

	bool ScriptEngine::HasClass(const eastl::string& className)
	{
		return s_Data->EntityClasses.find_as(className) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::RemoveInstance(Entity entity, const eastl::string& name)
	{
		delete (*s_Data->CurrentEntityInstanceMap)[entity.GetUUID()][name];
		(*s_Data->CurrentEntityInstanceMap)[entity.GetUUID()].erase(name);
	}

	MonoDomain* ScriptEngine::GetDomain()
	{
		return s_Data->AppDomain;
	}

	eastl::vector<eastl::string>& ScriptEngine::GetFields(Entity entity, const char* className)
	{
		return (*s_Data->CurrentEntityInstanceMap)[entity.GetUUID()].at(className)->GetFields();
	}

	eastl::map<eastl::string, Ref<Field>>& ScriptEngine::GetFieldMap(Entity entity, const char* className)
	{
		return (*s_Data->CurrentEntityInstanceMap)[entity.GetUUID()].at(className)->GetFieldMap();
	}

	eastl::unordered_map<eastl::string, Ref<ScriptClass>>& ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	void ScriptEngine::SetProperty(GCHandle handle, void* property, void** params)
	{
		ARC_PROFILE_SCOPE();

		MonoMethod* method = mono_property_get_set_method((MonoProperty*)property);
		MonoObject* reference = GCManager::GetReferencedObject(handle);
		if (reference)
			mono_runtime_invoke(method, reference, params, nullptr);
	}

	MonoProperty* ScriptEngine::GetProperty(const char* className, const char* propertyName)
	{
		ARC_PROFILE_SCOPE();

		return s_Data->EntityClasses.at(className)->GetProperty(className, propertyName);
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

	MonoProperty* ScriptClass::GetProperty(const char* className, const char* propertyName)
	{
		ARC_PROFILE_SCOPE();

		eastl::string key = eastl::string(className) + propertyName;
		if (m_Properties.find(key) != m_Properties.end())
			return m_Properties.at(key);

		MonoProperty* property = mono_class_get_property_from_name(m_MonoClass, propertyName);
		if (property)
			m_Properties.emplace(key, property);
		else
			ARC_CORE_ERROR("Property: {0} not found in class {1}", propertyName, className);

		return property;
	}

	void ScriptClass::SetProperty(GCHandle gcHandle, void* property, void** params)
	{
		ARC_PROFILE_SCOPE();

		MonoMethod* method = mono_property_get_set_method((MonoProperty*)property);
		InvokeMethod(gcHandle, method, params);
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
		}

		return accessibility;
	}

	void ScriptClass::LoadFields()
	{
		m_Fields.clear();
		m_FieldsMap.clear();

		MonoClassField* monoField;
		void* ptr = 0;
		while ((monoField = mono_class_get_fields(m_MonoClass, &ptr)) != NULL)
		{
			const char* propertyName = mono_field_get_name(monoField);
			m_Fields.push_back(propertyName);
			m_FieldsMap.emplace(propertyName, monoField);
		}
	}

	////////////////////////////////////////////////////////////////////////
	// Script Instance /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	ScriptInstance::ScriptInstance(ScriptInstance* scriptInstance, UUID entityID)
	{
		m_ScriptClass = scriptInstance->m_ScriptClass;

		MonoObject* copy = mono_object_clone(GCManager::GetReferencedObject(scriptInstance->m_Handle));
		m_Handle = GCManager::CreateObjectReference(copy, false);

		m_EntityClass = CreateRef<ScriptClass>(s_Data->EntityClass);
		m_Constructor = m_EntityClass->GetMethod(".ctor", 1);
		void* params = &entityID;
		m_EntityClass->InvokeMethod(m_Handle, m_Constructor, &params);

		LoadMethods();
		LoadFields();
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, UUID entityID)
		: m_ScriptClass(scriptClass)
	{
		m_Handle = scriptClass->Instantiate();

		m_EntityClass = CreateRef<ScriptClass>(s_Data->EntityClass);
		m_Constructor = m_EntityClass->GetMethod(".ctor", 1);
		void* params = &entityID;
		m_EntityClass->InvokeMethod(m_Handle, m_Constructor, &params);
		
		LoadMethods();
		LoadFields();
	}

	ScriptInstance::~ScriptInstance()
	{
		GCManager::ReleaseObjectReference(m_Handle);
	}

	void ScriptInstance::Invalidate(Ref<ScriptClass> scriptClass, UUID entityID)
	{
		m_ScriptClass = scriptClass;
		m_Handle = scriptClass->Instantiate();

		m_EntityClass = CreateRef<ScriptClass>(s_Data->EntityClass);
		m_Constructor = m_EntityClass->GetMethod(".ctor", 1);
		void* params = &entityID;
		m_EntityClass->InvokeMethod(m_Handle, m_Constructor, &params);

		LoadMethods();
		LoadFields();
	}

	void ScriptInstance::LoadMethods()
	{
		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);
		m_OnDestroyMethod = m_ScriptClass->GetMethod("OnDestroy", 0);

		m_OnCollisionEnter2DMethod = m_EntityClass->GetMethod("HandleOnCollisionEnter2D", 1);
		m_OnCollisionExit2DMethod = m_EntityClass->GetMethod("HandleOnCollisionExit2D", 1);
		m_OnSensorEnter2DMethod = m_EntityClass->GetMethod("HandleOnSensorEnter2D", 1);
		m_OnSensorExit2DMethod = m_EntityClass->GetMethod("HandleOnSensorExit2D", 1);
	}

	void ScriptInstance::LoadFields()
	{
		m_Fields.clear();

		auto& fieldMap = m_ScriptClass->m_FieldsMap;
		eastl::map<eastl::string, Ref<Field>> finalFields;

		for (auto fieldName : m_ScriptClass->m_Fields)
		{
			MonoClassField* monoField = fieldMap.at(fieldName);
			MonoType* fieldType = mono_field_get_type(monoField);
			Field::FieldType type = Field::GetFieldType(fieldType);
			
			if (type == Field::FieldType::Unknown)
				continue;

			const char* name = fieldName.c_str();
			uint8_t accessibilityFlag = GetFieldAccessibility(monoField);
			bool serializable = accessibilityFlag & (uint8_t)Accessibility::Public;
			bool hidden = !serializable;
			eastl::string header = "";
			eastl::string tooltip = "";
			float min = 0;
			float max = 0;

			MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(m_ScriptClass->m_MonoClass, monoField);
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

			bool alreadyPresent = m_FieldsMap.find(fieldName) != m_FieldsMap.end();
			bool sameType = alreadyPresent && m_FieldsMap.at(fieldName)->Type == type;

			if (sameType)
			{
				void* value = m_FieldsMap[fieldName]->GetUnmanagedValue();
				size_t size = m_FieldsMap[fieldName]->GetSize();
				void* copy = new char[size];
				memcpy(copy, value, size);
				finalFields[fieldName] = CreateRef<Field>(fieldName, type, monoField, m_Handle);
				finalFields[fieldName]->SetValue(copy);
				delete[size] copy;
			}
			else
			{
				finalFields[fieldName] = CreateRef<Field>(fieldName, type, monoField, m_Handle);
			}

			finalFields[fieldName]->Serializable = serializable;
			finalFields[fieldName]->Hidden = hidden;
			finalFields[fieldName]->Header = header;
			finalFields[fieldName]->Tooltip = tooltip;
			finalFields[fieldName]->Min = min;
			finalFields[fieldName]->Max = max;

			m_Fields.push_back(fieldName);
		}

		m_FieldsMap = finalFields;
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
}
