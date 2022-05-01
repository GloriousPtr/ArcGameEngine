namespace ArcEngine
{
	public static class Input
	{
		public static bool IsKeyPressed(KeyCodes keycode) => InternalCalls.Input_IsKeyPressed(keycode);
		public static bool IsMouseButtonPressed(MouseCodes button) => InternalCalls.Input_IsMouseButtonPressed(button);

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 result);
			return result;
		}
	}
}
