namespace ArcEngine
{
	public abstract class Component : Entity
	{
	}

	public class TagComponent : Component
	{
		public string Tag
		{
			get => InternalCalls.TagComponent_GetTag(ID);
			set => InternalCalls.TagComponent_SetTag(ID, value);
		}
	}

	public class TransformComponent : Component
	{
		public Transform Transform
		{
			get
			{
				InternalCalls.TransformComponent_GetTransform(ID, out Transform result);
				return result;
			}
			set => InternalCalls.TransformComponent_SetTransform(ID, ref value);
		}
	}
}
