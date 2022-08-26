using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector2
	{
		public float X;
		public float Y;

		public Vector2(in float scalar)
		{
			X = Y = scalar;
		}

		public Vector2(in float x, in float y)
		{
			X = x;
			Y = y;
		}

		public static Vector2 operator +(in Vector2 left, in Vector2 right) { return new Vector2(left.X + right.X, left.Y + right.Y); }
		public static Vector2 operator -(in Vector2 left, in Vector2 right) { return new Vector2(left.X - right.X, left.Y - right.Y); }
		public static Vector2 operator *(in Vector2 left, in Vector2 right) { return new Vector2(left.X * right.X, left.Y * right.Y); }
		public static Vector2 operator *(in Vector2 left, in float scalar ) { return new Vector2(left.X * scalar , left.Y * scalar ); }
		public static Vector2 operator *(in float scalar, in Vector2 right) { return new Vector2(scalar * right.X, scalar * right.Y); }
		public static Vector2 operator /(in Vector2 left, in Vector2 right) { return new Vector2(left.X / right.X, left.Y / right.Y); }
		public static Vector2 operator /(in Vector2 left, in float scalar ) { return new Vector2(left.X / scalar , left.Y / scalar ); }

		public override string ToString() { return "Vector2(" + X + ", " + Y + ")"; }
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3
	{
		public float X;
		public float Y;
		public float Z;

		public Vector3(in float scalar)
		{
			X = Y = Z = scalar;
		}

		public Vector3(in float x, in float y, in float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(in Vector2 xy, in float z)
		{
			X = xy.X;
			Y = xy.Y;
			Z = z;
		}

		public static Vector3 operator +(in Vector3 left, in Vector3 right) { return new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z); }
		public static Vector3 operator -(in Vector3 left, in Vector3 right) { return new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z); }
		public static Vector3 operator *(in Vector3 left, in Vector3 right) { return new Vector3(left.X * right.X, left.Y * right.Y, left.Z * right.Z); }
		public static Vector3 operator *(in Vector3 left, in float scalar ) { return new Vector3(left.X * scalar , left.Y * scalar , left.Z * scalar ); }
		public static Vector3 operator *(in float scalar, in Vector3 right) { return new Vector3(scalar * right.X, scalar * right.Y, scalar * right.Z); }
		public static Vector3 operator /(in Vector3 left, in Vector3 right) { return new Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z); }
		public static Vector3 operator /(in Vector3 left, in float scalar ) { return new Vector3(left.X / scalar , left.Y / scalar , left.Z / scalar ); }

		public override string ToString() { return "Vector3(" + X + ", " + Y + ", " + Z + ")"; }
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct Vector4
	{
		public float X;
		public float Y;
		public float Z;
		public float W;

		public Vector4(in float scalar)
		{
			X = Y = Z = W = scalar;
		}

		public Vector4(in float x, in float y,in float z, in float w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
		}

		public Vector4(in Vector2 xy, in float z, in float w)
		{
			X = xy.X;
			Y = xy.Y;
			Z = z;
			W = w;
		}

		public Vector4(in Vector2 xy, in Vector2 zw)
		{
			X = xy.X;
			Y = xy.Y;
			Z = zw.X;
			W = zw.Y;
		}

		public static Vector4 operator +(in Vector4 left, in Vector4 right) { return new Vector4(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W); }
		public static Vector4 operator -(in Vector4 left, in Vector4 right) { return new Vector4(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W); }
		public static Vector4 operator *(in Vector4 left, in Vector4 right) { return new Vector4(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W); }
		public static Vector4 operator *(in Vector4 left, in float scalar ) { return new Vector4(left.X * scalar , left.Y * scalar , left.Z * scalar , left.W * scalar ); }
		public static Vector4 operator *(in float scalar, in Vector4 right) { return new Vector4(scalar * right.X, scalar * right.Y, scalar * right.Z, scalar * right.W); }
		public static Vector4 operator /(in Vector4 left, in Vector4 right) { return new Vector4(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W); }
		public static Vector4 operator /(in Vector4 left, in float scalar ) { return new Vector4(left.X / scalar , left.Y / scalar , left.Z / scalar , left.W / scalar ); }

		public override string ToString() { return "Vector4(" + X + ", " + Y + ", " + Z + ", " + W + ")"; }
	}
}
