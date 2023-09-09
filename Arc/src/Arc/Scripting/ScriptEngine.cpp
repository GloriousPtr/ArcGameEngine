#include "arcpch.h"
#include "ScriptEngine.h"

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#include "Arc/Project/Project.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Scene.h"
#include "ProjectBuilder.h"
#include "ScriptEngineRegistry.h"

#include "Platform/VisualStudio/VisualStudioAccessor.h"

namespace ArcEngine
{
	static const eastl::hash_map<eastl::string, FieldType> s_ScriptFieldTypeMap =
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

	template<typename Output, typename... Args>
	using ManagedMethod = Output(*)(Args...);

	template<typename Output, typename... Args>
	auto GetManagedMethod(const char_t* methodName) -> ManagedMethod<Output, Args...>;

	struct Vec2
	{
		float min = 0.0f;
		float max = 0.0f;
	};

	struct ScriptEngineData
	{
		hostfxr_initialize_for_runtime_config_fn DotnetRuntimeConfig;
		hostfxr_handle DotnetRuntimeContext;
		hostfxr_get_runtime_delegate_fn RuntimeDelegate;
		load_assembly_and_get_function_pointer_fn DotnetLoadAssemblyAndFunc;
		hostfxr_close_fn DotnetClose;

		DotnetAssembly CoreAssembly = nullptr;
		DotnetAssembly ClientAssembly = nullptr;

		std::string CoreAssemblyPath;
		std::string ClientAssemblyPath;
		Scope<ScriptClass> EntityClass;

		eastl::hash_map<eastl::string, Ref<ScriptClass>> EntityClasses;
		eastl::hash_map<UUID, eastl::hash_map<eastl::string, eastl::hash_map<eastl::string, ScriptFieldInstance>>> EntityFields;

		using EntityInstanceMap = eastl::hash_map<UUID, eastl::hash_map<eastl::string, ScriptInstance*>>;
		EntityInstanceMap EntityRuntimeInstances;
	};



#define STR(x) ARC_CONCAT(L, x)
#define RegisterManagedMethod(Name, ...) ManagedMethod<__VA_ARGS__> Name = GetManagedMethod<__VA_ARGS__>(ARC_CONCAT(L, #Name))

	struct ReflectionMethods
	{
		RegisterManagedMethod(LoadAssembly, DotnetAssembly, const char*);
		RegisterManagedMethod(CreateEntityReference, GCHandle, DotnetAssembly, const char*, UUID);
		RegisterManagedMethod(CreateObjectReference, GCHandle, DotnetAssembly, const char*);
		RegisterManagedMethod(ReleaseObjectReference, void, GCHandle);
		RegisterManagedMethod(GetClassNames, char**, DotnetAssembly, DotnetAssembly, const char*);
		RegisterManagedMethod(GetMethod, DotnetMethod, GCHandle, const char*, int);
		RegisterManagedMethod(InvokeMethod, void, DotnetMethod, void**, int);
		RegisterManagedMethod(IsDebuggerAttached, int);
		RegisterManagedMethod(GetTypeFromName, DotnetType, DotnetAssembly, const char*);
		RegisterManagedMethod(GetNameFromType, char*, DotnetType);
		RegisterManagedMethod(Free, void, void*);
		RegisterManagedMethod(FreeArray, void, void*);
		RegisterManagedMethod(UnloadAssemblies, void);

		RegisterManagedMethod(GetFieldNames, char**, DotnetAssembly, const char*);
		RegisterManagedMethod(GetFieldTypeName, char*, DotnetAssembly, const char*, const char*);
		RegisterManagedMethod(GetFieldDisplayName, char*, DotnetAssembly, const char*, const char*);
		RegisterManagedMethod(GetFieldTooltip, char*, DotnetAssembly, const char*, const char*);
		RegisterManagedMethod(IsFieldPublic, int, DotnetAssembly, const char*, const char*);
		RegisterManagedMethod(FieldHasAttribute, int, DotnetAssembly, const char*, const char*, const char*);
		RegisterManagedMethod(GetFieldRange, Vec2, DotnetAssembly, const char*, const char*);
		RegisterManagedMethod(GetFieldValue, void, GCHandle, const char*, void*);
		RegisterManagedMethod(GetFieldValueString, char*, GCHandle, const char*);
		RegisterManagedMethod(SetFieldValue, void, GCHandle, const char*, const void*);
	};

	static Scope<ScriptEngineData> s_Data;
	static Scope<ReflectionMethods> s_Reflection;

	Scene* ScriptEngine::s_CurrentScene = nullptr;

	inline static eastl::hash_map<DotnetType, std::function<bool(const Entity&, DotnetType)>> s_HasComponentFuncs;
	inline static eastl::hash_map<DotnetType, std::function<void(const Entity&, DotnetType)>> s_AddComponentFuncs;
	inline static eastl::hash_map<DotnetType, std::function<GCHandle(const Entity&, DotnetType)>> s_GetComponentFuncs;
	inline static eastl::hash_map<DotnetType, eastl::string> s_ClassTypes;


	template<typename Output, typename... Args>
	auto GetManagedMethod(const char_t* methodName) -> ManagedMethod<Output, Args...>
	{
		typedef Output(CORECLR_DELEGATE_CALLTYPE* ManagedMethod)(Args...);
		ManagedMethod method = nullptr;
		static const std::filesystem::path asmPath = s_Data->CoreAssemblyPath;
		s_Data->DotnetLoadAssemblyAndFunc(asmPath.c_str(), STR("ArcEngine.AssemblyHelper, Arc-ScriptCore"), methodName, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&method);
		return method;
	}




	template<typename... Component>
	void RegisterComponent()
	{
		([]()
		{
			char* nameCstr = nullptr;
			static eastl::string componentPrefix = "ArcEngine.";
		#if defined(__clang__) || defined(__llvm__) || defined(__GNUC__) || defined(__GNUG__)
			constexpr size_t n = eastl::string_view("ArcEngine::").size();
		#elif defined(_MSC_VER)
			constexpr size_t n = eastl::string_view("struct ArcEngine::").size();
		#endif
			const std::string componentName = static_cast<std::string>(entt::type_id<Component>().name().substr(n));
			eastl::string name = componentPrefix + componentName.c_str();
			nameCstr = name.data();
			if (!nameCstr)
			{
				ARC_CORE_ASSERT(false, "Could not register component");
				return;
			}

			DotnetAssembly assembly = s_Data->CoreAssembly;
			DotnetType type = s_Reflection->GetTypeFromName(assembly, nameCstr);
			if (type)
			{
				ARC_CORE_DEBUG("Registering {}", name);
				s_HasComponentFuncs[type] = [](const Entity& entity, [[maybe_unused]] DotnetType) { return entity.HasComponent<Component>(); };
				s_AddComponentFuncs[type] = [](const Entity& entity, [[maybe_unused]] DotnetType) { entity.AddComponent<Component>(); };
			}
		}(), ...);
	}

	template<typename... Component>
	void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void RegisterScriptComponent(DotnetAssembly assembly, const eastl::string& className)
	{
		DotnetType type = s_Reflection->GetTypeFromName(assembly, className.c_str());
		if (type)
		{
			ARC_CORE_DEBUG("Registering {}", className);
			s_ClassTypes[type] = className;
			s_HasComponentFuncs[type] = [](const Entity& entity, DotnetType type) { return ScriptEngine::HasInstance(entity, s_ClassTypes.at(type)); };
			s_AddComponentFuncs[type] = [](const Entity& entity, DotnetType type) { ScriptEngine::CreateInstance(entity, s_ClassTypes.at(type)); };
			s_GetComponentFuncs[type] = [](const Entity& entity, DotnetType type) { return ScriptEngine::GetInstance(entity, s_ClassTypes.at(type))->GetHandle(); };
		}
	}


	void ScriptEngine::Init()
	{
		ARC_PROFILE_SCOPE();

		
		s_Data = CreateScope<ScriptEngineData>();
		s_Data->CoreAssemblyPath = std::filesystem::absolute("Resources/Scripts/Arc-ScriptCore.dll").string();
		

		HMODULE handleNethost{ LoadLibraryW(STR("nethost.dll")) };
		ARC_CORE_ASSERT(handleNethost, "Failed loading nethost.dll");

		auto get_hostfxr_path_handle{ reinterpret_cast<decltype (&get_hostfxr_path)>(GetProcAddress(handleNethost, "get_hostfxr_path")) };
		ARC_CORE_ASSERT(get_hostfxr_path_handle, "Failed getting address of get_hostfxr_path");

		char_t buffer[MAX_PATH];
		size_t bufferSize{ std::size(buffer) };

		[[maybe_unused]] int result{ get_hostfxr_path_handle(buffer, &bufferSize, nullptr) };
		ARC_CORE_ASSERT(result == 0, "get_hostfxr_path failed, provided buffer is too small");

		HMODULE handleHostFxr{ LoadLibraryW(buffer) };
		ARC_CORE_ASSERT(handleHostFxr, "Failed loading module");

		s_Data->DotnetRuntimeConfig = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(GetProcAddress(handleHostFxr, "hostfxr_initialize_for_runtime_config"));
		ARC_CORE_ASSERT(s_Data->DotnetRuntimeConfig, "Failed getting address of hostfxr_initialize_for_runtime_config_fn");

		s_Data->RuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(GetProcAddress(handleHostFxr, "hostfxr_get_runtime_delegate"));
		ARC_CORE_ASSERT(s_Data->RuntimeDelegate, "Failed getting address of hostfxr_get_runtime_delegate");

		s_Data->DotnetClose = reinterpret_cast<hostfxr_close_fn>(GetProcAddress(handleHostFxr, "hostfxr_close"));
		ARC_CORE_ASSERT(s_Data->DotnetClose, "Failed getting address of hostfxr_close");

		const std::filesystem::path configPath = "Resources/Scripts/Arc-ScriptCore.runtimeconfig.json";

		result = s_Data->DotnetRuntimeConfig(configPath.c_str(), nullptr, &s_Data->DotnetRuntimeContext);
		if (result != 0 || s_Data->DotnetRuntimeContext == nullptr)
		{
			s_Data->DotnetClose(s_Data->DotnetRuntimeContext);
			ARC_CORE_ASSERT(false, "Runtime initialization failed ");
		}

		result = s_Data->RuntimeDelegate(s_Data->DotnetRuntimeContext, hdt_load_assembly_and_get_function_pointer, (void**)&s_Data->DotnetLoadAssemblyAndFunc);
		if (result != 0 || s_Data->DotnetRuntimeContext == nullptr || s_Data->DotnetLoadAssemblyAndFunc == nullptr)
		{
			s_Data->DotnetClose(s_Data->DotnetRuntimeContext);
			ARC_CORE_ASSERT(false, "Failed loading the runtime");
		}

		s_Reflection = CreateScope<ReflectionMethods>();

		s_Data->DotnetClose(s_Data->DotnetRuntimeContext);
		s_Data->DotnetRuntimeContext = nullptr;

		LoadAssemblyHelper();
		ReloadAppDomain();
	}

	void ScriptEngine::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		s_Data->EntityClasses.clear();
		s_Data->EntityFields.clear();
		s_Data->EntityRuntimeInstances.clear();

		if (s_Reflection->UnloadAssemblies)
			s_Reflection->UnloadAssemblies();

		s_Reflection.reset();
		s_Data.reset();
	}

	void ScriptEngine::LoadAssemblyHelper()
	{
		ARC_PROFILE_SCOPE();
		
	}

	void ScriptEngine::LoadCoreAndClientAssembly()
	{
		ARC_PROFILE_SCOPE();
		
		const auto project = Project::GetActive();
		if (!project)
			return;

		s_Data->ClientAssemblyPath = std::filesystem::absolute(Project::GetScriptModuleDirectory() / (Project::GetActive()->GetConfig().Name + ".dll").c_str()).string();

		s_Data->EntityClasses.clear();

		ScriptEngineRegistry::Init();
		s_ClassTypes.clear();
		s_HasComponentFuncs.clear();
		s_GetComponentFuncs.clear();
		s_AddComponentFuncs.clear();

		s_Data->CoreAssembly = s_Reflection->LoadAssembly(s_Data->CoreAssemblyPath.c_str());
		s_Data->EntityClass = CreateScope<ScriptClass>(s_Data->CoreAssembly, "ArcEngine.Entity");

		s_Data->ClientAssembly = s_Reflection->LoadAssembly(s_Data->ClientAssemblyPath.c_str());
		LoadAssemblyClasses(s_Data->ClientAssembly);

		RegisterComponent<TagComponent>();
		RegisterComponent(AllComponents{});
		const auto& scripts = ScriptEngine::GetClasses();
		RegisterScriptComponent(s_Data->CoreAssembly, "ArcEngine.Entity");
		for (const auto& [className, _] : scripts)
			RegisterScriptComponent(s_Data->ClientAssembly, className);
	}

	void ScriptEngine::ReloadAppDomain(bool asyncBuild)
	{
		ARC_PROFILE_SCOPE();

		if (!Project::GetActive())
			return;

		auto begin = std::chrono::high_resolution_clock::now();
		ProjectBuilder::GenerateProjectFiles([asyncBuild, begin](bool generated)
		{
			if (generated)
			{
				ProjectBuilder::BuildProject(asyncBuild, [begin](bool success)
				{
					auto end = std::chrono::high_resolution_clock::now();
					auto timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

					if (success)
					{
						ARC_CORE_INFO("Build Suceeded; Compile time: {0}ms", timeTaken);

						if (s_Reflection->UnloadAssemblies)
							s_Reflection->UnloadAssemblies();

						LoadCoreAndClientAssembly();
					}
					else
					{
						ARC_CORE_ERROR("Build Failed; Compile time: {0}ms", timeTaken);
					}
				});
			}
		});
	}

	void ScriptEngine::LoadAssemblyClasses(DotnetAssembly assembly)
	{
		ARC_PROFILE_SCOPE();

		char** classes = s_Reflection->GetClassNames(assembly, s_Data->CoreAssembly, "ArcEngine.Entity");
		if (classes)
		{
			int i = 0;
			while (classes[i])
			{
				s_Data->EntityClasses[classes[i]] = CreateRef<ScriptClass>(assembly, classes[i], true);
				++i;
			}
			s_Reflection->FreeArray(classes);
		}
	}

	bool ScriptEngine::HasComponent(Entity entity, DotnetType type)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(s_HasComponentFuncs.find(type) != s_HasComponentFuncs.end());
		return s_HasComponentFuncs.at(type)(entity, type);
	}

	void ScriptEngine::AddComponent(Entity entity, DotnetType type)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(s_AddComponentFuncs.find(type) != s_AddComponentFuncs.end());
		s_AddComponentFuncs.at(type)(entity, type);
	}

	GCHandle ScriptEngine::GetComponent(Entity entity, DotnetType type)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(s_HasComponentFuncs.find(type) != s_HasComponentFuncs.end());
		ARC_CORE_ASSERT(s_GetComponentFuncs.find(type) != s_GetComponentFuncs.end());
		ARC_CORE_ASSERT(HasInstance(entity, s_ClassTypes.at(type)));

		return s_GetComponentFuncs.at(type)(entity, type);
	}

	void ScriptEngine::AttachDebugger()
	{
		if (!IsDebuggerAttached())
			VisualStudioAccessor::AttachDebugger();
	}

	void ScriptEngine::DetachDebugger()
	{
		if (IsDebuggerAttached())
			VisualStudioAccessor::DetachDebugger();
	}

	bool ScriptEngine::IsDebuggerAttached()
	{
		ARC_PROFILE_SCOPE();

		if (!s_Reflection->IsDebuggerAttached)
			return false;

		return s_Reflection->IsDebuggerAttached() == 0 ? false : true;
	}

	ScriptInstance* ScriptEngine::CreateInstance(Entity entity, const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		const auto& scriptClass = s_Data->EntityClasses.at(name);
		const UUID entityID = entity.GetUUID();
		auto* instance = new ScriptInstance(scriptClass, entityID);
		s_Data->EntityRuntimeInstances[entityID][name] = instance;
		return instance;
	}

	bool ScriptEngine::HasInstance(Entity entity, const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		return s_Data->EntityRuntimeInstances[entity.GetUUID()].find(name) != s_Data->EntityRuntimeInstances[entity.GetUUID()].end();
	}

	ScriptInstance* ScriptEngine::GetInstance(Entity entity, const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		return s_Data->EntityRuntimeInstances.at(entity.GetUUID()).at(name);
	}

	bool ScriptEngine::HasClass(const eastl::string& className)
	{
		ARC_PROFILE_SCOPE();

		return s_Data->EntityClasses.find(className) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::RemoveInstance(Entity entity, const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		delete s_Data->EntityRuntimeInstances.at(entity.GetUUID()).at(name);
		s_Data->EntityRuntimeInstances[entity.GetUUID()].erase(name);
	}

	const eastl::vector<eastl::string>& ScriptEngine::GetFields(const char* className)
	{
		return s_Data->EntityClasses.at(className)->GetFields();
	}

	const eastl::hash_map<eastl::string, ScriptField>& ScriptEngine::GetFieldMap(const char* className)
	{
		return s_Data->EntityClasses.at(className)->GetFieldsMap();
	}

	eastl::hash_map<eastl::string, ScriptFieldInstance>& ScriptEngine::GetFieldInstanceMap(Entity entity, const char* className)
	{
		return s_Data->EntityFields[entity.GetUUID()][className];
	}

	eastl::hash_map<eastl::string, Ref<ScriptClass>>& ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	////////////////////////////////////////////////////////////////////////
	// Script Class ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	ScriptClass::ScriptClass(DotnetAssembly assembly, const eastl::string_view classname, bool loadFields)
		: m_Assembly(assembly), m_Classname(classname)
	{
		ARC_PROFILE_SCOPE();

		if (loadFields)
			LoadFields();
	}

	DotnetMethod ScriptClass::GetMethod(GCHandle object, const char* methodName, int parameterCount) const
	{
		ARC_PROFILE_SCOPE();

		return s_Reflection->GetMethod(object, methodName, parameterCount);
	}

	void ScriptClass::GetFieldValue(GCHandle instance, const eastl::string_view fieldName, void* value) const
	{
		s_Reflection->GetFieldValue(instance, fieldName.data(), value);
	}

	void ScriptClass::SetFieldValue(GCHandle instance, const eastl::string_view fieldName, const void* value) const
	{
		s_Reflection->SetFieldValue(instance, fieldName.data(), value);
	}

	std::string ScriptClass::GetFieldValueString(GCHandle instance, const eastl::string_view fieldName) const
	{
		ARC_PROFILE_SCOPE();
		auto string = s_Reflection->GetFieldValueString(instance, fieldName.data());
		std::string str = string ? string : "";
		if (string)
			s_Reflection->Free(string);

		return str;
	}

	void ScriptClass::LoadFields()
	{
		ARC_PROFILE_SCOPE();

		m_Fields.clear();
		m_FieldsMap.clear();

		const char* className = m_Classname.c_str();

		char** fields = s_Reflection->GetFieldNames(m_Assembly, m_Classname.c_str());
		if (fields)
		{
			GCHandle obj = s_Reflection->CreateObjectReference(m_Assembly, m_Classname.c_str());
			int i = 0;
			while (fields[i] != nullptr)
			{
				eastl::string fieldName = fields[i];

				FieldType type = FieldType::Unknown;

				auto typeName = s_Reflection->GetFieldTypeName(m_Assembly, className, fieldName.c_str());
				if (!typeName)
				{
					ARC_CORE_WARN("Unsupported Field Type Name: TypeName is null");
					++i;
					continue;
				}

				const auto& fieldIt = s_ScriptFieldTypeMap.find_as(typeName);
				if (fieldIt != s_ScriptFieldTypeMap.end())
					type = fieldIt->second;
				s_Reflection->Free(typeName);

				if (type == FieldType::Unknown)
				{
					ARC_CORE_WARN("Unsupported Field Type Name: {}", typeName);
					++i;
					continue;
				}

				bool isPublic = s_Reflection->IsFieldPublic(m_Assembly, className, fieldName.c_str());
				bool serializable = isPublic || s_Reflection->FieldHasAttribute(m_Assembly, className, fieldName.c_str(), "ArcEngine.SerializeFieldAttribute");
				bool hidden = !serializable;
				if (s_Reflection->FieldHasAttribute(m_Assembly, className, fieldName.c_str(), "ArcEngine.HideInPropertiesAttribute"))
					hidden = true;
				else if (s_Reflection->FieldHasAttribute(m_Assembly, className, fieldName.c_str(), "ArcEngine.ShowInPropertiesAttribute"))
					hidden = false;

				auto header = s_Reflection->GetFieldDisplayName(m_Assembly, className, fieldName.c_str());
				auto tooltip = s_Reflection->GetFieldTooltip(m_Assembly, className, fieldName.c_str());
				Vec2 rangeMinMax = s_Reflection->GetFieldRange(m_Assembly, className, fieldName.c_str());

				auto& scriptField = m_FieldsMap[fieldName];
				scriptField.Name = fieldName;
				if (scriptField.Name.size() > 1 && scriptField.Name[0] == '_')
					scriptField.DisplayName = &scriptField.Name[1];
				else if (scriptField.Name.size() > 2 && scriptField.Name[1] == '_')
					scriptField.DisplayName = &scriptField.Name[2];
				else
					scriptField.DisplayName = scriptField.Name;
				scriptField.Type = type;
				scriptField.Serializable = serializable;
				scriptField.Hidden = hidden;
				scriptField.Header = header ? header : "";
				scriptField.Tooltip = tooltip ? tooltip : "";
				scriptField.Min = rangeMinMax.min;
				scriptField.Max = rangeMinMax.max;

				if (type == FieldType::String)
				{
					auto string = s_Reflection->GetFieldValueString(obj, fieldName.c_str());
					eastl::string str = string ? string : "";
					if (string)
						s_Reflection->Free(string);
					memcpy(scriptField.DefaultValue, str.data(), sizeof(scriptField.DefaultValue));
				}
				else
				{
					s_Reflection->GetFieldValue(obj, fieldName.c_str(), scriptField.DefaultValue);
				}

				m_Fields.emplace_back(fieldName);

				if (header)
					s_Reflection->Free(header);
				if (tooltip)
					s_Reflection->Free(tooltip);

				++i;
			}
			s_Reflection->ReleaseObjectReference(obj);
			s_Reflection->FreeArray(fields);
		}
	}
	////////////////////////////////////////////////////////////////////////
	// Script Instance /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	ScriptInstance::ScriptInstance(const Ref<ScriptClass>& scriptClass, UUID entityID)
		: m_ScriptClass(scriptClass)
	{
		ARC_PROFILE_SCOPE();

		m_Handle = s_Reflection->CreateEntityReference(scriptClass->m_Assembly, scriptClass->m_Classname.c_str(), entityID);

		auto& fieldsMap = s_Data->EntityFields[entityID][scriptClass->m_Classname];
		for (const auto& [fieldName, _] : scriptClass->m_FieldsMap)
		{
			const auto& fieldIt = fieldsMap.find(fieldName);
			if (fieldIt != fieldsMap.end())
			{
				const ScriptFieldInstance& fieldInstance = fieldIt->second;
				SetFieldValueInternal(fieldName, fieldInstance.GetBuffer());
			}
		}

		m_OnCreateMethod = scriptClass->GetMethod(m_Handle, "OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod(m_Handle, "OnUpdate", 1);
		m_OnDestroyMethod = scriptClass->GetMethod(m_Handle, "OnDestroy", 0);

		m_OnCollisionEnter2DMethod = s_Data->EntityClass->GetMethod(m_Handle, "HandleOnCollisionEnter2D", 1);
		m_OnCollisionExit2DMethod = s_Data->EntityClass->GetMethod(m_Handle, "HandleOnCollisionExit2D", 1);
		m_OnSensorEnter2DMethod = s_Data->EntityClass->GetMethod(m_Handle, "HandleOnSensorEnter2D", 1);
		m_OnSensorExit2DMethod = s_Data->EntityClass->GetMethod(m_Handle, "HandleOnSensorExit2D", 1);
	}

	ScriptInstance::~ScriptInstance()
	{
		ARC_PROFILE_SCOPE();

		s_Reflection->ReleaseObjectReference(m_Handle);
	}

	void ScriptInstance::InvokeOnCreate() const
	{
		ARC_PROFILE_SCOPE();

		if (m_OnCreateMethod)
			s_Reflection->InvokeMethod(m_OnCreateMethod, nullptr, 0);
	}

	void ScriptInstance::InvokeOnUpdate(float ts) const
	{
		ARC_PROFILE_SCOPE();

		if (m_OnUpdateMethod)
		{
			void* params = &ts;
			s_Reflection->InvokeMethod(m_OnUpdateMethod, &params, 1);
		}
	}

	void ScriptInstance::InvokeOnDestroy() const
	{
		ARC_PROFILE_SCOPE();

		if (m_OnDestroyMethod)
			s_Reflection->InvokeMethod(m_OnDestroyMethod, nullptr, 0);
	}

	void ScriptInstance::InvokeOnCollisionEnter2D(Collision2DData& other) const
	{
		ARC_PROFILE_SCOPE();

		void* param = &other;
		s_Reflection->InvokeMethod(m_OnCollisionEnter2DMethod, &param, 1);
	}

	void ScriptInstance::InvokeOnCollisionExit2D(Collision2DData& other) const
	{
		ARC_PROFILE_SCOPE();
		
		void* param = &other;
		s_Reflection->InvokeMethod(m_OnCollisionExit2DMethod, &param, 1);
	}

	void ScriptInstance::InvokeOnSensorEnter2D(Collision2DData& other) const
	{
		ARC_PROFILE_SCOPE();

		void* param = &other;
		s_Reflection->InvokeMethod(m_OnSensorEnter2DMethod, &param, 1);
	}

	void ScriptInstance::InvokeOnSensorExit2D(Collision2DData& other) const
	{
		ARC_PROFILE_SCOPE();
		
		void* param = &other;
		s_Reflection->InvokeMethod(m_OnSensorExit2DMethod, &param, 1);
	}

    GCHandle ScriptInstance::GetHandle() const
    {
		return m_Handle;
    }

    void ScriptInstance::GetFieldValueInternal(const eastl::string& name, void* value) const
	{
		ARC_PROFILE_SCOPE();

		m_ScriptClass->GetFieldValue(m_Handle, name, value);
	}

	void ScriptInstance::SetFieldValueInternal(const eastl::string& name, const void* value) const
	{
		ARC_PROFILE_SCOPE();

		m_ScriptClass->SetFieldValue(m_Handle, name, value);
	}

	std::string ScriptInstance::GetFieldValueStringInternal(const eastl::string& name) const
	{
		ARC_PROFILE_SCOPE();

		return m_ScriptClass->GetFieldValueString(m_Handle, name);
	}
}
