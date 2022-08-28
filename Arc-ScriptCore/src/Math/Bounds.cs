using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Bounds
	{
		public Vector3 center;
		public Vector3 size;

		public Vector3 extents
		{
			get => size * 0.5f;
			set => size = value * 2.0f;
		}

		public Vector3 max
		{
			get => center + extents;
			set => center = value - extents;
		}

		public Vector3 min
		{
			get => center - extents;
			set => center = value + extents;
		}

		public Bounds(in Vector3 center, in Vector3 size)
		{
			this.center = center;
			this.size = size;
		}
	}
}
