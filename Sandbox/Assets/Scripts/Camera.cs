using ArcEngine;

namespace Sandbox
{
	class Camera : Entity
	{
		[Range(3.0f, 1000.0f)]
		[SerializeField]
		private float Speed = 5.0f;

		private TransformComponent m_TransformComponent;

		internal void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
		}

		private void OnUpdate(float timestep)
		{
			float speed = Speed * timestep;
			Vector2 dir = new Vector2(0.0f);

			if (Input.IsKeyPressed(KeyCodes.W))
				dir.y = 1.0f;
			else if (Input.IsKeyPressed(KeyCodes.S))
				dir.y = -1.0f;
			if (Input.IsKeyPressed(KeyCodes.D))
				dir.x = 1.0f;
			else if (Input.IsKeyPressed(KeyCodes.A))
				dir.x = -1.0f;

			Vector2 velocity = dir * speed;
			m_TransformComponent.translation += new Vector3(velocity.x, velocity.y, 0.0f);
		}
	}
}
