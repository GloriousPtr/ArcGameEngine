using System.Runtime.InteropServices;

namespace ArcEngine
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Rect
	{
		public float x;
		public float y;
		public float width;
		public float height;

		public Vector2 center
		{
			get => new Vector2(x + width * 0.5f, y + height * 0.5f);
			set
			{
				x = value.x - width * 0.5f;
				y = value.y - height * 0.5f;
			}
		}

		public Vector2 max
		{
			get => new Vector2(x + width, y + height);
			set
			{
				x = value.x - width;
				y = value.y - height;
			}
		}

		public Vector2 min
		{
			get => new Vector2(x, y);
			set
			{
				x = value.x;
				y = value.y;
			}
		}

		public Vector2 position
		{
			get => new Vector2(x, y);
			set
			{
				x = value.x;
				y = value.y;
			}
		}

		public Vector2 size
		{
			get => new Vector2(width, height);
			set
			{
				width = value.x;
				height = value.y;
			}
		}

		public float xMax
		{
			get => x + width;
			set => x = value - width;
		}

		public float xMin
		{
			get => x;
			set => x = value;
		}

		public float yMax
		{
			get => y + height;
			set => y = value - height;
		}

		public float yMin
		{
			get => y;
			set => y = value;
		}

		public Rect(float x, float y, float width, float height)
		{
			this.x = x;
			this.y = y;
			this.width = width;
			this.height = height;
		}

		public Rect(Vector2 xy, Vector2 size)
		{
			x = xy.x;
			y = xy.y;
			width = size.x;
			height = size.y;
		}

		public static Rect zero() => new Rect(0.0f, 0.0f, 0.0f, 0.0f);
	}
}
