using System.Runtime.CompilerServices;

namespace ArcEngine
{
	public interface IComponent
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal ulong GetEntityID();
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal void SetEntity(Entity e);
	}

	public abstract class Component : IComponent
	{
		internal Entity entity;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		ulong IComponent.GetEntityID() => entity.ID;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		void IComponent.SetEntity(Entity e) => entity = e;
	}

	public class TagComponent : Component
	{
		public string Tag
		{
			get => InternalCalls.TagComponent_GetTag(entity.ID);
			set => InternalCalls.TagComponent_SetTag(entity.ID, value);
		}
	}

	public class TransformComponent : Component
	{
		public Transform Transform
		{
			get
			{
				InternalCalls.TransformComponent_GetTransform(entity.ID, out Transform result);
				return result;
			}
			set => InternalCalls.TransformComponent_SetTransform(entity.ID, ref value);
		}
	}
}
