using ArcEngine;

namespace Test
{
	public class Box : Entity
	{
		public float Speed = 5.0f;

		private TransformComponent tc;

		public void OnCreate()
		{
			Log.Info($"Created entity with id {ID}");
			tc = GetComponent<TransformComponent>();
		}

		public void OnUpdate(float timestep)
		{
			Vector2 pos = Input.GetMousePosition();
			Log.Info("MousePos: {0}, {1}", pos.X, pos.Y);

			Transform t = tc.Transform;
			float speed = Speed * timestep;

			if (Input.IsKeyPressed(KeyCodes.W))
				t.Translation.Y += speed;
			else if (Input.IsKeyPressed(KeyCodes.S))
				t.Translation.Y -= speed;
			if (Input.IsKeyPressed(KeyCodes.D))
				t.Translation.X += speed;
			else if (Input.IsKeyPressed(KeyCodes.A))
				t.Translation.X -= speed;

			tc.Transform = t;
		}

		public void OnDestroy()
		{
			Log.Info($"Destroyed entity with id {ID}");
		}
	}
}
