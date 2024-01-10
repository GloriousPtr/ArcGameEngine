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
	public struct Quaternion : IEquatable<Quaternion>
	{
		const MethodImplOptions INLINE = MethodImplOptions.AggressiveInlining;

		public Vector128<float> xyzw;

		public float x { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[0]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[0] = value; } }
		public float y { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[1]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[1] = value; } }
		public float z { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[2]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[2] = value; } }
		public float w { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[3]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[3] = value; } }

		[MethodImpl(INLINE)]
		public Quaternion(in float x, in float y, in float z, in float w)
		{
			xyzw = Vector128.Create(x, y, z, w);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Quaternion(in Vector128<float> xyzw)
		{
			this.xyzw = xyzw;
		}

		[MethodImpl(INLINE)]
		public Quaternion(in Vector4 xyzw)
		{
			this.xyzw = xyzw.xyzw;
		}

		[MethodImpl(INLINE)]
		public Quaternion(in Vector3 xyz, in float w)
		{
			xyzw = Vector128.Create(xyz.x, xyz.y, xyz.z, w);
		}

		[MethodImpl(INLINE)]
		public Quaternion(in Vector2 xy, in Vector2 zw)
		{
			xyzw = Vector128.Create(xy.x, xy.y, zw.x, zw.y);
		}

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

		[MethodImpl(INLINE)]
		public static Quaternion FromToRotation(Vector3 v1, Vector3 v2)
		{
			Vector3 xyz = Vector3.Cross(v1, v2);
			float perspectiveDivideTerm = Mathfs.Sqrt((v1.sqrMagnitude * v2.sqrMagnitude) + Vector3.Dot(v1, v2));
			return new Quaternion(xyz, perspectiveDivideTerm);
		}

		[MethodImpl(INLINE)]
		public bool Equals(Quaternion other)
		{
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}
	}
}
