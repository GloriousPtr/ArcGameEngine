using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("[{x}, {y}, {z}, {w}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector4 : IEquatable<Vector4>
	{
		public Vector128<float> xyzw;

		public float x { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[0]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[0] = value; } }
		public float y { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[1]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[1] = value; } }
		public float z { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[2]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[2] = value; } }
		public float w { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[3]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[3] = value; } }

		public float magnitude { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return (float)Math.Sqrt(sqrMagnitude); } }
		public float sqrMagnitude { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { Vector128<float> mul = xyzw * xyzw; return mul[0] + mul[1] + mul[2] + mul[3]; } }
		public Vector4 normalized { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this / magnitude; } }

		public static Vector4 one { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector4(1.0f); } }
		public static Vector4 zero { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return new Vector4(0.0f); } }

		#region Constructors

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in float scalar)
		{
			xyzw = Vector128.Create(scalar);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in Vector128<float> xyzw)
		{
			this.xyzw = xyzw;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in float x, in float y,in float z, in float w)
		{
			xyzw = Vector128.Create(x, y, z, w);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in Vector2 xy, in float z, in float w)
		{
			xyzw = Vector128.Create(xy.x, xy.y, z, w);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4(in Vector2 xy, in Vector2 zw)
		{
			xyzw = Vector128.Create(xy.x, xy.y, zw.x, zw.y);
		}

		#endregion

		#region OverloadedOperators

		public float this[int index]
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				if (index < 4)
					return xyzw.GetElement(index);
				else
					throw new IndexOutOfRangeException();
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				if (index < 4)
					xyzw = xyzw.WithElement(0, value);
				else
					throw new IndexOutOfRangeException();
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator +(in Vector4 left, in Vector4 right) { return new Vector4(left.xyzw + right.xyzw); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator -(in Vector4 left, in Vector4 right) { return new Vector4(left.xyzw - right.xyzw); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator *(in Vector4 left, in Vector4 right) { return new Vector4(left.xyzw * right.xyzw); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator *(in Vector4 left, in float scalar ) { return new Vector4(left.xyzw * scalar); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator *(in float scalar, in Vector4 right) { return new Vector4(scalar * right.xyzw); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator /(in Vector4 left, in Vector4 right) { return new Vector4(left.xyzw / right.xyzw); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 operator /(in Vector4 left, in float scalar ) { return new Vector4(left.xyzw / Vector128.Create(scalar)); }

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static implicit operator Vector3(in Vector4 v) { return new Vector3(v.xyzw[0], v.xyzw[1], v.xyzw[2]); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static implicit operator Color(in Vector4 v) { return new Color(v.xyzw); }

		#endregion

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector4 Normalize()
		{
			this = normalized;
			return this;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public override string ToString() { return "Vector4(" + xyzw[0] + ", " + xyzw[1] + ", " + xyzw[2] + ", " + xyzw[3] + ")"; }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 LerpUnclamped(Vector4 a, Vector4 b, float t) => Mathfs.Lerp(a, b, new Vector4(t));

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Equals(Vector4 other)
		{
			return xyzw == other.xyzw;
		}
	}
}
