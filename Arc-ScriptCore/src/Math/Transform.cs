using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Transform
	{
		public Vector3 Translation;
		public Vector3 Rotation;
		public Vector3 Scale;

		#region Constructor

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Transform(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			Translation = position;
			Rotation = rotation;
			Scale = scale;
		}

		#endregion
	}
}
