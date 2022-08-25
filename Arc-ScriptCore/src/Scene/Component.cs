namespace ArcEngine
{
	public interface IComponent
	{
		internal ulong GetEntityID();
		internal void SetEntity(Entity e);
	}

	public abstract class Component : IComponent
	{
		internal Entity entity;

		ulong IComponent.GetEntityID() => entity.ID;

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
