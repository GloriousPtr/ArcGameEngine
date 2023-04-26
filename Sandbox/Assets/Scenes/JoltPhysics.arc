Scene: Untitled
Entities:
  - Entity: 7924704810687730180
    TagComponent:
      Tag: Camera
      Layer: 0
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
        ProjectionType: Perspective
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
      Layer: 0
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
      Type: Dynamic
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      GravityScale: 1
      AllowSleep: true
      Awake: true
      Continuous: false
      Interpolation: true
      IsSensor: false
    BoxColliderComponent:
      Size: [0.5, 0.5, 0.5]
      Offset: [0, 0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0.5
    MeshComponent:
      Filepath: Resources/Models/Primitives/Cube.obj
      SubmeshIndex: 0
  - Entity: 1304237688638990982
    TagComponent:
      Tag: Light
      Layer: 0
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
      Type: Directional
      UseColorTemperatureMode: false
      Temperature: 6570
      Color: [1, 1, 1]
      Intensity: 2
      Range: 1
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: UltraSoft
  - Entity: 13079083868139151945
    TagComponent:
      Tag: Ground
      Layer: 0
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
      Type: Static
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      GravityScale: 1
      AllowSleep: true
      Awake: true
      Continuous: false
      Interpolation: true
      IsSensor: false
    BoxColliderComponent:
      Size: [0.5, 0.5, 0.5]
      Offset: [0, 0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0
    MeshComponent:
      Filepath: Resources/Models/Primitives/Cube.obj
      SubmeshIndex: 0
  - Entity: 4444126669705181132
    TagComponent:
      Tag: Cubemap
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
    SkyLightComponent:
      TexturePath: Textures/Cubemap.hdr
      Intensity: 0.699999988
      Rotation: 0
  - Entity: 1788361504626844316
    TagComponent:
      Tag: Sphere
      Layer: 2
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
    MeshComponent:
      Filepath: Resources/Models/Primitives/Sphere.obj
      SubmeshIndex: 0