using System;
using System.Runtime.CompilerServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	/// <summary>
	/// Tells Arc to serialize the field, public fields are automatically serialized.
	/// Use this attribute to mark internal/private fields to be serialized.
	/// </summary>
	[AttributeUsage(AttributeTargets.Field)]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public class SerializeFieldAttribute : Attribute { }

	/// <summary>
	/// Tells Arc to hide the field from properties panel.
	/// This attribute doesn't change the serialization behavior.
	/// </summary>
	[AttributeUsage(AttributeTargets.Field)]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public class HideInPropertiesAttribute : Attribute { }

	/// <summary>
	/// Tells Arc to show the field from properties panel.
	/// Public fields are already visible. Use this attribute to mark internal/private fields as visible.
	/// This attribute doesn't change the serialization behavior.
	/// </summary>
	[AttributeUsage(AttributeTargets.Field)]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public class ShowInPropertiesAttribute : Attribute { }

	/// <summary>
	/// Adds a header before the field
	/// </summary>
	[AttributeUsage(AttributeTargets.Field)]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public class HeaderAttribute : Attribute
	{
		internal string Message;

		/// <summary>
		/// Show header before the field
		/// </summary>
		/// <param name="message">Header string.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public HeaderAttribute(string message)
		{
			Message = message;
		}
	}

	/// <summary>
	/// Shows the tooltip when mouse is hovered over the field.
	/// </summary>
	[AttributeUsage(AttributeTargets.Field)]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public class TooltipAttribute : Attribute
	{
		internal string Message;

		/// <summary>
		/// Shows the tooltip when mouse is hovered over the field.
		/// </summary>
		/// <param name="message">Tooltip message.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public TooltipAttribute(string message) => Message = message;
	}

	/// <summary>
	/// Change the input field to a slider.
	/// Use this attribute to make a number type variable (byte, ubyte, short, ushort, int, uint, long, ulong, float, double) in a script be restricted to a specific range.
	/// </summary>
	[AttributeUsage(AttributeTargets.Field)]
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public class RangeAttribute : Attribute
	{
		internal float Min;
		internal float Max;

		/// <summary>
		/// Change the input field to a slider.
		/// Use this attribute to make a number type variable (byte, ubyte, short, ushort, int, uint, long, ulong, float, double) in a script be restricted to a specific range.
		/// </summary>
		/// <param name="min">Min value (inclusive)</param>
		/// <param name="max">Max value (inclusive)</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RangeAttribute(float min, float max)
		{
			Min = min;
			Max = max;
		}

		/// <summary>
		/// Change the input field to a slider.
		/// Use this attribute to make a number type variable (byte, ubyte, short, ushort, int, uint, long, ulong, float, double) in a script be restricted to a specific range.
		/// </summary>
		/// <param name="min">Min value (inclusive)</param>
		/// <param name="max">Max value (inclusive)</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RangeAttribute(int min, int max)
		{
			Min = min;
			Max = max;
		}
	}
}
