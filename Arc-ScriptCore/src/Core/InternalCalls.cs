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
		internal static extern object Entity_GetComponent(ulong entityID, Type type);

		#endregion

		#region Log

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Log_LogMessage(Log.LogLevel level, string formattedMessage, string fileName, string memberName, int lineNumber);

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
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_GetRotation(ulong entityID, out Vector3 eulerAngles);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_SetRotation(ulong entityID, ref Vector3 eulerAngles);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_GetScale(ulong entityID, out Vector3 scale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);
		
		#endregion

		#region SpriteRendererComponent

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SpriteRendererComponent_GetColor(ulong entityID, out Color tint);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SpriteRendererComponent_SetColor(ulong entityID, ref Color tint);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SpriteRendererComponent_GetTilingFactor(ulong entityID, out float tiling);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SpriteRendererComponent_SetTilingFactor(ulong entityID, ref float tiling);

		#endregion

		#region Rigidbody2DComponent

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetBodyType(ulong entityID, out int v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetBodyType(ulong entityID, ref int v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetAutoMass(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetAutoMass(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetMass(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetMass(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetLinearDrag(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetLinearDrag(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetAngularDrag(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetAngularDrag(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetAllowSleep(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetAllowSleep(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetAwake(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetAwake(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetContinuous(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetContinuous(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetFreezeRotation(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetFreezeRotation(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetGravityScale(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetGravityScale(ulong entityID, ref float v);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_ApplyForceAtCenter(ulong entityID, ref Vector2 f);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_ApplyForce(ulong entityID, ref Vector2 f, ref Vector2 p);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_ApplyLinearImpulseAtCenter(ulong entityID, ref Vector2 i);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 i, ref Vector2 p);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_ApplyAngularImpulse(ulong entityID, ref float i);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_ApplyTorque(ulong entityID, ref float t);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_IsAwake(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_IsSleeping(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_MovePosition(ulong entityID, ref Vector2 p);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_MoveRotation(ulong entityID, ref float r);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetVelocity(ulong entityID, out Vector2 v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetVelocity(ulong entityID, ref Vector2 v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_GetAngularVelocity(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_SetAngularVelocity(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_Sleep(ulong entityID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Rigidbody2DComponent_WakeUp(ulong entityID);

		#endregion

		#region AudioSourceComponent

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetVolume(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetVolume(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetPitch(ulong entityID, out float p);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetPitch(ulong entityID, ref float p);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetPlayOnAwake(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetPlayOnAwake(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetLooping(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetLooping(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetSpatialization(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetSpatialization(ulong entityID, ref bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetAttenuationModel(ulong entityID, out int v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetAttenuationModel(ulong entityID, ref int v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetRollOff(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetRollOff(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetMinGain(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetMinGain(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetMaxGain(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetMaxGain(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetMinDistance(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetMinDistance(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetMaxDistance(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetMaxDistance(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetConeInnerAngle(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetConeInnerAngle(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetConeOuterAngle(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetConeOuterAngle(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetConeOuterGain(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetConeOuterGain(ulong entityID, ref float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetCone(ulong entityID, ref float innerConeAngle, ref float outerConeAngle, ref float outerConeGain);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_GetDopplerFactor(ulong entityID, out float v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_SetDopplerFactor(ulong entityID, ref float v);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_IsPlaying(ulong entityID, out bool v);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_Play(ulong entityID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_Pause(ulong entityID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_UnPause(ulong entityID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void AudioSource_Stop(ulong entityID);

		#endregion
	}
}
