using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#pragma warning disable IDE0051

namespace ArcEngine
{
	public class Entity : IComponent
	{
		internal ulong ID { get; private set; }

		#region CollisionCallbacks

		/// <summary>
		/// Holds data for collision events
		/// </summary>
		[StructLayout(LayoutKind.Sequential)]
		public struct CollisionData
		{
			private ulong entityID;
			public Vector2 relativeVelocity;

			public Entity entity => new(entityID);
		}

		protected event Action<CollisionData> OnCollisionEnter2D;
		protected event Action<CollisionData> OnCollisionExit2D;
		protected event Action<CollisionData> OnSensorEnter2D;
		protected event Action<CollisionData> OnSensorExit2D;

		#endregion

		#region Constructors

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		protected Entity()
		{
			ID = 0;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private Entity(ulong id)
		{
			ID = id;
		}

		#endregion

		#region PublicMethods

		/// <summary>
		/// Check for component of Type T.
		/// </summary>
		/// <typeparam name="T">Component Type</typeparam>
		/// <returns>If component is attached to the entity: true, else false.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool HasComponent<T>() where T : class, IComponent
		{
			return InternalCalls.Entity_HasComponent(ID, typeof(T));
		}

		/// <summary>
		/// Adds the component of Type T to the entity if it's not already attached.
		/// </summary>
		/// <typeparam name="T">Component Type</typeparam>
		/// <returns>If already attached: Newly added component, else already attached component of Type T.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public T AddComponent<T>() where T : class, IComponent, new()
		{
			if (HasComponent<T>())
			{
				Log.Error("{0} already exists on Entity: {1}", typeof(T).Name, ID);
				return GetComponent<T>();
			}

			InternalCalls.Entity_AddComponent(ID, typeof(T));
			T component = new T();
			component.SetEntity(ID);
			return component;
		}

		/// <summary>
		/// Get the component of Type T.
		/// </summary>
		/// <typeparam name="T">Component Type</typeparam>
		/// <returns>Component of Type T if the game object has one attached, null if it doesn't.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public T GetComponent<T>() where T : class, IComponent, new()
		{
			if (!HasComponent<T>())
				return null;
			
			if (typeof(T).BaseType == typeof(Entity))
				return (T)InternalCalls.Entity_GetComponent(ID, typeof(T));

			T component = new T();
			component.SetEntity(ID);
			return component;
		}

		#endregion

		#region CollisionHandlingMethods

		private void HandleOnCollisionEnter2D(CollisionData data) => OnCollisionEnter2D?.Invoke(data);

		private void HandleOnCollisionExit2D(CollisionData data) => OnCollisionExit2D?.Invoke(data);

		private void HandleOnSensorEnter2D(CollisionData data) => OnSensorEnter2D?.Invoke(data);

		private void HandleOnSensorExit2D(CollisionData data) => OnSensorExit2D?.Invoke(data);

		#endregion

		#region IComponentMethods

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		ulong IComponent.GetEntityID()
		{
			return ID;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		void IComponent.SetEntity(ulong id)
		{
			ID = id;
		}

		#endregion
	}
}

#pragma warning restore IDE0051
