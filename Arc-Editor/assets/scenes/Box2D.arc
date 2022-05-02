Scene: Untitled
Entities:
  - Entity: 13306146136167739395
    TagComponent:
      Tag: Sprite
    TransformComponent:
      Translation: [-0.540859818, 1.45595407, 0]
      Rotation: [0, 0, 0.431065738]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.800000012, 0.200000003, 0.200000003, 1]
      TextureFilepath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 2
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      AllowSleep: false
      Awake: true
      Continuous: false
      FreezeRotation: false
      GravityScale: 1
    BoxCollider2DComponent:
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0.649999976
      RestitutionThreshold: 0.5
  - Entity: 1324993968739374613
    TagComponent:
      Tag: Camera
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
  - Entity: 10566615274522482950
    TagComponent:
      Tag: Ground
    TransformComponent:
      Translation: [0, -2.02967358, 0]
      Rotation: [0, 0, 0]
      Scale: [6.34841347, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.272219539, 0.807829201, 0.264485002, 1]
      TextureFilepath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 0
      AutoMass: true
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.0500000007
      AllowSleep: true
      Awake: true
      Continuous: false
      FreezeRotation: false
      GravityScale: 1
    BoxCollider2DComponent:
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0
      RestitutionThreshold: 0.5
  - Entity: 8816232855549296649
    TagComponent:
      Tag: Box
    TransformComponent:
      Translation: [1.20071387, 1.70085049, 0]
      Rotation: [0, 0, 0.92857939]
      Scale: [0.999999881, 0.999999881, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 0
      Children:
        {}
    SpriteRendererComponent:
      Color: [0.829181492, 0.646030903, 0.348197222, 1]
      TextureFilepath: ""
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: 2
      AutoMass: false
      Mass: 3
      LinearDrag: 0
      AngularDrag: 0.0500000007
      AllowSleep: false
      Awake: true
      Continuous: false
      FreezeRotation: false
      GravityScale: 1
    BoxCollider2DComponent:
      Size: [0.5, 0.5]
      Offset: [0, 0]
      Density: 1
      Friction: 0.5
      Restitution: 0.699999988
      RestitutionThreshold: 0.5