using System;
using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Color
	{
		public float R;
		public float G;
		public float B;
		public float A;

		public float this[int index]
		{
			get
			{
				return index switch
				{
					0 => R,
					1 => G,
					2 => B,
					3 => A,
					_ => throw new IndexOutOfRangeException(),
				};
			}
			set
			{
				switch (index)
				{
					case 0: R = value; break;
					case 1: G = value; break;
					case 2: B = value; break;
					case 3: A = value; break;
					default: throw new IndexOutOfRangeException();
				}
			}
		}

		public Color Greyscale
		{
			get
			{
				float avg = (R + G + B) / 3.0f;
				return new(avg, avg, avg, A);
			}
		}

		// Constructor
		public Color(in float r, in float g, in float b, in float a)
		{
			R = r;
			G = g;
			B = b;
			A = a;
		}

		public Color(in Vector4 rgba)
		{
			R = rgba.X;
			G = rgba.Y;
			B = rgba.Z;
			A = rgba.W;
		}

		public Color(in Vector3 rgb, in float a)
		{
			R = rgb.X;
			G = rgb.Y;
			B = rgb.Z;
			A = a;
		}

		public Color(in Vector2 rg, in Vector2 ba)
		{
			R = rg.X;
			G = rg.Y;
			B = ba.X;
			A = ba.Y;
		}

		public override string ToString() { return "Color(" + R + ", " + G + ", " + B + ", " + A + ")"; }

		// Static Methods
		public static Color Black => new(0.0f, 0.0f, 0.0f, 1.0f);
		public static Color Blue => new(0.0f, 0.0f, 1.0f, 1.0f);
		public static Color Clear => new(0.0f, 0.0f, 0.0f, 0.0f);
		public static Color Cyan => new(0.0f, 1.0f, 1.0f, 1.0f);
		public static Color Green => new(0.0f, 1.0f, 0.0f, 1.0f);
		public static Color Grey => new(0.5f, 0.5f, 0.5f, 1.0f);
		public static Color Magenta => new(1.0f, 0.0f, 1.0f, 1.0f);
		public static Color Red => new(1.0f, 0.0f, 0.0f, 1.0f);
		public static Color White => new(1.0f, 1.0f, 1.0f, 1.0f);
		public static Color Yellow => new(1.0f, 0.92f, 0.016f, 1.0f);
	}
}
