using ArcEngine;

namespace Sandbox
{
	public class Enemy : Entity
	{
		public float Speed = 20.0f;
		/*
		public bool Bool = false;
		public int Int = -10;
		public uint UInt = 5;
		public string String = "FU";
		public Vector2 Vec2 = new Vector2(2.0f, 3.0f);
		public Vector3 Vec3 = new Vector3(2.0f, 3.0f, 4.0f);
		public Vector4 Vec4 = new Vector4(2.0f, 3.0f, 4.0f, 1.0f);
		*/
		private TransformComponent m_TransformComponent;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
		}

		public void OnUpdate(float timestep)
		{
			Transform transform = m_TransformComponent.Transform;

			if (Input.IsKeyPressed(KeyCodes.Q))
				transform.Rotation.Z += Speed * timestep;
			else if (Input.IsKeyPressed(KeyCodes.E))
				transform.Rotation.Z -= Speed * timestep;

			m_TransformComponent.Transform = transform;
		}
	}
}
