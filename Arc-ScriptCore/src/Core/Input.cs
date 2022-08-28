using System.Runtime.CompilerServices;

namespace ArcEngine
{
	public static class Input
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool IsKeyPressed(KeyCodes keycode) => InternalCalls.Input_IsKeyPressed(keycode);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool IsMouseButtonPressed(MouseCodes button) => InternalCalls.Input_IsMouseButtonPressed(button);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 result);
			return result;
		}
	}
}
