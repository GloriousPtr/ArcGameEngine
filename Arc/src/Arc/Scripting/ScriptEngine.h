#pragma once

#include "Arc/Core/UUID.h"

typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClass MonoClass;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoProperty MonoProperty;
typedef struct _MonoClassField MonoClassField;
typedef struct _MonoType MonoType;

namespace ArcEngine
{
	class Entity;
	class Scene;

	using GCHandle = void*;

	enum class FieldType
	{
		Unknown = 0,
		Float,
		Double,
		Bool,
		Char,
		Byte,
		Short,
		Int,
		Long,
		UByte,
		UShort,
		UInt,
		ULong,
		String,

		Vector2,
		Vector3,
		Vector4,
		Color
	};

	struct ScriptField
	{
		eastl::string Name = "";
		eastl::string DisplayName = "";
		FieldType Type = FieldType::Unknown;
		MonoClassField* Field = nullptr;

		// For attributes
		bool Serializable = true;
		bool Hidden = false;
		eastl::string Header = "";
		eastl::string Tooltip = "";
		float Min = 0.0f;
		float Max = 0.0f;

		char DefaultValue[16];

		ScriptField()
		{
			memset(DefaultValue, 0, sizeof(DefaultValue));
		}

		template<typename T>
		T GetDefaultValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large");
			return *(const T*)DefaultValue;
		}
	};

	struct ScriptFieldInstance
	{
		FieldType Type = FieldType::Unknown;
		static constexpr size_t MaxSize = 16;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= MaxSize, "Type too large");
			return *(const T*)m_Buffer;
		}

		template<typename T>
		void SetValue(const T& value)
		{
			static_assert(sizeof(T) <= MaxSize, "Type too large");
			memcpy(m_Buffer, &value, sizeof(T));
		}

		void SetValueString(eastl::string_view value)
		{
			if (value.size() < MaxSize)
				memcpy(m_Buffer, value.data(), MaxSize);
		}

		const void* GetBuffer() const { return m_Buffer; }

	private:
		char m_Buffer[MaxSize];
	};

	class ScriptClass
	{
	public:
		ScriptClass() = delete;
		explicit ScriptClass(MonoClass* monoClass);
		ScriptClass(const eastl::string& classNamespace, const eastl::string& className);
		
		ScriptClass(const ScriptClass& other) = delete;
		ScriptClass(ScriptClass&& other) = delete;

		GCHandle Instantiate();
		MonoMethod* GetMethod(const char* methodName, uint32_t parameterCount);
		GCHandle InvokeMethod(GCHandle gcHandle, MonoMethod* method, void** params = nullptr);

		const eastl::vector<eastl::string>& GetFields() const { return m_Fields; }
		const eastl::hash_map<eastl::string, ScriptField>& GetFieldsMap() const { return m_FieldsMap; }

	private:
		void LoadFields();

	private:
		friend class ScriptEngine;
		friend class ScriptInstance;

		eastl::string m_ClassNamespace;
		eastl::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
		eastl::vector<eastl::string> m_Fields;
		eastl::hash_map<eastl::string, ScriptField> m_FieldsMap;
	};

	struct Collision2DData
	{
		UUID EntityID = 0;
		glm::vec2 RelativeVelocity = { 0.0f, 0.0f };
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, UUID entityID);

		ScriptInstance(const ScriptInstance& other) = delete;
		ScriptInstance(ScriptInstance&& other) = delete;

		~ScriptInstance();

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float ts) const;
		void InvokeOnDestroy() const;
		void InvokeOnCollisionEnter2D(Collision2DData& other) const;
		void InvokeOnCollisionExit2D(Collision2DData& other) const;
		void InvokeOnSensorEnter2D(Collision2DData& other) const;
		void InvokeOnSensorExit2D(Collision2DData& other) const;

		template<typename T>
		T GetFieldValue(const eastl::string& fieldName) const
		{
			T value;
			GetFieldValueInternal(fieldName, &value);
			return value;
		}

		template<typename T>
		void SetFieldValue(const eastl::string& fieldName, const T& value) const
		{
			SetFieldValueInternal(fieldName, &value);
		}

		eastl::string GetFieldValueString(const eastl::string& fieldName) const
		{
			return GetFieldValueStringInternal(fieldName);
		}

		const GCHandle GetHandle() const;

	private:
		void GetFieldValueInternal(const eastl::string& name, void* value) const;
		void SetFieldValueInternal(const eastl::string& name, const void* value) const;
		eastl::string GetFieldValueStringInternal(const eastl::string& name) const;

	private:
		Ref<ScriptClass> m_EntityClass;
		Ref<ScriptClass> m_ScriptClass;

		GCHandle m_Handle = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;

		MonoMethod* m_OnCollisionEnter2DMethod = nullptr;
		MonoMethod* m_OnCollisionExit2DMethod = nullptr;
		MonoMethod* m_OnSensorEnter2DMethod = nullptr;
		MonoMethod* m_OnSensorExit2DMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();
		static void LoadCoreAssembly();
		static void LoadClientAssembly();
		static void ReloadAppDomain();
		static void LoadAssemblyClasses(MonoAssembly* assembly);

		static MonoDomain* GetDomain();
		static MonoImage* GetCoreAssemblyImage();
		static MonoImage* GetAppAssemblyImage();

		static bool HasClass(const eastl::string& className);
		static ScriptInstance* CreateInstance(Entity entity, const eastl::string& name);
		static bool HasInstance(Entity entity, const eastl::string& name);
		static ScriptInstance* GetInstance(Entity entity, const eastl::string& name);
		static void RemoveInstance(Entity entity, const eastl::string& name);
		
		static eastl::hash_map<eastl::string, Ref<ScriptClass>>& GetClasses();
		static const eastl::vector<eastl::string>& GetFields (const char* className);
		static const eastl::hash_map<eastl::string, ScriptField>& GetFieldMap(const char* className);
		static eastl::hash_map<eastl::string, ScriptFieldInstance>& GetFieldInstanceMap(Entity entity, const char* className);

		static void SetScene(Scene* scene) { s_CurrentScene = scene; }
		static Scene* GetScene() { return s_CurrentScene; }

	private:
		static Scene* s_CurrentScene;
	};
}
