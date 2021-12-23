Scene: Untitled
Entities:
  - Entity: 1032059750266340232
    TagComponent:
      Tag: Camera
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      Parent: 5580797897901373104
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
  - Entity: 5580797897901373104
    TagComponent:
      Tag: 3
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      Parent: 360708516231036225
      ChildrenCount: 1
      Children:
        0: 1032059750266340232
  - Entity: 14534088824447782919
    TagComponent:
      Tag: 2
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      Parent: 360708516231036225
      ChildrenCount: 0
      Children:
        {}
  - Entity: 3362028609394935696
    TagComponent:
      Tag: 1
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      Parent: 360708516231036225
      ChildrenCount: 0
      Children:
        {}
  - Entity: 360708516231036225
    TagComponent:
      Tag: Parent
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      Parent: 0
      ChildrenCount: 3
      Children:
        0: 3362028609394935696
        1: 14534088824447782919
        2: 5580797897901373104