using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("[{x}, {y}, {z}, {w}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector4 : IEquatable<Vector4>
	{
		public float x;
		public float y;
		public float z;
		public float w;

		public float magnitude { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return (float) Math.Sqrt(x* x + y* y + z* z + w* w); } }
		public float sqrMagnitude { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return x * x + y * y + z * z + w * w; } }
		public Vector4 normalized { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this / magnitude; } }

		public static Vector4 one { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector4(1.0f); } }
		public static Vector4 zero { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector4(0.0f); } }

		#region Constructors

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in float scalar)
		{
			x = y = z = w = scalar;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in float x, in float y,in float z, in float w)
		{
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in Vector2 xy, in float z, in float w)
		{
			x = xy.x;
			y = xy.y;
			this.z = z;
			this.w = w;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in Vector2 xy, in Vector2 zw)
		{
			x = xy.x;
			y = xy.y;
			z = zw.x;
			w = zw.y;
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
					case 2: return z;
					case 3: return w;
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
					case 2: z = value; break;
					case 3: w = value; break;
					default: throw new IndexOutOfRangeException();
				}
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator +(in Vector4 left, in Vector4 right) { return new Vector4(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator -(in Vector4 left, in Vector4 right) { return new Vector4(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator *(in Vector4 left, in Vector4 right) { return new Vector4(left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator *(in Vector4 left, in float scalar ) { return new Vector4(left.x * scalar , left.y * scalar , left.z * scalar , left.w * scalar ); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator *(in float scalar, in Vector4 right) { return new Vector4(scalar * right.x, scalar * right.y, scalar * right.z, scalar * right.w); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator /(in Vector4 left, in Vector4 right) { return new Vector4(left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator /(in Vector4 left, in float scalar ) { return new Vector4(left.x / scalar , left.y / scalar , left.z / scalar , left.w / scalar ); }

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static implicit operator Vector3(in Vector4 v) { return new Vector3(v.x, v.y, v.z); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static implicit operator Color(in Vector4 v) { return new Color(v.x, v.y, v.z, v.w); }

		#endregion

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4 Normalize()
		{
			this = normalized;
			return this;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public override string ToString() { return "Vector4(" + x + ", " + y + ", " + z + ", " + w + ")"; }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 LerpUnclamped(Vector4 a, Vector4 b, float t) => Mathfs.Lerp(a, b, new Vector4(t));

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Equals(Vector4 other)
		{
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}
	}
}
