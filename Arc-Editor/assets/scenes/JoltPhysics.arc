Scene: Untitled
Entities:
  - Entity: 11483124955699556981
    TagComponent:
      Tag: RootNode
      Enabled: true
    TransformComponent:
      Translation: [0.304588795, -0.413956165, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 8705794451217545590
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
    SphereColliderComponent:
      Radius: 0.5
      Offset: [0, 0, 0]
      IsSensor: false
      Density: 1
      Friction: 0.5
      Restitution: 0
    MeshComponent:
      Filepath: Assets\models\Primitives\Sphere.fbx
      SubmeshIndex: 0
      CullMode: 1
  - Entity: 8705794451217545590
    TagComponent:
      Tag: Sphere
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 1
      Children:
        0: 11483124955699556981
  - Entity: 2789078515034820223
    TagComponent:
      Tag: Cube
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 1
      Children:
        0: 15603952106719446287
  - Entity: 15603952106719446287
    TagComponent:
      Tag: RootNode
      Enabled: true
    TransformComponent:
      Translation: [8.20967293, 5.73394251, 7.81906877e-08]
      Rotation: [9.5446886e-09, 1.05479281e-09, -0.220128343]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 2789078515034820223
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
      Friction: 1
      Restitution: 0.5
    MeshComponent:
      Filepath: Assets\models\Primitives\Cube.fbx
      SubmeshIndex: 0
      CullMode: 1
  - Entity: 2913474625882804317
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
  - Entity: 7488003786625717088
    TagComponent:
      Tag: Cube
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 1
      Children:
        0: 16784490985120181073
  - Entity: 16784490985120181073
    TagComponent:
      Tag: RootNode
      Enabled: true
    TransformComponent:
      Translation: [0, -5.23015213, 0]
      Rotation: [0, 0, 0.445557922]
      Scale: [12.5196524, 0.273816586, 8.81022453]
    RelationshipComponent:
      Parent: 7488003786625717088
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
  - Entity: 10802558725339819539
    TagComponent:
      Tag: Camera
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 23.2929039]
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