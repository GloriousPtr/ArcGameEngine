using System;
using System.Runtime.CompilerServices;

namespace ArcEngine
{
	internal static class InternalCalls
	{
		#region Entity

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Entity_AddComponent(ulong entityID, Type type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool Entity_HasComponent(ulong entityID, Type type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Entity_GetComponent(ulong entityID, Type type, out IntPtr gcHandle);

		#endregion

		#region Log

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Log_LogMessage(Log.LogLevel level, string formattedMessage);

		#endregion

		#region Input

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool Input_IsKeyPressed(KeyCodes keycode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool Input_IsMouseButtonPressed(MouseCodes button);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Input_GetMousePosition(out Vector2 mousePosition);

		#endregion

		#region TagComponent

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern string TagComponent_GetTag(ulong entityID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TagComponent_SetTag(ulong entityID, string value);

		#endregion

		#region TransformComponent

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_GetTransform(ulong entityID, out Transform transform);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_SetTransform(ulong entityID, ref Transform transform);
		
		#endregion

	}
}
