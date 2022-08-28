using System;
using System.Runtime.CompilerServices;

namespace ArcEngine
{
	[AttributeUsage(AttributeTargets.Field)]
	public class SerializeField : Attribute { }

	[AttributeUsage(AttributeTargets.Field)]
	public class HideInProperties : Attribute { }

	[AttributeUsage(AttributeTargets.Field)]
	public class ShowInProperties : Attribute { }

	[AttributeUsage(AttributeTargets.Field)]
	public class Tooltip : Attribute
	{
		public string Message;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Tooltip(string message)
		{
			Message = message;
		}
	}

	[AttributeUsage(AttributeTargets.Field)]
	public class Range : Attribute
	{
		public float Min;
		public float Max;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Range(float min, float max)
		{
			Min = min;
			Max = max;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Range(int min, int max)
		{
			Min = min;
			Max = max;
		}
	}
}
