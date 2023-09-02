using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("{ToHtmlStringRGBA()} [{r}, {g}, {b}, {a}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Color : IEquatable<Color>
	{
		const MethodImplOptions INLINE = MethodImplOptions.AggressiveInlining;

		public float r;
		public float g;
		public float b;
		public float a;

		#region Constructors

		[MethodImpl(INLINE)]
		public Color(in float r, in float g, in float b, in float a)
		{
			this.r = r;
			this.g = g;
			this.b = b;
			this.a = a;
		}

		[MethodImpl(INLINE)]
		public Color(in Vector4 rgba)
		{
			r = rgba.x;
			g = rgba.y;
			b = rgba.z;
			a = rgba.w;
		}

		[MethodImpl(INLINE)]
		public Color(in Vector3 rgb, in float a)
		{
			r = rgb.x;
			g = rgb.y;
			b = rgb.z;
			this.a = a;
		}

		[MethodImpl(INLINE)]
		public Color(in Vector2 rg, in Vector2 ba)
		{
			r = rg.x;
			g = rg.y;
			b = ba.x;
			a = ba.y;
		}

		#endregion

		[MethodImpl(INLINE)]
		public override string ToString() { return "Color(" + r + ", " + g + ", " + b + ", " + a + ")"; }

		[MethodImpl(INLINE)]
		public string ToHtmlStringRGBA() => $"#{Mathfs.RoundToInt(r * 255.0f):X2}{Mathfs.RoundToInt(g * 255.0f):X2}{Mathfs.RoundToInt(b * 255.0f):X2}{Mathfs.RoundToInt(a * 255.0f):X2}";

		[MethodImpl(INLINE)]
		public string ToHtmlStringRGB() => $"#{Mathfs.RoundToInt(r * 255.0f):X2}{Mathfs.RoundToInt(g * 255.0f):X2}{Mathfs.RoundToInt(b * 255.0f):X2}";

		[MethodImpl(INLINE)]
		public bool Equals(Color other)
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}

		#region OverloadedOperators

		public float this[int index]
		{
			[MethodImpl(INLINE)]
			get
			{
				switch (index)
				{
					case 0: return r;
					case 1: return g;
					case 2: return b;
					case 3: return a;
					default: throw new IndexOutOfRangeException();
				}
			}
			[MethodImpl(INLINE)]
			set
			{
				switch (index)
				{
					case 0: r = value; break;
					case 1: g = value; break;
					case 2: b = value; break;
					case 3: a = value; break;
					default: throw new IndexOutOfRangeException();
				}
			}
		}

		// Static Methods
		[MethodImpl(INLINE)]
		public static implicit operator Vector4(in Color color) { return new Vector4(color.r, color.g, color.b, color.a); }
		[MethodImpl(INLINE)]
		public static implicit operator Vector3(in Color color) { return new Vector3(color.r, color.g, color.b); }

		#endregion

		#region Properties

		public Color Greyscale
		{
			[MethodImpl(INLINE)]
			get
			{
				float avg = (r + g + b) / 3.0f;
				return new Color(avg, avg, avg, a);
			}
		}

		#endregion

		#region StaticProperties


		public static Color Black { [MethodImpl(INLINE)] get { return new Color(0.0f, 0.0f, 0.0f, 1.0f); } }
		public static Color Blue { [MethodImpl(INLINE)] get { return new Color(0.0f, 0.0f, 1.0f, 1.0f); } }
		public static Color Clear { [MethodImpl(INLINE)] get { return new Color(0.0f, 0.0f, 0.0f, 0.0f); } }
		public static Color Cyan { [MethodImpl(INLINE)] get { return new Color(0.0f, 1.0f, 1.0f, 1.0f); } }
		public static Color Green {	[MethodImpl(INLINE)] get { return new Color(0.0f, 1.0f, 0.0f, 1.0f); } }
		public static Color Grey { [MethodImpl(INLINE)] get { return new Color(0.5f, 0.5f, 0.5f, 1.0f); } }
		public static Color Magenta { [MethodImpl(INLINE)] get { return new Color(1.0f, 0.0f, 1.0f, 1.0f); } }
		public static Color Red { [MethodImpl(INLINE)] get { return new Color(1.0f, 0.0f, 0.0f, 1.0f); } }
		public static Color White { [MethodImpl(INLINE)] get { return new Color(1.0f, 1.0f, 1.0f, 1.0f); } }
		public static Color Yellow { [MethodImpl(INLINE)] get { return new Color(1.0f, 0.92f, 0.016f, 1.0f); } }

		#endregion
	}
}
