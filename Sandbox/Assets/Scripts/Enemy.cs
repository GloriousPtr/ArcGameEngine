using ArcEngine;

namespace Sandbox
{
	public class Enemy : Entity
	{
		[Header("Player Config")]
		[Tooltip("Go Brrrr.............")]
		[SerializeField] private float Speed = 20.0f;

		[Header("Test")]
		public bool Bool = true;
		public byte Byte = 255;
		public sbyte SByte = 127;
		public short Short = -25600;
		public ushort UShort = 50600;
		public int Int = -10;
		public uint UInt = 5;
		public long Long = -5000000000000000000L;
		public ulong ULong = 5000000000000000000UL;
		[Range(0.0f, 300.0f)]
		public float Float = 69.69f;
		public double Double = 6969.69;
		public string String = "Arc En";
		public char Char = 'E';
		public Vector2 Vec2 = new Vector2(2.0f, 3.0f);
		public Vector3 Vec3 = new Vector3(2.0f, 3.0f, 4.0f);
		[SerializeField]
		private Vector4 Vec4 = new Vector4(2.0f, 3.0f, 4.0f, 1.0f);
		public Color EnemyColor = new Color(1.0f, 0.0f, 1.0f, 1.0f);

		private TransformComponent m_TransformComponent;

		public void OnCreate()
		{
			m_TransformComponent = GetComponent<TransformComponent>();

			OnSensorEnter2D += (CollisionData data) =>
			{
				Log.Info(string.Format("Sensor Enter 2D: {0}, other: {1}", GetTag(), data.entity.GetTag()));
			};

			OnSensorExit2D += (CollisionData data) =>
			{
				Log.Info(string.Format("Sensor Exit 2D: {0}, other: {1}", GetTag(), data.entity.GetTag()));
			};

			OnCollisionEnter2D += (CollisionData data) =>
			{
				Log.Info(string.Format("Collision Enter 2D: {0}, other: {1}", GetTag(), data.entity.GetTag()));
			};

			OnCollisionExit2D += (CollisionData data) =>
			{
				Log.Info(string.Format("Collision Exit 2D: {0}, other: {1}", GetTag(), data.entity.GetTag()));
			};
		}

		public void OnUpdate(float timestep)
		{
			Transform transform = m_TransformComponent.transform;

			if (Input.IsKeyPressed(KeyCodes.Q))
				transform.Rotation.z += Speed * timestep;
			else if (Input.IsKeyPressed(KeyCodes.E))
				transform.Rotation.z -= Speed * timestep;

			m_TransformComponent.transform = transform;
		}
	}
}
