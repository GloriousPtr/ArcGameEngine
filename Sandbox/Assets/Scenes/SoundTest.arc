Scene: Untitled
Entities:
  - Entity: 3351491715015009777
    TagComponent:
      Tag: Sprite
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [1, 1, 1, 1]
      SortingOrder: 0
      TilingFactor: 1
      TexturePath: ""
    Rigidbody2DComponent:
      Type: 0
      AutoMass: false
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      GravityScale: 1
      AllowSleep: false
      Awake: true
      Continuous: false
      Interpolation: true
      FreezeRotation: false
    BoxCollider2DComponent:
      IsSensor: false
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0
    AudioSourceComponent:
      Filepath: Audio\ding.wav
      VolumeMultiplier: 1
      PitchMultiplier: 1
      PlayOnAwake: true
      Looping: true
      Spatialization: true
      AttenuationModel: 1
      RollOff: 1
      MinGain: 0
      MaxGain: 1
      MinDistance: 0.300000012
      MaxDistance: 1000
      ConeInnerAngle: 360
      ConeOuterAngle: 360
      ConeOuterGain: 0
      DopplerFactor: 1
  - Entity: 4794170410324563299
    TagComponent:
      Tag: Camera
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    CameraComponent:
      Camera:
        ProjectionType: 1
        PerspectiveFOV: 0.785398185
        PerspectiveNear: 0.00999999978
        PerspectiveFar: 1000
        OrthographicSize: 10
        OrthographicNear: -1
        OrthographicFar: 1
      Primary: true
      FixedAspectRatio: false
    ScriptComponent:
      ScriptCount: 1
      Scripts:
        0:
          Name: Sandbox.Camera
          Fields:
            {}
    AudioListenerComponent:
      Active: true
      ConeInnerAngle: 360
      ConeOuterAngle: 360
      ConeOuterGain: 0