Scene: Untitled
Entities:
  - Entity: 9795675143866417012
    TagComponent:
      Tag: Box
      Enabled: true
    TransformComponent:
      Translation: [-5.78105068, 5.13415051, 0]
      Rotation: [0, 0, -0.921533823]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.800000012, 0.228085086, 0.228085086, 1]
      TexturePath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 2
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
      Restitution: 0.5
    ScriptComponent:
      ScriptCount: 0
      Scripts:
        {}
  - Entity: 8164303301525394496
    TagComponent:
      Tag: Circle
      Enabled: true
    TransformComponent:
      Translation: [-8.61093616, 5.09371948, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [1, 1, 1, 1]
      TexturePath: Assets\textures\circle.png
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 2
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
    CircleCollider2DComponent:
      IsSensor: false
      Radius: 0.5
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0.5
  - Entity: 2391845101551189660
    TagComponent:
      Tag: Ground
      Enabled: true
    TransformComponent:
      Translation: [-6.26217461, -5.0128684, 0]
      Rotation: [0, 0, 0]
      Scale: [11.5878592, 3.19387722, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.297582626, 0.842553198, 0.376429349, 1]
      TexturePath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 0
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
      Restitution: 0.5
  - Entity: 8816232855549296649
    TagComponent:
      Tag: Box
      Enabled: true
    TransformComponent:
      Translation: [7.38603306, 5.3796463, 0]
      Rotation: [0, 0, 2.37671542]
      Scale: [0.99999994, 0.99999994, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.829181492, 0.646030903, 0.348197222, 1]
      TexturePath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 2
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
      Friction: 0
      Restitution: 0
  - Entity: 10566615274522482950
    TagComponent:
      Tag: Water
      Enabled: true
    TransformComponent:
      Translation: [6.71770573, -4.91728497, 0]
      Rotation: [0, 0, 0]
      Scale: [11.6558533, 3, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.199999988, 0.76510638, 1, 0.53191489]
      TexturePath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 1
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
      IsSensor: true
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0
      Restitution: 0
    BuoyancyEffector2DComponent:
      Density: 2
      DragMultiplier: 1
      FlipGravity: false
      FlowMagnitude: 0
      FlowAngle: 0
  - Entity: 1324993968739374613
    TagComponent:
      Tag: Camera
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
        OrthographicSize: 15
        OrthographicNear: -1
        OrthographicFar: 1
      Primary: true
      FixedAspectRatio: false
  - Entity: 13306146136167739395
    TagComponent:
      Tag: Circle
      Enabled: true
    TransformComponent:
      Translation: [9.80843163, 5.21560526, 0]
      Rotation: [0, 0, 0.431065619]
      Scale: [0.999999642, 0.999999642, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [1, 1, 1, 1]
      TexturePath: Assets\textures\circle.png
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 2
      AutoMass: false
      Mass: 1
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
      Friction: 0
      Restitution: 0