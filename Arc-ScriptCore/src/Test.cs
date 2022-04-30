using System;

namespace ArcEngine
{
	public class Entity
	{
		public void OnCreate()
		{
			Console.WriteLine("On Create Called!");
		}

		public void OnUpdate(float timestep)
		{
			Console.WriteLine("On Update Called: {0}", timestep);
		}

		public void OnDestroy()
		{
			Console.WriteLine("On Destroyed Called");
		}
	}
}
