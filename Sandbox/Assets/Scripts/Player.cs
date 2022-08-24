using ArcEngine;

namespace Sandbox
{
	public class Player : Entity
	{
		public float Speed = 5.0f;
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
		private Enemy m_Enemy;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();
			Log.Info("Created entity: {0}", GetComponent<TagComponent>().Tag);
			m_Enemy = GetComponent<Enemy>();
			/*
			Log.Info("Bool: {0}", Bool);
			Log.Info("Speed: {0}", Speed);
			Log.Info("Int: {0}", Int);
			Log.Info("UInt: {0}", UInt);
			Log.Info("String: {0}", String);
			Log.Info("Vec2: {0}", Vec2);
			Log.Info("Vec3: {0}", Vec3);
			Log.Info("Vec4: {0}", Vec4);
			*/
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

			Log.Info($"Enemy Speed: {m_Enemy.Speed}");
		}

		public void OnDestroy()
		{
			Log.Info("Destroyed entity: {0}", GetComponent<TagComponent>().Tag);
		}
	}
}
