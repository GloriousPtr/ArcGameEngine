Scene: Untitled
Entities:
  - Entity: 3437048607795030429
    TagComponent:
      Tag: Hook
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [0.100000001, 0.100000001, 0.100000001]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.876595736, 0.249923021, 0.249923021, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
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
      FreezeRotation: false
    BoxCollider2DComponent:
      IsSensor: false
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0
  - Entity: 11002321157373702071
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
        ProjectionType: Orthographic
        PerspectiveFOV: 0.785398185
        PerspectiveNear: 0.00999999978
        PerspectiveFar: 1000
        OrthographicSize: 10
        OrthographicNear: -1
        OrthographicFar: 1
      Primary: true
      FixedAspectRatio: false
  - Entity: 7392771533619020041
    TagComponent:
      Tag: Sprite
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, -2, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.19568257, 0.859574437, 0.153626055, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
      Type: Dynamic
      AutoMass: true
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
    SpringJoint2DComponent:
      EnableCollision: false
      ConnectedRigidbody: 3437048607795030429
      Anchor: [0, 0]
      ConnectedAnchor: [0, 0]
      AutoDistance: true
      Distance: 0
      MinDistance: 0
      MaxDistanceBy: 2
      Frequency: 3
      DampingRatio: 0
      BreakForce: 3.40282347e+38
  - Entity: 1136875182593621305
    TagComponent:
      Tag: Sprite
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [3.6597538, -2.28152037, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.19568257, 0.859574437, 0.153626055, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
      Type: Dynamic
      AutoMass: true
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
    DistanceJoint2DComponent:
      EnableCollision: false
      ConnectedRigidbody: 3437048607795030429
      Anchor: [0, 0]
      ConnectedAnchor: [0, 0]
      AutoDistance: false
      Distance: 2
      MinDistance: 2
      MaxDistanceBy: 2
      BreakForce: 3.40282347e+38
  - Entity: 4519022041829155700
    TagComponent:
      Tag: HingePlatform
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [-3.56999993, 1.9505372, 0]
      Rotation: [0, 0, 0]
      Scale: [3, 1, 0.100000001]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.876595736, 0.249923021, 0.249923021, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
      Type: Dynamic
      AutoMass: true
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
    HingeJoint2DComponent:
      EnableCollision: true
      ConnectedRigidbody: 6856658344230727925
      Anchor: [-1.5, -0.5]
      UseLimits: true
      LowerAngle: 0
      UpperAngle: 45.7669983
      UseMotor: true
      MotorSpeed: -100
      MaxMotorTorque: 50
      BreakForce: 3.40282347e+38
      BreakTorque: 3.40282347e+38
  - Entity: 6856658344230727925
    TagComponent:
      Tag: Platform
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [-5.61000013, 1.9505372, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 0.100000001]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.876595736, 0.249923021, 0.249923021, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
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
      FreezeRotation: false
    BoxCollider2DComponent:
      IsSensor: false
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0
  - Entity: 16781464313560543156
    TagComponent:
      Tag: Sprite
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [-4.42000008, 5, 0]
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
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: Textures/circle.png
    Rigidbody2DComponent:
      Type: Kinematic
      AutoMass: false
      Mass: 5
      LinearDrag: 0
      AngularDrag: 0.0500000007
      GravityScale: 1
      AllowSleep: true
      Awake: true
      Continuous: false
      Interpolation: true
      FreezeRotation: false
    CircleCollider2DComponent:
      IsSensor: false
      Radius: 0.5
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0
    ScriptComponent:
      ScriptCount: 1
      Scripts:
        0:
          Name: Sandbox.Player
          Fields:
            Speed: 209.276001