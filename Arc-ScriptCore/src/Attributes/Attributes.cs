using System;

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

		public Range(float min, float max)
		{
			Min = min;
			Max = max;
		}

		public Range(int min, int max)
		{
			Min = min;
			Max = max;
		}
	}
}
