using System;

namespace ArcEngine
{
	public class Entity
	{
		internal ulong ID { get; private set; }

		public bool HasComponent<T>() where T : Component, new() => InternalCalls.Entity_HasComponent(ID, typeof(T));

		public T AddComponent<T>() where T : Component, new()
		{
			InternalCalls.Entity_AddComponent(ID, typeof(T));
			T component = new T();
			component.Entity = this;
			return component;
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (HasComponent<T>())
			{
				T component = new T();
				component.Entity = this;
				return component;
			}

			return null;
		}
	}
}
