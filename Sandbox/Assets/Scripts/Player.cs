using ArcEngine;

namespace Sandbox
{
	public class Player : Entity
	{
		public float Speed = 5.0f;

		private TransformComponent m_TransformComponent;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
			Log.Info($"Created entity: {0}", GetComponent<TagComponent>().Tag);
		}

		public void OnUpdate(float timestep)
		{
			Transform transform = m_TransformComponent.Transform;
			float speed = Speed * timestep;

			if (Input.IsKeyPressed(KeyCodes.W))
				transform.Translation.Y += speed;
			else if (Input.IsKeyPressed(KeyCodes.S))
				transform.Translation.Y -= speed;
			if (Input.IsKeyPressed(KeyCodes.D))
				transform.Translation.X += speed;
			else if (Input.IsKeyPressed(KeyCodes.A))
				transform.Translation.X -= speed;

			m_TransformComponent.Transform = transform;
		}

		public void OnDestroy()
		{
			Log.Info($"Destroyed entity: {0}", GetComponent<TagComponent>().Tag);
		}
	}
}
