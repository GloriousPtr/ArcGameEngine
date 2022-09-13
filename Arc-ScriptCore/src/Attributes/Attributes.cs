using System;
using System.Runtime.CompilerServices;

namespace ArcEngine
{
	[AttributeUsage(AttributeTargets.Field)]
	public class SerializeFieldAttribute : Attribute { }

	[AttributeUsage(AttributeTargets.Field)]
	public class HideInPropertiesAttribute : Attribute { }

	[AttributeUsage(AttributeTargets.Field)]
	public class ShowInPropertiesAttribute : Attribute { }

	[AttributeUsage(AttributeTargets.Field)]
	public class HeaderAttribute : Attribute
	{
		internal string Message;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public HeaderAttribute(string message)
		{
			Message = message;
		}
	}

	[AttributeUsage(AttributeTargets.Field)]
	public class TooltipAttribute : Attribute
	{
		internal string Message;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public TooltipAttribute(string message)
		{
			Message = message;
		}
	}

	[AttributeUsage(AttributeTargets.Field)]
	public class RangeAttribute : Attribute
	{
		internal float Min;
		internal float Max;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RangeAttribute(float min, float max)
		{
			Min = min;
			Max = max;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RangeAttribute(int min, int max)
		{
			Min = min;
			Max = max;
		}
	}
}
