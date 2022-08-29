using System.Runtime.CompilerServices;

namespace ArcEngine
{
	public interface IComponent
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal ulong GetEntityID();
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal void SetEntity(Entity e);
	}

	public abstract class Component : IComponent
	{
		internal Entity entity;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		ulong IComponent.GetEntityID() => entity.ID;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		void IComponent.SetEntity(Entity e) => entity = e;
	}

	public class TagComponent : Component
	{
		public string tag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get => InternalCalls.TagComponent_GetTag(entity.ID);
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TagComponent_SetTag(entity.ID, value);
		}
	}

	public class TransformComponent : Component
	{
		public Transform transform
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.TransformComponent_GetTransform(entity.ID, out Transform result);
				return result;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.TransformComponent_SetTransform(entity.ID, ref value);
		}
	}
	
	public class SpriteRendererComponent : Component
	{
		public Color tint
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(entity.ID, out Color t);
				return t;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.SpriteRendererComponent_SetColor(entity.ID, ref value);
		}

		public float tilingFactor
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.SpriteRendererComponent_GetTilingFactor(entity.ID, out float tiling);
				return tiling;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.SpriteRendererComponent_SetTilingFactor(entity.ID, ref value);
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
				InternalCalls.Rigidbody2DComponent_GetBodyType(entity.ID, out int bodyType);
				return (BodyType)bodyType;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				int bodyType = (int)value;
				InternalCalls.Rigidbody2DComponent_SetBodyType(entity.ID, ref bodyType);
			}
		}

		public bool autoMass
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAutoMass(entity.ID, out bool useAutoMass);
				return useAutoMass;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAutoMass(entity.ID, ref value);
		}

		public float mass
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetMass(entity.ID, out float m);
				return m;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetMass(entity.ID, ref value);
		}

		public float linearDrag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetLinearDrag(entity.ID, out float drag);
				return drag;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetLinearDrag(entity.ID, ref value);
		}

		public float angularDrag
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAngularDrag(entity.ID, out float drag);
				return drag;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAngularDrag(entity.ID, ref value);
		}

		public bool allowSleep
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAllowSleep(entity.ID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAllowSleep(entity.ID, ref value);
		}

		public bool awake
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetAwake(entity.ID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetAwake(entity.ID, ref value);
		}

		public bool continuous
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetContinuous(entity.ID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetContinuous(entity.ID, ref value);
		}

		public bool freezeRotation
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetFreezeRotation(entity.ID, out bool a);
				return a;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetFreezeRotation(entity.ID, ref value);
		}

		public float gravityScale
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.Rigidbody2DComponent_GetGravityScale(entity.ID, out float gravityScale);
				return gravityScale;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.Rigidbody2DComponent_SetGravityScale(entity.ID, ref value);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyForce(Vector2 force) => InternalCalls.Rigidbody2DComponent_ApplyForceAtCenter(entity.ID, ref force);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyForce(Vector2 force, Vector2 point) => InternalCalls.Rigidbody2DComponent_ApplyForce(entity.ID, ref force, ref point);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyLinearImpulse(Vector2 force) => InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseAtCenter(entity.ID, ref force);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyLinearImpulse(Vector2 force, Vector2 point) => InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(entity.ID, ref force, ref point);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyAngularImpulse(float force) => InternalCalls.Rigidbody2DComponent_ApplyAngularImpulse(entity.ID, ref force);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void ApplyTorque(float torque) => InternalCalls.Rigidbody2DComponent_ApplyTorque(entity.ID, ref torque);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsAwake()
		{
			InternalCalls.Rigidbody2DComponent_IsAwake(entity.ID, out bool awake);
			return awake;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsSleeping()
		{
			InternalCalls.Rigidbody2DComponent_IsSleeping(entity.ID, out bool sleeping);
			return sleeping;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void MovePosition(Vector2 position) => InternalCalls.Rigidbody2DComponent_MovePosition(entity.ID, ref position);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void MoveRotation(float rotation) => InternalCalls.Rigidbody2DComponent_MoveRotation(entity.ID, ref rotation);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Sleep() => InternalCalls.Rigidbody2DComponent_Sleep(entity.ID);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void WakeUp() => InternalCalls.Rigidbody2DComponent_WakeUp(entity.ID);
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
				InternalCalls.AudioSource_GetVolume(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetVolume(entity.ID, ref value);
		}

		public float pitch
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetPitch(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetPitch(entity.ID, ref value);
		}

		public bool playOnAwake
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetPlayOnAwake(entity.ID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetPlayOnAwake(entity.ID, ref value);
		}

		public bool looping
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetLooping(entity.ID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetLooping(entity.ID, ref value);
		}

		public bool spatialization
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetSpatialization(entity.ID, out bool v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetSpatialization(entity.ID, ref value);
		}

		public AttenuationModelType attenuationModel
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetAttenuationModel(entity.ID, out int v);
				return (AttenuationModelType)v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set
			{
				int type = (int)value;
				InternalCalls.AudioSource_SetAttenuationModel(entity.ID, ref type);
			}
		}

		public float rollOff
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetRollOff(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetRollOff(entity.ID, ref value);
		}

		public float minGain
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMinGain(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMinGain(entity.ID, ref value);
		}

		public float maxGain
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMaxGain(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMaxGain(entity.ID, ref value);
		}

		public float minDistance
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMinDistance(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMinDistance(entity.ID, ref value);
		}

		public float maxDistance
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetMaxDistance(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetMaxDistance(entity.ID, ref value);
		}

		public float coneInnerAngle
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetConeInnerAngle(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetConeInnerAngle(entity.ID, ref value);
		}

		public float coneOuterAngle
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetConeOuterAngle(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetConeOuterAngle(entity.ID, ref value);
		}

		public float coneOuterGain
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetConeOuterGain(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetConeOuterGain(entity.ID, ref value);
		}

		public float doplerFactor
		{
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			get
			{
				InternalCalls.AudioSource_GetDopplerFactor(entity.ID, out float v);
				return v;
			}
			[MethodImpl(MethodImplOptions.AggressiveInlining)]
			set => InternalCalls.AudioSource_SetDopplerFactor(entity.ID, ref value);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool IsPlaying()
		{
			InternalCalls.AudioSource_IsPlaying(entity.ID, out bool v);
			return v;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Play() => InternalCalls.AudioSource_Play(entity.ID);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Pause() => InternalCalls.AudioSource_Pause(entity.ID);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void UnPause() => InternalCalls.AudioSource_UnPause(entity.ID);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Stop() => InternalCalls.AudioSource_Stop(entity.ID);
	}
}
