using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("[{x}, {y}, {z}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3 : IEquatable<Vector3>
	{
		Vector128<float> xyz;

		public float x { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[0]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[0] = value; } }
		public float y { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[1]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[1] = value; } }
		public float z { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[2]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[2] = value; } }

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
			xyz = Vector128.Create(scalar, scalar, scalar, 0.0f);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3(in Vector128<float> xyz)
		{
			this.xyz = Vector128.Create(xyz[0], xyz[1], xyz[2], 0.0f);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3(in float x, in float y, in float z)
		{
			xyz = Vector128.Create(x, y, z, 0.0f);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vector3(in Vector2 xy, in float z)
		{
			xyz = Vector128.Create(xy.x, xy.y, z, 0.0f);
		}

		#endregion

		#region OverloadedOperators

		public float this[int index]
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				if (index < 3)
					return xyz.GetElement(index);
				else
					throw new IndexOutOfRangeException();
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				if (index < 3)
					xyz = xyz.WithElement(index, value);
				else
					throw new IndexOutOfRangeException();
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator +(in Vector3 left, in Vector3 right) { return new Vector3(left.xyz + right.xyz); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator -(in Vector3 left, in Vector3 right) { return new Vector3(left.xyz - right.xyz); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator *(in Vector3 left, in Vector3 right) { return new Vector3(left.xyz * right.xyz); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator *(in Vector3 left, in float scalar) { return new Vector3(left.xyz * scalar); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator *(in float scalar, in Vector3 right) { return new Vector3(scalar * right.xyz); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator /(in Vector3 left, in Vector3 right) { return new Vector3(left.xyz / right.xyz); }
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 operator /(in Vector3 left, in float scalar) { return new Vector3(left.xyz / Vector128.Create<float>(scalar)); }

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
			Vector4 tmp0 = new Vector4(v1.xyz[3], v1.x, v1.z, v1.y);
			Vector4 tmp1 = new Vector4(v2.xyz[3], v2.z, v2.x, v2.y);
			Vector128<float> tmp2 = tmp0.xyzw * v2.xyz;
			Vector128<float> tmp3 = tmp0.xyzw * tmp1.xyzw;
			Vector128<float> tmp4 = Vector128.Create(tmp2[3], tmp2[0], tmp2[2], tmp2[1]);
			return new Vector4(tmp3 - tmp4);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t) => Mathfs.Lerp(a, b, new Vector3(t));

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Equals(Vector3 other) => xyz == other.xyz;
	}
}
