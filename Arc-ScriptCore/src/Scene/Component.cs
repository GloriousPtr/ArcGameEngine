namespace ArcEngine
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TagComponent : Component
	{
		public string Tag
		{
			get => InternalCalls.TagComponent_GetTag(Entity.ID);
			set => InternalCalls.TagComponent_SetTag(Entity.ID, value);
		}
	}

	public class TransformComponent : Component
	{
		public Transform Transform
		{
			get
			{
				InternalCalls.TransformComponent_GetTransform(Entity.ID, out Transform result);
				return result;
			}
			set => InternalCalls.TransformComponent_SetTransform(Entity.ID, ref value);
		}
	}
}
