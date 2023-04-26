Scene: Untitled
Entities:
  - Entity: 10680150244262521285
    TagComponent:
      Tag: Green
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [-10.7103243, 1.32837677, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 13464390929186834787
      ChildrenCount: 0
      Children:
        {}
    LightComponent:
      Type: Point
      UseColorTemperatureMode: false
      Temperature: 6570
      Color: [0.160919666, 1, 0]
      Intensity: 20
      Range: 10
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: UltraSoft
  - Entity: 13464390929186834787
    TagComponent:
      Tag: PointLights
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 6631891353277229707
      ChildrenCount: 2
      Children:
        0: 14301618019071964031
        1: 10680150244262521285
  - Entity: 14301618019071964031
    TagComponent:
      Tag: Blue
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 2, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 13464390929186834787
      ChildrenCount: 0
      Children:
        {}
    LightComponent:
      Type: Point
      UseColorTemperatureMode: false
      Temperature: 6570
      Color: [0, 0.275862217, 1]
      Intensity: 65
      Range: 5
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: UltraSoft
  - Entity: 1900647786115943484
    TagComponent:
      Tag: sponza
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 25
      Children:
        0: 16484834023453057403
        1: 9838241152144991898
        2: 861998861965754813
        3: 15751752526908026905
        4: 5794960747746557335
        5: 14653948611369502714
        6: 5036459482807748356
        7: 575903852694178809
        8: 4105027073037869382
        9: 16536703140984935075
        10: 13138774069436837992
        11: 14269506995403230443
        12: 16964542492198351252
        13: 7065221905690173654
        14: 13285847275102555933
        15: 5247346448757476093
        16: 2508669298851889244
        17: 13656944375313487358
        18: 11413275653437956207
        19: 6225032001798920303
        20: 9497956432310012474
        21: 12340539741192413070
        22: 2171958093859330569
        23: 12896254358261950236
        24: 3496615779418459996
  - Entity: 6631891353277229707
    TagComponent:
      Tag: Sky Light
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 1
      Children:
        0: 13464390929186834787
    SkyLightComponent:
      TexturePath: Textures/Cubemap.hdr
      Intensity: 0.699999988
      Rotation: 0
  - Entity: 3496615779418459996
    TagComponent:
      Tag: sponza_sponza_vase_round
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 24
  - Entity: 12896254358261950236
    TagComponent:
      Tag: sponza_sponza_vase_hanging
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 23
  - Entity: 2171958093859330569
    TagComponent:
      Tag: sponza_sponza_vase
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 22
  - Entity: 12340539741192413070
    TagComponent:
      Tag: sponza_sponza_roof
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 21
  - Entity: 9497956432310012474
    TagComponent:
      Tag: sponza_sponza_leaf
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 20
  - Entity: 6225032001798920303
    TagComponent:
      Tag: sponza_sponza_floor
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 19
  - Entity: 11413275653437956207
    TagComponent:
      Tag: sponza_sponza_flagpole
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 18
  - Entity: 13656944375313487358
    TagComponent:
      Tag: sponza_sponza_fabric_g
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 17
  - Entity: 2508669298851889244
    TagComponent:
      Tag: sponza_sponza_fabric_f
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 16
  - Entity: 5247346448757476093
    TagComponent:
      Tag: sponza_sponza_fabric_e
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 15
  - Entity: 13285847275102555933
    TagComponent:
      Tag: sponza_sponza_fabric_d
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 14
  - Entity: 7065221905690173654
    TagComponent:
      Tag: sponza_sponza_fabric_c
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 13
  - Entity: 16964542492198351252
    TagComponent:
      Tag: sponza_sponza_fabric_a
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 12
  - Entity: 14269506995403230443
    TagComponent:
      Tag: sponza_sponza_details
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 11
  - Entity: 13138774069436837992
    TagComponent:
      Tag: sponza_sponza_column_c
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 10
  - Entity: 16536703140984935075
    TagComponent:
      Tag: sponza_sponza_column_b
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 9
  - Entity: 4105027073037869382
    TagComponent:
      Tag: sponza_sponza_column_a
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 8
  - Entity: 575903852694178809
    TagComponent:
      Tag: sponza_sponza_chain
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 7
  - Entity: 5036459482807748356
    TagComponent:
      Tag: sponza_sponza_ceiling
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 6
  - Entity: 14653948611369502714
    TagComponent:
      Tag: sponza_sponza_bricks
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 5
  - Entity: 5794960747746557335
    TagComponent:
      Tag: sponza_sponza_arch
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 4
  - Entity: 15751752526908026905
    TagComponent:
      Tag: sponza_sponza_VasePlant
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 3
  - Entity: 861998861965754813
    TagComponent:
      Tag: sponza_sponza_Material__47
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 2
  - Entity: 9838241152144991898
    TagComponent:
      Tag: sponza_sponza_Material__298
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 1
  - Entity: 16484834023453057403
    TagComponent:
      Tag: sponza_sponza_Material__25
      Layer: 0
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 1900647786115943484
      ChildrenCount: 0
      Children:
        {}
    MeshComponent:
      Filepath: Models/sponza/sponza.obj
      SubmeshIndex: 0