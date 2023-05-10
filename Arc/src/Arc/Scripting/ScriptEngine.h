#pragma once

#include "Arc/Core/UUID.h"
#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	class Entity;
	class Scene;

	using DotnetAssembly	= void*;
	using DotnetMethod		= void*;
	using DotnetType		= void*;
	using DotnetField		= void*;
	using GCHandle			= void*;

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
		std::string Name;
		std::string DisplayName;
		FieldType Type = FieldType::Unknown;

		// For attributes
		bool Serializable = true;
		bool Hidden = false;
		std::string Header;
		std::string Tooltip;
		float Min = 0.0f;
		float Max = 0.0f;

		char DefaultValue[16] = {};

		ScriptField()
		{
			memset(DefaultValue, 0, sizeof(DefaultValue));
		}

		template<typename T>
		[[nodiscard]] T GetDefaultValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large");
			return *reinterpret_cast<const T*>(DefaultValue);
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
		[[nodiscard]] T GetValue() const
		{
			static_assert(sizeof(T) <= MaxSize, "Type too large");
			return *reinterpret_cast<const T*>(m_Buffer);
		}

		template<typename T>
		void SetValue(const T& value)
		{
			static_assert(sizeof(T) <= MaxSize, "Type too large");
			memcpy(m_Buffer, &value, sizeof(T));
		}

		void SetValueString(std::string_view value)
		{
			if (value.size() < MaxSize)
				memcpy(m_Buffer, value.data(), MaxSize);
		}

		[[nodiscard]] const void* GetBuffer() const { return m_Buffer; }

	private:
		char m_Buffer[MaxSize] = {};
	};

	class ScriptClass
	{
	public:
		ScriptClass() = delete;
		explicit ScriptClass(DotnetAssembly assembly, const std::string_view classname, bool loadFields = false);
		
		ScriptClass(const ScriptClass& other) = delete;
		ScriptClass(ScriptClass&& other) = delete;

		[[nodiscard]] DotnetMethod GetMethod(GCHandle object, const char* methodName, int parameterCount) const;
		void GetFieldValue(GCHandle instance, const std::string_view fieldName, void* value) const;
		void SetFieldValue(GCHandle instance, const std::string_view fieldName, const void* value) const;
		[[nodiscard]] std::string GetFieldValueString(GCHandle instance, const std::string_view fieldName) const;

		[[nodiscard]] const std::vector<std::string>& GetFields() const { return m_Fields; }
		[[nodiscard]] const str_umap<ScriptField>& GetFieldsMap() const { return m_FieldsMap; }

	private:
		void LoadFields();

	private:
		friend class ScriptEngine;
		friend class ScriptInstance;

		DotnetAssembly m_Assembly;
		std::string m_Classname{};
		std::vector<std::string> m_Fields;
		str_umap<ScriptField> m_FieldsMap;
	};

	struct Collision2DData
	{
		UUID EntityID = 0;
		glm::vec2 RelativeVelocity = { 0.0f, 0.0f };
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(const Ref<ScriptClass>& scriptClass, UUID entityID);

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
		[[nodiscard]] T GetFieldValue(const std::string& fieldName) const
		{
			T value;
			GetFieldValueInternal(fieldName, &value);
			return value;
		}

		template<typename T>
		void SetFieldValue(const std::string& fieldName, const T& value) const
		{
			SetFieldValueInternal(fieldName, &value);
		}

		[[nodiscard]] std::string GetFieldValueString(const std::string& fieldName) const
		{
			return GetFieldValueStringInternal(fieldName);
		}

		[[nodiscard]] GCHandle GetHandle() const;

	private:
		void GetFieldValueInternal(const std::string& name, void* value) const;
		void SetFieldValueInternal(const std::string& name, const void* value) const;
		[[nodiscard]] std::string GetFieldValueStringInternal(const std::string& name) const;

	private:
		Ref<ScriptClass> m_ScriptClass;

		GCHandle m_Handle = 0;
		DotnetMethod m_OnCreateMethod = nullptr;
		DotnetMethod m_OnUpdateMethod = nullptr;
		DotnetMethod m_OnDestroyMethod = nullptr;
		
		DotnetMethod m_OnCollisionEnter2DMethod = nullptr;
		DotnetMethod m_OnCollisionExit2DMethod = nullptr;
		DotnetMethod m_OnSensorEnter2DMethod = nullptr;
		DotnetMethod m_OnSensorExit2DMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();
		static void LoadAssemblyHelper();
		static void LoadCoreAndClientAssembly();
		static void ReloadAppDomain(bool asyncBuild = true);
		static void LoadAssemblyClasses(DotnetAssembly assembly);

		static void AddComponent(Entity entity, DotnetType type);
		[[nodiscard]] static bool HasComponent(Entity entity, DotnetType type);
		[[nodiscard]] static GCHandle GetComponent(Entity entity, DotnetType type);

		static void AttachDebugger();
		static void DetachDebugger();
		[[nodiscard]] static bool IsDebuggerAttached();

		[[nodiscard]] static bool HasClass(const std::string& className);
		static ScriptInstance* CreateInstance(Entity entity, const std::string& name);
		[[nodiscard]] static bool HasInstance(Entity entity, const std::string& name);
		[[nodiscard]] static ScriptInstance* GetInstance(Entity entity, const std::string& name);
		static void RemoveInstance(Entity entity, const std::string& name);
		
		[[nodiscard]] static str_umap<Ref<ScriptClass>>& GetClasses();
		[[nodiscard]] static const std::vector<std::string>& GetFields (const char* className);
		[[nodiscard]] static const str_umap<ScriptField>& GetFieldMap(const char* className);
		[[nodiscard]] static str_umap<ScriptFieldInstance>& GetFieldInstanceMap(Entity entity, const char* className);

		static void SetScene(Scene* scene) { s_CurrentScene = scene; }
		[[nodiscard]] static Scene* GetScene() { return s_CurrentScene; }

	private:
		static Scene* s_CurrentScene;
	};
}
