Scene: Untitled
Entities:
  - Entity: 13306146136167739395
    TagComponent:
      Tag: Circle
      Layer: 2
      Enabled: true
    TransformComponent:
      Translation: [7.30999994, 5.21560526, 0]
      Rotation: [0, 0, 0.431065619]
      Scale: [0.999999642, 0.999999642, 1]
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
      Type: Dynamic
      AutoMass: false
      Mass: 1
      LinearDrag: 0
      AngularDrag: 5
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
      Friction: 0.800000012
      Restitution: 0.200000003
  - Entity: 1324993968739374613
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
        OrthographicSize: 15
        OrthographicNear: -1
        OrthographicFar: 1
      Primary: true
      FixedAspectRatio: false
  - Entity: 10566615274522482950
    TagComponent:
      Tag: Water
      Layer: 4
      Enabled: true
    TransformComponent:
      Translation: [4.52125072, -3.30789065, 0]
      Rotation: [0, 0, 0]
      Scale: [8.96000004, 6.5999999, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.199999988, 0.76510638, 1, 0.53191489]
      SortingOrder: 1
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
      Type: Kinematic
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
  - Entity: 8816232855549296649
    TagComponent:
      Tag: Box
      Layer: 2
      Enabled: true
    TransformComponent:
      Translation: [4.57744217, 4.33827591, 0]
      Rotation: [0, 0, 2.37671518]
      Scale: [0.999998748, 0.999998748, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.829181492, 0.646030903, 0.348197222, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""
    Rigidbody2DComponent:
      Type: Dynamic
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.5
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
      Friction: 0.800000012
      Restitution: 0.100000001
  - Entity: 2391845101551189660
    TagComponent:
      Tag: Ground
      Layer: 4
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
      Restitution: 0.5
  - Entity: 8164303301525394496
    TagComponent:
      Tag: Circle
      Layer: 2
      Enabled: true
    TransformComponent:
      Translation: [-5.98972368, 5.47240162, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.758974373, 0.613406956, 0.202393159, 1]
      SortingOrder: 0
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: Textures/circle.png
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
    CircleCollider2DComponent:
      IsSensor: false
      Radius: 0.5
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0.5
  - Entity: 9795675143866417012
    TagComponent:
      Tag: Box
      Layer: 2
      Enabled: true
    TransformComponent:
      Translation: [-6.72261047, 4.20066595, 0]
      Rotation: [0, 0, -0.921533883]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.800000012, 0.228085086, 0.228085086, 1]
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
      Restitution: 0.5
    ScriptComponent:
      ScriptCount: 0
      Scripts:
        {}
  - Entity: 10678094010813816488
    TagComponent:
      Tag: BG
      Layer: 2
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [-37.4000015, -25.6000004, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.297097564, 0.273872435, 0.651282072, 1]
      SortingOrder: -1
      Tiling: [1, 1]
      Offset: [0, 0]
      TexturePath: ""