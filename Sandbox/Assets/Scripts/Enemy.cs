using ArcEngine;

namespace Sandbox
{
	public class Enemy : Entity
	{
		public float Speed = 20.0f;

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
