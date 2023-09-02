using System.Runtime.InteropServices;

namespace ArcEngine
{
	/// <summary>
	/// Holds data for collision events
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct CollisionData
	{
		private ulong entityID;
		public Vector2 relativeVelocity;

		public Entity entity => new Entity(entityID);
	}
}
