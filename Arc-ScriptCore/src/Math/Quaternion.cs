using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Quaternion : IEquatable<Quaternion>
	{
		const MethodImplOptions INLINE = MethodImplOptions.AggressiveInlining;

		public float x;
		public float y;
		public float z;
		public float w;

		[MethodImpl(INLINE)]
		public Quaternion(in float x, in float y, in float z, in float w)
		{
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}

		[MethodImpl(INLINE)]
		public Quaternion(in Vector4 xyzw)
		{
			x = xyzw.x;
			y = xyzw.y;
			z = xyzw.z;
			w = xyzw.w;
		}

		[MethodImpl(INLINE)]
		public Quaternion(in Vector3 xyz, in float w)
		{
			x = xyz.x;
			y = xyz.y;
			z = xyz.z;
			this.w = w;
		}

		[MethodImpl(INLINE)]
		public Quaternion(in Vector2 xy, in Vector2 zw)
		{
			x = xy.x;
			y = xy.y;
			z = zw.x;
			w = zw.y;
		}

		[MethodImpl(INLINE)]
		public static Quaternion FromToRotation(Vector3 v1, Vector3 v2)
		{
			Vector3 xyz = Vector3.Cross(v1, v2);
			float w = Mathfs.Sqrt((v1.sqrMagnitude * v2.sqrMagnitude) + Vector3.Dot(v1, v2));
			return new Quaternion(xyz, w);
		}

		[MethodImpl(INLINE)]
		public bool Equals(Quaternion other)
		{
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}
	}
}
