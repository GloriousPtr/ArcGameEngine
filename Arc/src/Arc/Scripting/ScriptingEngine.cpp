#include "arcpch.h"
#include "ScriptingEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#ifdef ARC_DEBUG
#include <mono/metadata/debug-helpers.h>
#endif // ARC_DEBUG

namespace ArcEngine
{
	MonoDomain* ScriptingEngine::s_MonoDomain;
	MonoAssembly* ScriptingEngine::s_ScriptCoreAssembly;
	MonoImage* ScriptingEngine::s_ScriptCoreImage;

	std::unordered_map<std::string, MonoClass*> ScriptingEngine::s_ClassMap;
	std::unordered_map<std::string, MonoMethod*> ScriptingEngine::s_MethodMap;

	void ScriptingEngine::Init(const char* assemblyPath)
	{
		mono_set_dirs("C:/Program Files/Mono/lib",
        "C:/Program Files/Mono/etc");

		s_MonoDomain = mono_jit_init("ScriptEngine");
		ARC_CORE_ASSERT(s_MonoDomain, "Could not initialize domain");

		s_ScriptCoreAssembly = mono_domain_assembly_open(s_MonoDomain, assemblyPath);
		if (!s_ScriptCoreAssembly)
		{
			ARC_CORE_ERROR("Could not open assembly: {0}", assemblyPath);
			return;
		}

		s_ScriptCoreImage = mono_assembly_get_image(s_ScriptCoreAssembly);
		if (!s_ScriptCoreImage)
		{
			ARC_CORE_ERROR("Could not get image from assembly: {0}", assemblyPath);
			return;
		}
	}

	void ScriptingEngine::Shutdown()
	{
		s_MethodMap.clear();
		s_ClassMap.clear();

		delete s_ScriptCoreImage;
		delete s_ScriptCoreAssembly;
		delete s_MonoDomain;
	}

	void* ScriptingEngine::MakeInstance(const char* className)
	{
		if (!s_MonoDomain)
		{
			ARC_CORE_ERROR("Domain not found");
			return nullptr;
		}

		MonoClass* clazz = GetClass(className);
		if (!clazz)
			return nullptr;

		MonoObject* object = mono_object_new(s_MonoDomain, clazz);
		if (object)
			mono_runtime_object_init(object);
		
		return object;
	}

	void ScriptingEngine::Call(void* instance, const char* className, const char* methodSignature, void** args)
	{
		MonoMethod* method = GetMethod(className, methodSignature);
		if (!method)
			return;

		mono_runtime_invoke(method, instance, args, nullptr);
	}

	void ScriptingEngine::CacheMethodIfAvailable(const char* className, const char* methodSignature)
	{
		std::string desc = std::string(className) + ":" + (methodSignature);
		if (s_MethodMap.find(desc) != s_MethodMap.end())
			return;
		
		MonoMethodDesc* methodDesc = mono_method_desc_new(desc.c_str(), true);
		MonoClass* clazz = GetClass(className);
		if (!clazz)
			return;

		if (MonoMethod* method = mono_method_desc_search_in_class(methodDesc, clazz))
			s_MethodMap.emplace(desc, method);
	}

	void ScriptingEngine::CacheClassIfAvailable(const char* className)
	{
		if (!s_ScriptCoreImage)
			return;

		if (s_ClassMap.find(className) != s_ClassMap.end())
			return;

		std::string name(className);
		int length = name.size();
		int lastColon = name.find_last_of(':');
		std::string namespaceName = name.substr(0, lastColon);
		std::string clazzName = name.substr(lastColon + 1, length - lastColon);

		if (MonoClass* clazz = mono_class_from_name(s_ScriptCoreImage, namespaceName.c_str(), clazzName.c_str()))
			s_ClassMap.emplace(className, clazz);
	}

	MonoMethod* ScriptingEngine::GetCachedMethodIfAvailable(const char* className, const char* methodSignature)
	{
		std::string desc = std::string(className) + ":" + (methodSignature);
		if (s_MethodMap.find(desc) != s_MethodMap.end())
			return s_MethodMap.at(desc);
		else
			return nullptr;
	}

	MonoClass* ScriptingEngine::GetCachedClassIfAvailable(const char* className)
	{
		if (s_ClassMap.find(className) != s_ClassMap.end())
			return s_ClassMap.at(className);
		else
			return nullptr;
	}

	MonoMethod* ScriptingEngine::GetMethod(const char* className, const char* methodSignature)
	{
		std::string desc = std::string(className) + ":" + (methodSignature);
		if (s_MethodMap.find(desc) != s_MethodMap.end())
			return s_MethodMap.at(desc);
		
		MonoMethodDesc* methodDesc = mono_method_desc_new(desc.c_str(), true);
		MonoClass* clazz = GetClass(className);
		if (!clazz)
			return nullptr;
		MonoMethod* method = mono_method_desc_search_in_class(methodDesc, clazz);

		if (method)
			s_MethodMap.emplace(desc, method);
		else
			ARC_CORE_ERROR("Method not found: {0}", desc);

		return method;
	}

	bool ScriptingEngine::HasClass(const char* className)
	{
		if (!s_ScriptCoreImage)
		{
			ARC_CORE_ERROR("ScriptCore Image not found!");
			return false;
		}

		if (s_ClassMap.find(className) != s_ClassMap.end())
			return true;

		std::string name(className);
		int length = name.size();
		int lastColon = name.find_last_of(':');
		std::string namespaceName = name.substr(0, lastColon);
		std::string clazzName = name.substr(lastColon + 1, length - lastColon);

		MonoClass* clazz = mono_class_from_name(s_ScriptCoreImage, namespaceName.c_str(), clazzName.c_str());
		return clazz != nullptr;
	}

	MonoClass* ScriptingEngine::GetClass(const char* className)
	{
		if (!s_ScriptCoreImage)
		{
			ARC_CORE_ERROR("ScriptCore Image not found!");
			return nullptr;
		}

		if (s_ClassMap.find(className) != s_ClassMap.end())
			return s_ClassMap.at(className);

		std::string name(className);
		int length = name.size();
		int lastColon = name.find_last_of(':');
		std::string namespaceName = name.substr(0, lastColon);
		std::string clazzName = name.substr(lastColon + 1, length - lastColon);

		MonoClass* clazz = mono_class_from_name(s_ScriptCoreImage, namespaceName.c_str(), clazzName.c_str());
		if (clazz)
			s_ClassMap.emplace(className, clazz);
		else
			ARC_CORE_ERROR("Class not found: {0}", className);

		return clazz;
	}
}
