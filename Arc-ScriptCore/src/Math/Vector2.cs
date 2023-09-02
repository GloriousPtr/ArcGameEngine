using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("[{x}, {y}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector2 : IEquatable<Vector2>
	{
		public float x;
		public float y;

		public float magnitude => (float)Math.Sqrt(x * x + y * y);
		public float sqrMagnitude => x * x + y * y;
		public Vector2 normalized => this / magnitude;

		public static Vector2 one => new Vector2(1.0f);
		public static Vector2 zero => new Vector2(0.0f);

		public static Vector2 up		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Vector2( 0.0f,  1.0f); }
		public static Vector2 down		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Vector2( 0.0f, -1.0f); }
		public static Vector2 right		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Vector2( 1.0f,  0.0f); }
		public static Vector2 left		{ [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Vector2(-1.0f,  0.0f); }

		#region Constructors

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector2(in float scalar)
		{
			x = y = scalar;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector2(in float x, in float y)
		{
			this.x = x;
			this.y = y;
		}

		#endregion

		#region OverloadedOperators

		public float this[int index]
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				switch (index)
				{
					case 0: return x;
					case 1: return y;
					default: throw new IndexOutOfRangeException();
				}
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				switch (index)
				{
					case 0: x = value; break;
					case 1: y = value; break;
					default: throw new IndexOutOfRangeException();
				}
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator +(in Vector2 left, in Vector2 right) { return new Vector2(left.x + right.x, left.y + right.y); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator -(in Vector2 left, in Vector2 right) { return new Vector2(left.x - right.x, left.y - right.y); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator *(in Vector2 left, in Vector2 right) { return new Vector2(left.x * right.x, left.y * right.y); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator *(in Vector2 left, in float scalar) { return new Vector2(left.x * scalar, left.y * scalar); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator *(in float scalar, in Vector2 right) { return new Vector2(scalar * right.x, scalar * right.y); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator /(in Vector2 left, in Vector2 right) { return new Vector2(left.x / right.x, left.y / right.y); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 operator /(in Vector2 left, in float scalar) { return new Vector2(left.x / scalar, left.y / scalar); }

		#endregion

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector2 Normalize()
		{
			this = normalized;
			return this;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public override string ToString() { return "Vector2(" + x + ", " + y + ")"; }

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Dot(in Vector2 v1, in Vector2 v2) => v1.magnitude * v2.magnitude * Mathfs.Acos(Mathfs.AngleBetween(v1, v2));
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 Max(Vector2 v1, Vector2 v2) => new Vector2(Mathfs.Max(v1.x, v2.x), Mathfs.Max(v1.y, v2.y));
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 LerpUnclamped(Vector2 a, Vector2 b, float t) => Mathfs.Lerp(a, b, new Vector2(t));

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Equals(Vector2 other)
		{
			return x == other.x && y == other.y;
		}
	}
}
