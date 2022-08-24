using System;
using System.Runtime.InteropServices;

namespace ArcEngine
{
	public abstract class Entity
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

		public bool HasComponent<T>() where T : Entity, new()
		{
			return InternalCalls.Entity_HasComponent(ID, typeof(T));
		}

		public T AddComponent<T>() where T : Entity, new()
		{
			InternalCalls.Entity_AddComponent(ID, typeof(T));
			T component = new T();
			component.ID = ID;
			return component;
		}

		public T GetComponent<T>() where T : Entity, new()
		{
			if (HasComponent<T>())
			{
				if (typeof(T).BaseType == typeof(Component))
				{
					T component = new T();
					component.ID = ID;
					return component;
				}
				else
				{
					InternalCalls.Entity_GetComponent(ID, typeof(T), out IntPtr gcHandle);
					GCHandle gch = GCHandle.FromIntPtr(gcHandle);
					return (T)gch.Target;
				}
			}

			return null;
		}
	}
}
