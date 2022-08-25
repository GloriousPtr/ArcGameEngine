using System;
using System.Runtime.InteropServices;

namespace ArcEngine
{
	public abstract class Entity : IComponent
	{
		protected Entity()
		{
			ID = 0;
		}

		private Entity(ulong id)
		{
			ID = id;
			Log.Info("Setting ID: {0}", id);
		}

		internal ulong ID { get; private set; }

		public bool HasComponent<T>() where T : IComponent, new()
		{
			return InternalCalls.Entity_HasComponent(ID, typeof(T));
		}

		public T AddComponent<T>() where T : IComponent, new()
		{
			if (HasComponent<T>())
			{
				Log.Error($"{typeof(T).Name} already exists on Entity: {ID}");
				return GetComponent<T>();
			}

			InternalCalls.Entity_AddComponent(ID, typeof(T));
			T component = new T();
			component.SetEntity(this);
			return component;
		}

		public T GetComponent<T>() where T : IComponent, new()
		{
			if (HasComponent<T>())
			{
				if (typeof(T).BaseType == typeof(Component))
				{
					T component = new T();
					component.SetEntity(this);
					return component;
				}
				else
				{
					InternalCalls.Entity_GetComponent(ID, typeof(T), out IntPtr gcHandle);
					GCHandle gch = GCHandle.FromIntPtr(gcHandle);
					return (T)gch.Target;
				}
			}

			return new T();
		}

		ulong IComponent.GetEntityID()
		{
			return ID;
		}

		void IComponent.SetEntity(Entity e)
		{
			ID = e.ID;
		}
	}
}
