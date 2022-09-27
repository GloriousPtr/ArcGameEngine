using System.Runtime.CompilerServices;

namespace ArcEngine
{
	public interface IComponent
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal ulong GetEntityID();
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal void SetEntity(ulong id);
	}

	public abstract class Component : IComponent
	{
		internal ulong entityID;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		ulong IComponent.GetEntityID() => entityID;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		void IComponent.SetEntity(ulong id) => entityID = id;
	}

	/// <summary>
	/// Contains name of the entity.
	/// </summary>
	public class TagComponent : Component
	{
		/// <summary>
		/// Entity name.
		/// </summary>
		public string tag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get => InternalCalls.TagComponent_GetTag(entityID);
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TagComponent_SetTag(entityID, value);
		}
	}

	/// <summary>
	/// Transform Component, has Translation, Rotation and Scale information.
	/// </summary>
	public class TransformComponent : Component
	{
		/// <summary>
		/// Actual transform containing Translation, Rotation and Scale.
		/// </summary>
		public Transform transform
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.TransformComponent_GetTransform(entityID, out Transform v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TransformComponent_SetTransform(entityID, ref value);
		}

		/// <summary>
		/// World space translation.
		/// </summary>
		public Vector3 translation
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.TransformComponent_GetTranslation(entityID, out Vector3 v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TransformComponent_SetTranslation(entityID, ref value);
		}

		/// <summary>
		/// World space rotation in euler angles (radians).
		/// </summary>
		public Vector3 rotation
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.TransformComponent_GetRotation(entityID, out Vector3 v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TransformComponent_SetRotation(entityID, ref value);
		}

		/// <summary>
		/// World space scale.
		/// </summary>
		public Vector3 scale
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.TransformComponent_GetScale(entityID, out Vector3 v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TransformComponent_SetScale(entityID, ref value);
		}
	}
	
	/// <summary>
	/// Renders a sprite
	/// </summary>
	public class SpriteRendererComponent : Component
	{
		/// <summary>
		/// Sprite tint color
		/// </summary>
		public Color tint
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(entityID, out Color v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.SpriteRendererComponent_SetColor(entityID, ref value);
		}

		/// <summary>
		/// Tiling factor for repeated tiling
		/// </summary>
		public float tilingFactor
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.SpriteRendererComponent_GetTilingFactor(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.SpriteRendererComponent_SetTilingFactor(entityID, ref value);
		}
	}

	/// <summary>
	/// Rigidbody physics component for 2D sprites.
	/// </summary>
	public class Rigidbody2DComponent : Component
	{
		/// <summary>
		/// <br/>Static: No affect of physics forces and not mutable from script.
		///	<br/>Kinematic: No affect of physics forces but is mutable from script.
		///	<br/>Dynamic: Affect of physics forces.
		/// </summary>
		public enum BodyType { Static = 0, Kinematic, Dynamic }

		/// <summary>
		/// <br/>Static: No affect of physics forces and not mutable from script.
		///	<br/>Kinematic: No affect of physics forces but is mutable from script.
		///	<br/>Dynamic: Affect of physics forces.
		/// </summary>
		public BodyType type
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetBodyType(entityID, out int v);
				return (BodyType)v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				int v = (int)value;
				InternalCalls.Rigidbody2DComponent_SetBodyType(entityID, ref v);
			}
		}

		/// <summary>
		/// Enable this property to have the Rigidbody 2D automatically detect the Entity’s mass from its Collider 2D.
		/// </summary>
		public bool autoMass
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAutoMass(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAutoMass(entityID, ref value);
		}

		/// <summary>
		/// Define the mass of the Rigidbody 2D.
		/// </summary>
		public float mass
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetMass(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetMass(entityID, ref value);
		}

		/// <summary>
		/// Set the drag coefficient affecting translational movement.
		/// </summary>
		public float linearDrag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetLinearDrag(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetLinearDrag(entityID, ref value);
		}

		/// <summary>
		/// Set the drag coefficient affecting rotational movement.
		/// </summary>
		public float angularDrag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAngularDrag(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAngularDrag(entityID, ref value);
		}

		/// <summary>
		/// Define the degree to which the Entity is affected by gravity.
		/// </summary>
		public float gravityScale
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetGravityScale(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetGravityScale(entityID, ref value);
		}

		/// <summary>
		/// Select this option to have sleeping enabled.
		/// Important: This should be enabled where possible, as disabling it can impact system resources.
		/// </summary>
		public bool allowSleep
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAllowSleep(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAllowSleep(entityID, ref value);
		}

		/// <summary>
		/// Select this to have the Rigidbody 2D initially awake.
		/// </summary>
		public bool awake
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAwake(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAwake(entityID, ref value);
		}

		/// <summary>
		/// Continuous Collision Detection (CCD). Useful for very fast moving objects like bullet.
		/// Select this option to ensure Entities with Rigidbody 2Ds and Collider 2Ds do not pass through each other during a physics update.
		/// Note: This option takes more CPU time than default i.e. Discrete Collision Detection (DCD).
		/// </summary>
		public bool continuous
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetContinuous(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetContinuous(entityID, ref value);
		}

		/// <summary>
		/// Stops the Rigidbody 2D rotating around the Z world axis selectively.
		/// </summary>
		public bool freezeRotation
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetFreezeRotation(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetFreezeRotation(entityID, ref value);
		}

		/// <summary>
		/// Angular velocity in radians per second.
		/// </summary>
		public float angularVelocity
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAngularVelocity(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAngularVelocity(entityID, ref value);
		}

		/// <summary>
		/// Linear velocity in units per second.
		/// </summary>
		public Vector2 velocity
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetVelocity(entityID, out Vector2 v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetVelocity(entityID, ref value);
		}

		/// <summary>
		/// Apply a force to the Rigidbody.
		/// </summary>
		/// <param name="force">Force to apply.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyForce(Vector2 force) => InternalCalls.Rigidbody2DComponent_ApplyForceAtCenter(entityID, ref force);

		/// <summary>
		/// Apply a force to the Rigidbody at a given position in space.
		/// </summary>
		/// <param name="force">Force to apply.</param>
		/// <param name="point">Point to apply force at.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyForce(Vector2 force, Vector2 point) => InternalCalls.Rigidbody2DComponent_ApplyForce(entityID, ref force, ref point);

		/// <summary>
		/// Apply a linear impulse to the Rigidbody.
		/// </summary>
		/// <param name="force">Impulse to apply.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyLinearImpulse(Vector2 force) => InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseAtCenter(entityID, ref force);

		/// <summary>
		/// Apply a linear impulse to the Rigidbody at a given position in space.
		/// </summary>
		/// <param name="force">Impulse to apply.</param>
		/// <param name="point">Point to apply impulse at.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyLinearImpulse(Vector2 force, Vector2 point) => InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(entityID, ref force, ref point);

		/// <summary>
		/// Apply an angular impulse to the Rigidbody.
		/// </summary>
		/// <param name="force">Impulse to apply</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyAngularImpulse(float force) => InternalCalls.Rigidbody2DComponent_ApplyAngularImpulse(entityID, ref force);

		/// <summary>
		/// Apply a torque at the Rigidbody's centre of mass.
		/// </summary>
		/// <param name="torque"></param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyTorque(float torque) => InternalCalls.Rigidbody2DComponent_ApplyTorque(entityID, ref torque);

		/// <summary>
		/// Is the Rigidbody "awake"?
		/// </summary>
		/// <returns>Awake state</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsAwake()
		{
			InternalCalls.Rigidbody2DComponent_IsAwake(entityID, out bool v);
			return v;
		}

		/// <summary>
		/// Is the rigidbody "sleeping"?
		/// </summary>
		/// <returns>Sleep state</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsSleeping()
		{
			InternalCalls.Rigidbody2DComponent_IsSleeping(entityID, out bool v);
			return v;
		}

		/// <summary>
		/// Moves the Rigidbody to position.
		/// </summary>
		/// <param name="position">Target position</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void MovePosition(Vector2 position) => InternalCalls.Rigidbody2DComponent_MovePosition(entityID, ref position);

		/// <summary>
		/// Rotates the Rigidbody to angle (given in degrees).
		/// </summary>
		/// <param name="rotation">Target angle (in degrees)</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void MoveRotation(float rotation) => InternalCalls.Rigidbody2DComponent_MoveRotation(entityID, ref rotation);

		/// <summary>
		/// Make the Rigidbody "sleep".
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Sleep() => InternalCalls.Rigidbody2DComponent_Sleep(entityID);

		/// <summary>
		/// Disables the "sleeping" state of a Rigidbody.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void WakeUp() => InternalCalls.Rigidbody2DComponent_WakeUp(entityID);
	}

	/// <summary>
	/// A representation of audio sources in 3D.
	/// </summary>
	public class AudioSourceComponent : Component
	{
		/// <summary>
		/// Distance v Gain curve
		/// <br/>None: No attenuation is calculated
		/// <br/>Inverse: Inverse curve (Realistic and most suitable for games)
		/// <br/>Linear: Linear attenuation
		/// <br/>Exponential: Exponential curve
		/// </summary>
		public enum AttenuationModelType
		{
			None = 0,
			Inverse,
			Linear,
			Exponential
		};

		/// <summary>
		/// The volume of the audio source.
		/// </summary>
		public float volume
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetVolume(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetVolume(entityID, ref value);
		}

		/// <summary>
		/// The pitch of the audio source.
		/// </summary>
		public float pitch
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetPitch(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetPitch(entityID, ref value);
		}

		/// <summary>
		/// If set to true, the audio source will automatically start playing on awake.
		/// </summary>
		public bool playOnAwake
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetPlayOnAwake(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetPlayOnAwake(entityID, ref value);
		}

		/// <summary>
		/// Is the audio clip looping?
		/// </summary>
		public bool looping
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetLooping(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetLooping(entityID, ref value);
		}

		/// <summary>
		/// Enables or disables spatialization.
		/// </summary>
		public bool spatialization
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetSpatialization(entityID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetSpatialization(entityID, ref value);
		}

		/// <summary>
		/// Attenuation model describes how sound is attenuated over distance.
		/// </summary>
		public AttenuationModelType attenuationModel
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetAttenuationModel(entityID, out int v);
				return (AttenuationModelType)v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				int v = (int)value;
				InternalCalls.AudioSource_SetAttenuationModel(entityID, ref v);
			}
		}

		/// <summary>
		/// How the AudioSource attenuates over distance.
		/// </summary>
		public float rollOff
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetRollOff(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetRollOff(entityID, ref value);
		}

		/// <summary>
		/// Min gain the audio source can have.
		/// </summary>
		public float minGain
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMinGain(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMinGain(entityID, ref value);
		}

		/// <summary>
		/// Max gain attainable
		/// </summary>
		public float maxGain
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMaxGain(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMaxGain(entityID, ref value);
		}

		/// <summary>
		/// Within the Min distance the AudioSource will cease to grow louder in volume.
		/// </summary>
		public float minDistance
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMinDistance(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMinDistance(entityID, ref value);
		}

		/// <summary>
		/// (Logarithmic rolloff) MaxDistance is the distance a sound stops attenuating at.
		/// </summary>
		public float maxDistance
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMaxDistance(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMaxDistance(entityID, ref value);
		}

		/// <summary>
		/// Sets the inner angle (in degrees) of a 3d stereo or multichannel sound in speaker space.
		/// </summary>
		public float coneInnerAngle
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetConeInnerAngle(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetConeInnerAngle(entityID, ref value);
		}

		/// <summary>
		/// Sets the outer angle (in degrees) of a 3d stereo or multichannel sound in speaker space.
		/// </summary>
		public float coneOuterAngle
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetConeOuterAngle(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetConeOuterAngle(entityID, ref value);
		}

		/// <summary>
		/// Gain at the outermost angle of the spread.
		/// </summary>
		public float coneOuterGain
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetConeOuterGain(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetConeOuterGain(entityID, ref value);
		}

		/// <summary>
		/// Sets the Doppler scale for this AudioSource.
		/// </summary>
		public float dopplerFactor
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetDopplerFactor(entityID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetDopplerFactor(entityID, ref value);
		}

		/// <summary>
		/// Is the clip playing right now?
		/// </summary>
		/// <returns>Playing status</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsPlaying()
		{
			InternalCalls.AudioSource_IsPlaying(entityID, out bool v);
			return v;
		}

		/// <summary>
		/// Plays the clip.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Play() => InternalCalls.AudioSource_Play(entityID);

		/// <summary>
		/// 	Pauses playing the clip.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Pause() => InternalCalls.AudioSource_Pause(entityID);

		/// <summary>
		/// Unpause the paused playback of this AudioSource.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void UnPause() => InternalCalls.AudioSource_UnPause(entityID);

		/// <summary>
		/// Stops playing the clip.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Stop() => InternalCalls.AudioSource_Stop(entityID);
	}
}
