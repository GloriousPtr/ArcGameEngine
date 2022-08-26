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

		public Matrix4(in float value)
		{
			D00 = value; D10 = 0.0f;  D20 = 0.0f;  D30 = 0.0f;
			D01 = 0.0f;  D11 = value; D21 = 0.0f;  D31 = 0.0f;
			D02 = 0.0f;  D12 = 0.0f;  D22 = value; D32 = 0.0f;
			D03 = 0.0f;  D13 = 0.0f;  D23 = 0.0f;  D33 = value;
		}

		public Vector3 Translation
		{
			get => new Vector3(D03, D13, D23);
			set { D03 = value.X; D13 = value.Y; D23 = value.Z; }
		}

		public static Matrix4 Translate(in Vector3 translation)
		{
			Matrix4 result = new Matrix4(1.0f)
			{
				D03 = translation.X,
				D13 = translation.Y,
				D23 = translation.Z
			};
			return result;
		}

		public static Matrix4 Scale(in Vector3 scale)
		{
			Matrix4 result = new Matrix4(1.0f)
			{
				D00 = scale.X,
				D11 = scale.Y,
				D22 = scale.Z,
			};
			return result;
		}

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
