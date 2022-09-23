Scene: Untitled
Entities:
  - Entity: 7924704810687730180
    TagComponent:
      Tag: Camera
      Enabled: true
    TransformComponent:
      Translation: [0, 2.43823433, 17.7286148]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    CameraComponent:
      Camera:
        ProjectionType: 0
        PerspectiveFOV: 0.785398185
        PerspectiveNear: 0.00999999978
        PerspectiveFar: 1000
        OrthographicSize: 10
        OrthographicNear: -1
        OrthographicFar: 1
      Primary: true
      FixedAspectRatio: false
  - Entity: 17837761170342484294
    TagComponent:
      Tag: Cube
      Enabled: true
    TransformComponent:
      Translation: [0, 8.14545631, 0]
      Rotation: [0.459420562, -4.47034836e-08, -0.657063842]
      Scale: [1.00000012, 0.999999762, 1.00000012]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    RigidbodyComponent:
      Type: 2
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      AllowSleep: true
      Awake: true
      Continuous: false
      GravityScale: 1
    BoxColliderComponent:
      Size: [0.5, 0.5, 0.5]
      Offset: [0, 0, 0]
      IsSensor: false
      Density: 1
      Friction: 0.5
      Restitution: 0.5
    MeshComponent:
      Filepath: Assets\models\Primitives\Cube.fbx
      SubmeshIndex: 0
      CullMode: 1
  - Entity: 1304237688638990982
    TagComponent:
      Tag: Light
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [1.57079637, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    LightComponent:
      Type: 0
      UseColorTemperatureMode: false
      Temperature: 6570
      Color: [1, 1, 1]
      Intensity: 2
      Range: 1
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: 2
  - Entity: 13079083868139151945
    TagComponent:
      Tag: Ground
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [19.3137817, 1, 18.7381535]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    RigidbodyComponent:
      Type: 0
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      AllowSleep: true
      Awake: true
      Continuous: false
      GravityScale: 1
    BoxColliderComponent:
      Size: [0.5, 0.5, 0.5]
      Offset: [0, 0, 0]
      IsSensor: false
      Density: 1
      Friction: 0.5
      Restitution: 0
    MeshComponent:
      Filepath: Assets\models\Primitives\Cube.fbx
      SubmeshIndex: 0
      CullMode: 1
  - Entity: 4444126669705181132
    TagComponent:
      Tag: Cubemap
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
    SkyLightComponent:
      TexturePath: Assets\textures\Cubemap.hdr
      Intensity: 0.699999988
      Rotation: 0