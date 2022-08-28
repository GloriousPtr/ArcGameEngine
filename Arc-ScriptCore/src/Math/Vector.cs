using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace ArcEngine
{
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
				return index switch
				{
					0 => x,
					1 => y,
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

	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3 : IEquatable<Vector3>
	{
		public float x;
		public float y;
		public float z;

		public float magnitude => (float)Math.Sqrt(x * x + y * y + z * z);
		public float sqrMagnitude => x * x + y * y + z * z;
		public Vector3 normalized => this / magnitude;

		public static Vector3 one => new Vector3(1.0f);
		public static Vector3 zero => new Vector3(0.0f);

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
		public bool Equals(Vector3 other)
		{
			return x == other.x && y == other.y && z == other.z;
		}
	}

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
				return index switch
				{
					0 => x,
					1 => y,
					2 => z,
					3 => w,
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
