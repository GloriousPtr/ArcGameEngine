using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Matrix4
	{
		public float D00;
		public float D10;
		public float D20;
		public float D30;
		public float D01;
		public float D11;
		public float D21;
		public float D31;
		public float D02;
		public float D12;
		public float D22;
		public float D32;
		public float D03;
		public float D13;
		public float D23;
		public float D33;

		#region Constructor

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix4(in float value)
		{
			D00 = value; D10 = 0.0f;  D20 = 0.0f;  D30 = 0.0f;
			D01 = 0.0f;  D11 = value; D21 = 0.0f;  D31 = 0.0f;
			D02 = 0.0f;  D12 = 0.0f;  D22 = value; D32 = 0.0f;
			D03 = 0.0f;  D13 = 0.0f;  D23 = 0.0f;  D33 = value;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix4(in Vector4 r0, in Vector4 r1, in Vector4 r2, in Vector4 r3)
		{
			D00 = r0.x; D10 = r0.y; D20 = r0.z; D30 = r0.w;
			D01 = r1.x; D11 = r1.y; D21 = r1.z; D31 = r1.w;
			D02 = r2.x; D12 = r2.y; D22 = r2.z; D32 = r2.w;
			D03 = r3.x; D13 = r3.y; D23 = r3.z; D33 = r3.w;
		}

		#endregion

		public Vector3 Translation
		{
			get => new Vector3(D03, D13, D23);
			set { D03 = value.x; D13 = value.y; D23 = value.z; }
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Matrix4 Translate(in Vector3 translation)
		{
			Matrix4 result = new Matrix4(1.0f)
			{
				D03 = translation.x,
				D13 = translation.y,
				D23 = translation.z
			};
			return result;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Matrix4 Scale(in Vector3 scale)
		{
			Matrix4 result = new Matrix4(1.0f)
			{
				D00 = scale.x,
				D11 = scale.y,
				D22 = scale.z,
			};
			return result;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Matrix4 Scale(in float scale)
		{
			Matrix4 result = new Matrix4(1.0f)
			{
				D00 = scale,
				D11 = scale,
				D22 = scale,
			};
			return result;
		}
	}
}
