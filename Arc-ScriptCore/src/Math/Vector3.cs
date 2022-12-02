using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("[{x}, {y}, {z}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3 : IEquatable<Vector3>
	{
		public float x;
		public float y;
		public float z;

		public float magnitude { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return (float)Math.Sqrt(x * x + y * y + z * z); } }
		public float sqrMagnitude { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return x * x + y * y + z * z; } }
		public Vector3 normalized { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this / magnitude; } }

		public static Vector3 one		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(1.0f); } }
		public static Vector3 zero		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(0.0f); } }
		public static Vector3 forward	{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3( 0.0f,  0.0f,  1.0f); } }
		public static Vector3 backward	{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3( 0.0f,  0.0f, -1.0f); } }
		public static Vector3 up		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3( 0.0f,  1.0f,  0.0f); } }
		public static Vector3 down		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3( 0.0f, -1.0f,  0.0f); } }
		public static Vector3 right		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3( 1.0f,  0.0f,  0.0f); } }
		public static Vector3 left		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector3(-1.0f,  0.0f,  0.0f); } }

		#region Constructors

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3(in float scalar)
		{
			x = y = z = scalar;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3(in float x, in float y, in float z)
		{
			this.x = x;
			this.y = y;
			this.z = z;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3(in Vector2 xy, in float z)
		{
			x = xy.x;
			y = xy.y;
			this.z = z;
		}

		#endregion

		#region OverloadedOperators

		public float this[int index]
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				return index switch
				{
					0 => x,
					1 => y,
					2 => z,
					_ => throw new IndexOutOfRangeException(),
				};
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				switch (index)
				{
					case 0: x = value; break;
					case 1: y = value; break;
					case 2: z = value; break;
					default: throw new IndexOutOfRangeException();
				}
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator +(in Vector3 left, in Vector3 right) { return new Vector3(left.x + right.x, left.y + right.y, left.z + right.z); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator -(in Vector3 left, in Vector3 right) { return new Vector3(left.x - right.x, left.y - right.y, left.z - right.z); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator *(in Vector3 left, in Vector3 right) { return new Vector3(left.x * right.x, left.y * right.y, left.z * right.z); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator *(in Vector3 left, in float scalar) { return new Vector3(left.x * scalar, left.y * scalar, left.z * scalar); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator *(in float scalar, in Vector3 right) { return new Vector3(scalar * right.x, scalar * right.y, scalar * right.z); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator /(in Vector3 left, in Vector3 right) { return new Vector3(left.x / right.x, left.y / right.y, left.z / right.z); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator /(in Vector3 left, in float scalar) { return new Vector3(left.x / scalar, left.y / scalar, left.z / scalar); }

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static implicit operator Vector2(in Vector3 v) { return new Vector2(v.x, v.y); }

		#endregion

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3 Normalize()
		{
			this = normalized;
			return this;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public override string ToString() { return "Vector3(" + x + ", " + y + ", " + z + ")"; }

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Dot(in Vector3 v1, in Vector3 v2) => v1.magnitude * v2.magnitude * Mathfs.Acos(Mathfs.AngleBetween(v1, v2));

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Cross(in Vector3 v1, in Vector3 v2)
		{
			return new Vector3((v1.y * v2.z) - (v1.z * v2.y),
				-(v1.x * v2.z) - (v1.z * v2.x),
				(v1.x * v2.y) - (v1.y * v2.x));
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t) => Mathfs.Lerp(a, b, new Vector3(t));

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Equals(Vector3 other) => x == other.x && y == other.y && z == other.z;
	}
}
