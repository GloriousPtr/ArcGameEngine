using ArcEngine;

namespace Sandbox
{
	public class Player : Entity
	{
		[Range(3.0f, 1000.0f)]
		[SerializeField]
		private float Speed = 5.0f;
		[SerializeField]
		private Vector2 Force = new(0.0f, 1.0f);

		private TransformComponent m_TransformComponent;
		private Rigidbody2DComponent m_Rigidbody2D;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
		}

		public void OnUpdate(float timestep)
		{
			/*if (Input.IsKeyPressed(KeyCodes.Space))
				m_Rigidbody2D.ApplyLinearImpulse(Force);
			*/
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
			m_Rigidbody2D.velocity = velocity;
		}
	}
}
