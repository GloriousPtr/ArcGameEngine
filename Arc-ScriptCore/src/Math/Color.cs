using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using JetBrains.Annotations;

namespace ArcEngine
{
	[DebuggerDisplay("{ToHtmlStringRGBA()} [{r}, {g}, {b}, {a}]")]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	[StructLayout(LayoutKind.Sequential)]
	public struct Color : IEquatable<Color>
	{
		const MethodImplOptions INLINE = MethodImplOptions.AggressiveInlining;
		public Vector128<float> rgba;

		public float r { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[0]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[0] = value; } }
		public float g { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[1]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[1] = value; } }
		public float b { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[2]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[2] = value; } }
		public float a { [MethodImpl(MethodImplOptions.AggressiveInlining)] get { return this[3]; } [MethodImpl(MethodImplOptions.AggressiveInlining)] set { this[3] = value; } }

		#region Constructors

		[MethodImpl(INLINE)]
		public Color(in float r, in float g, in float b, in float a)
		{
			rgba = Vector128.Create(r, g, b, a);
		}

		public Color(in Vector128<float> rgba)
		{
			this.rgba = rgba;
		}

		[MethodImpl(INLINE)]
		public Color(in Vector4 rgba)
		{
			this.rgba = rgba.xyzw;
		}

		[MethodImpl(INLINE)]
		public Color(in Vector3 rgb, in float a)
		{
			rgba = Vector128.Create(rgb.x, rgb.y, rgb.z, a);
		}

		[MethodImpl(INLINE)]
		public Color(in Vector2 rg, in Vector2 ba)
		{
			rgba = Vector128.Create(rg.x, rg.y, ba.x, ba.y);
		}

		#endregion

		[MethodImpl(INLINE)]
		public override string ToString() { return "Color(" + rgba[0] + ", " + rgba[1] + ", " + rgba[2] + ", " + rgba[3] + ")"; }

		[MethodImpl(INLINE)]
		public string ToHtmlStringRGBA() => $"#{Mathfs.RoundToInt(r * 255.0f):X2}{Mathfs.RoundToInt(g * 255.0f):X2}{Mathfs.RoundToInt(b * 255.0f):X2}{Mathfs.RoundToInt(a * 255.0f):X2}";

		[MethodImpl(INLINE)]
		public string ToHtmlStringRGB() => $"#{Mathfs.RoundToInt(r * 255.0f):X2}{Mathfs.RoundToInt(g * 255.0f):X2}{Mathfs.RoundToInt(b * 255.0f):X2}";

		[MethodImpl(INLINE)]
		public bool Equals(Color other)
		{
			return rgba == other.rgba;
		}

		#region OverloadedOperators

		public float this[int index]
		{
			[MethodImpl(INLINE)]
			get
			{
				if (index < 4)
					return rgba.GetElement(index);
				else
					throw new IndexOutOfRangeException();
			}
			[MethodImpl(INLINE)]
			set
			{
				if (index < 4)
					rgba = rgba.WithElement(0, value);
				else
					throw new IndexOutOfRangeException();
			}
		}

		// Static Methods
		[MethodImpl(INLINE)]
		public static implicit operator Vector4(in Color color) { return new Vector4(color.rgba); }
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
