using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

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

	public class TagComponent : Component
	{
		public string tag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get => InternalCalls.TagComponent_GetTag(entityID);
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TagComponent_SetTag(entityID, value);
		}
	}

	public class TransformComponent : Component
	{
		public Transform transform
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.TransformComponent_GetTransform(entityID, out Transform result);
				return result;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TransformComponent_SetTransform(entityID, ref value);
		}
	}
	
	public class SpriteRendererComponent : Component
	{
		public Color tint
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(entityID, out Color t);
				return t;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.SpriteRendererComponent_SetColor(entityID, ref value);
		}

		public float tilingFactor
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.SpriteRendererComponent_GetTilingFactor(entityID, out float tiling);
				return tiling;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.SpriteRendererComponent_SetTilingFactor(entityID, ref value);
		}
	}

	public class Rigidbody2DComponent : Component
	{
		public enum BodyType { Static = 0, Kinematic, Dynamic }

		public BodyType type
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetBodyType(entityID, out int bodyType);
				return (BodyType)bodyType;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				int bodyType = (int)value;
				InternalCalls.Rigidbody2DComponent_SetBodyType(entityID, ref bodyType);
			}
		}

		public bool autoMass
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAutoMass(entityID, out bool useAutoMass);
				return useAutoMass;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAutoMass(entityID, ref value);
		}

		public float mass
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetMass(entityID, out float m);
				return m;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetMass(entityID, ref value);
		}

		public float linearDrag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetLinearDrag(entityID, out float drag);
				return drag;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetLinearDrag(entityID, ref value);
		}

		public float angularDrag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAngularDrag(entityID, out float drag);
				return drag;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAngularDrag(entityID, ref value);
		}

		public bool allowSleep
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAllowSleep(entityID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAllowSleep(entityID, ref value);
		}

		public bool awake
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAwake(entityID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAwake(entityID, ref value);
		}

		public bool continuous
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetContinuous(entityID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetContinuous(entityID, ref value);
		}

		public bool freezeRotation
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetFreezeRotation(entityID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetFreezeRotation(entityID, ref value);
		}

		public float gravityScale
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetGravityScale(entityID, out float gravityScale);
				return gravityScale;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetGravityScale(entityID, ref value);
		}

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

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyForce(Vector2 force) => InternalCalls.Rigidbody2DComponent_ApplyForceAtCenter(entityID, ref force);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyForce(Vector2 force, Vector2 point) => InternalCalls.Rigidbody2DComponent_ApplyForce(entityID, ref force, ref point);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyLinearImpulse(Vector2 force) => InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseAtCenter(entityID, ref force);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyLinearImpulse(Vector2 force, Vector2 point) => InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(entityID, ref force, ref point);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyAngularImpulse(float force) => InternalCalls.Rigidbody2DComponent_ApplyAngularImpulse(entityID, ref force);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyTorque(float torque) => InternalCalls.Rigidbody2DComponent_ApplyTorque(entityID, ref torque);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsAwake()
		{
			InternalCalls.Rigidbody2DComponent_IsAwake(entityID, out bool awake);
			return awake;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsSleeping()
		{
			InternalCalls.Rigidbody2DComponent_IsSleeping(entityID, out bool sleeping);
			return sleeping;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void MovePosition(Vector2 position) => InternalCalls.Rigidbody2DComponent_MovePosition(entityID, ref position);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void MoveRotation(float rotation) => InternalCalls.Rigidbody2DComponent_MoveRotation(entityID, ref rotation);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Sleep() => InternalCalls.Rigidbody2DComponent_Sleep(entityID);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void WakeUp() => InternalCalls.Rigidbody2DComponent_WakeUp(entityID);
	}

	public class AudioSourceComponent : Component
	{
		public enum AttenuationModelType
		{
			None = 0,
			Inverse,
			Linear,
			Exponential
		};

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
				int type = (int)value;
				InternalCalls.AudioSource_SetAttenuationModel(entityID, ref type);
			}
		}

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

		public float doplerFactor
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

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsPlaying()
		{
			InternalCalls.AudioSource_IsPlaying(entityID, out bool v);
			return v;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Play() => InternalCalls.AudioSource_Play(entityID);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Pause() => InternalCalls.AudioSource_Pause(entityID);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void UnPause() => InternalCalls.AudioSource_UnPause(entityID);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Stop() => InternalCalls.AudioSource_Stop(entityID);
	}
}
