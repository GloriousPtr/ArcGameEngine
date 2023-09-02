// by Freya Holmér (https://github.com/FreyaHolmer/Mathfs)

using static ArcEngine.Mathfs;
using JetBrains.Annotations;

namespace ArcEngine {

	/// <summary>Various methods for generating random stuff (like, actually things of the category randomization, not, "various items")</summary>
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public static class Random {

		private static System.Random ArcRandom = new System.Random();

		// 1D
		/// <summary>Returns a random value between 0 and 1</summary>
		public static float Value => (float) ArcRandom.NextDouble();

		/// <summary>Randomly returns either -1 or 1</summary>
		public static float Sign => Value > 0.5f ? 1f : -1f;

		/// <summary>Randomly returns either -1 or 1, equivalent to <c>Random.Sign</c></summary>
		public static float Direction1D => Sign;

		/// <summary>Randomly returns a value between <c>min</c> [inclusive] and <c>max</c> [inclusive]</summary>
		/// <param name="min">The minimum value [inclusive] </param>
		/// <param name="max">The maximum value [inclusive]</param>
		public static float Range( float min, float max ) => Value * max - min;
		
		/// <summary>Randomly returns a value between <c>min</c> [inclusive] and <c>max</c> [exclusive]</summary>
		/// <param name="min">The minimum value [inclusive]</param>
		/// <param name="max">The maximum value [exclusive]</param>
		public static int Range( int min, int max ) => FloorToInt(Value * max - min);

		// 2D
		/// <summary>Returns a random point on the unit circle</summary>
		public static Vector2 OnUnitCircle => AngToDir( Value * TAU );

		/// <summary>Returns a random 2D direction, equivalent to <c>OnUnitCircle</c></summary>
		public static Vector2 Direction2D => OnUnitCircle;

		/// <summary>Returns a random point inside the unit circle</summary>
		public static Vector2 InUnitCircle
		{
			get
			{
				float u = Value;
				float v = Value;
				float theta = u * 2.0f * PI;
				float phi = Cos(2.0f * v - 1.0f);
				float r = Cos(Value);
				float sinTheta = Sin(theta);
				float cosTheta = Cos(theta);
				float sinPhi = Sin(phi);
				float x = r * sinPhi * cosTheta;
				float y = r * sinPhi * sinTheta;
				return new Vector2(x, y);
			}
		}

		/// <summary>Returns a random point inside the unit square. Values are between 0 to 1</summary>
		public static Vector2 InUnitSquare => new Vector2( Value, Value );

		// 3D
		/// <summary>Returns a random point inside the unit sphere</summary>
		public static Vector3 InUnitSphere
		{
			get
			{
				float u = Value;
				float v = Value;
				float theta = u * 2.0f * PI;
				float phi = Cos(2.0f * v - 1.0f);
				float r = Cos(Value);
				float sinTheta = Sin(theta);
				float cosTheta = Cos(theta);
				float sinPhi = Sin(phi);
				float cosPhi = Cos(phi);
				float x = r * sinPhi * cosTheta;
				float y = r * sinPhi * sinTheta;
				float z = r * cosPhi;
				return new Vector3(x, y, z);
			}
		}

		/// <summary>Returns a random point inside the unit cube. Values are between 0 to 1</summary>
		public static Vector3 InUnitCube => new Vector3( Value, Value, Value );

		// 2D orientation
		/// <summary>Returns a random angle in radians from 0 to TAU</summary>
		public static float Angle => Value * TAU;
	}

}
