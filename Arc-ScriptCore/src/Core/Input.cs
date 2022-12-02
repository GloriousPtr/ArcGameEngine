using System.Runtime.CompilerServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	/// <summary>
	/// Provides input state of the game.
	/// </summary>
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
	public static class Input
	{
		/// <summary>
		/// Key held down.
		/// </summary>
		/// <param name="keycode">KeyCode</param>
		/// <returns>true if key is held down, otherwise false.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool IsKeyPressed(KeyCodes keycode) => InternalCalls.Input_IsKeyPressed(keycode);

		/// <summary>
		/// Mouse button held down.
		/// </summary>
		/// <param name="button">MouseCode</param>
		/// <returns>true if mouse button is held down, otherwise false.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool IsMouseButtonPressed(MouseCodes button) => InternalCalls.Input_IsMouseButtonPressed(button);

		/// <summary>
		/// Mouse position.
		/// </summary>
		/// <returns>Vector2 with Mouse Position in screen space.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 result);
			return result;
		}
	}
}
