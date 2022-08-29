using ArcEngine;

namespace Sandbox
{
	public class Player : Entity
	{
		[Range(3.0f, 1000.0f)]
		[SerializeField] private float Speed = 5.0f;
		[SerializeField] private Vector2 Force = new(0.0f, 1.0f);

		private TransformComponent m_TransformComponent;
		private Rigidbody2DComponent m_Rigidbody2D;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();

			Log.Info("Created entity: {0}", GetComponent<TagComponent>().tag);
		}

		public void OnUpdate(float timestep)
		{
			if (Input.IsKeyPressed(KeyCodes.Space))
				m_Rigidbody2D.ApplyLinearImpulse(Force);

			float speed = Speed * timestep;
			Vector2 pos = m_TransformComponent.transform.Translation;
			if (Input.IsKeyPressed(KeyCodes.W))
				pos.y += speed;
			else if (Input.IsKeyPressed(KeyCodes.S))
				pos.y -= speed;
			if (Input.IsKeyPressed(KeyCodes.D))
				pos.x += speed;
			else if (Input.IsKeyPressed(KeyCodes.A))
				pos.x -= speed;
			m_Rigidbody2D.MovePosition(pos);

			/*
			Transform transform = m_TransformComponent.transform;

			if (Input.IsKeyPressed(KeyCodes.W))
				transform.Translation.y += speed;
			else if (Input.IsKeyPressed(KeyCodes.S))
				transform.Translation.y -= speed;
			if (Input.IsKeyPressed(KeyCodes.D))
				transform.Translation.x += speed;
			else if (Input.IsKeyPressed(KeyCodes.A))
				transform.Translation.x -= speed;

			m_TransformComponent.transform = transform;
			*/

			//Log.Info($"Enemy Speed: {m_Enemy.Speed}");
		}

		public void OnDestroy()
		{
			Log.Info("Destroyed entity: {0}", GetComponent<TagComponent>().tag);
		}
	}
}
