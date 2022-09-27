using ArcEngine;

namespace Sandbox
{
	class Camera : Entity
	{
		[Range(3.0f, 1000.0f)]
		[SerializeField]
		private float Speed = 5.0f;

		private TransformComponent m_TransformComponent;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
		}

		public void OnUpdate(float timestep)
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
			Transform t = m_TransformComponent.transform;
			t.Translation.x += velocity.x;
			t.Translation.y += velocity.y;
			m_TransformComponent.transform = t;
		}
	}
}
