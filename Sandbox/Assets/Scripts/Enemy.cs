using ArcEngine;

namespace Sandbox
{
	public class Enemy : Entity
	{
		public float Speed = 20.0f;
		
		public bool Bool = false;
		public byte Byte = 255;
		public sbyte SByte = 127;
		public short Short = -25600;
		public ushort UShort = 50600;
		public int Int = -10;
		public uint UInt = 5;
		public long Long = -5000000000000000000L;
		public ulong ULong = 5000000000000000000UL;
		public string String = "Arc Engine";
		public char Char = 'E';
		public Vector2 Vec2 = new(2.0f, 3.0f);
		public Vector3 Vec3 = new(2.0f, 3.0f, 4.0f);
		public Vector4 Vec4 = new(2.0f, 3.0f, 4.0f, 1.0f);
		
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
