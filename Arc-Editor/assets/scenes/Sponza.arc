Scene: Untitled
Entities:
  - Entity: 10680150244262521285
    TagComponent:
      Tag: Green
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
      Type: 1
      UseColorTempratureMode: false
      Color: [0.160919666, 1, 0]
      Intensity: 20
      Range: 10
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: 2
  - Entity: 13464390929186834787
    TagComponent:
      Tag: PointLights
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
      Type: 1
      UseColorTempratureMode: false
      Color: [0, 0.275862217, 1]
      Intensity: 65
      Range: 5
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: 2
  - Entity: 34557238050364489
    TagComponent:
      Tag: DirLight
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [1.00530958, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 6631891353277229707
      ChildrenCount: 0
      Children:
        {}
    LightComponent:
      Type: 0
      UseColorTempratureMode: false
      Color: [1, 0.996510088, 0.980556488]
      Intensity: 20
      Range: 1
      CutOffAngle: 12.5
      OuterCutOffAngle: 17.5
      ShadowQuality: 2
  - Entity: 6631891353277229707
    TagComponent:
      Tag: Sky Light
      Enabled: true
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    RelationshipComponent:
      Parent: 0
      ChildrenCount: 2
      Children:
        0: 34557238050364489
        1: 13464390929186834787
    SkyLightComponent:
      TexturePath: Assets\textures\Cubemap.hdr
      Intensity: 0.699999988
      Rotation: 0
  - Entity: 3496615779418459996
    TagComponent:
      Tag: sponza_sponza_vase_round
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 24
      CullMode: 1
  - Entity: 12896254358261950236
    TagComponent:
      Tag: sponza_sponza_vase_hanging
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 23
      CullMode: 1
  - Entity: 2171958093859330569
    TagComponent:
      Tag: sponza_sponza_vase
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 22
      CullMode: 1
  - Entity: 12340539741192413070
    TagComponent:
      Tag: sponza_sponza_roof
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 21
      CullMode: 1
  - Entity: 9497956432310012474
    TagComponent:
      Tag: sponza_sponza_leaf
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 20
      CullMode: 1
  - Entity: 6225032001798920303
    TagComponent:
      Tag: sponza_sponza_floor
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 19
      CullMode: 1
  - Entity: 11413275653437956207
    TagComponent:
      Tag: sponza_sponza_flagpole
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 18
      CullMode: 1
  - Entity: 13656944375313487358
    TagComponent:
      Tag: sponza_sponza_fabric_g
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 17
      CullMode: 1
  - Entity: 2508669298851889244
    TagComponent:
      Tag: sponza_sponza_fabric_f
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 16
      CullMode: 1
  - Entity: 5247346448757476093
    TagComponent:
      Tag: sponza_sponza_fabric_e
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 15
      CullMode: 1
  - Entity: 13285847275102555933
    TagComponent:
      Tag: sponza_sponza_fabric_d
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 14
      CullMode: 1
  - Entity: 7065221905690173654
    TagComponent:
      Tag: sponza_sponza_fabric_c
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 13
      CullMode: 1
  - Entity: 16964542492198351252
    TagComponent:
      Tag: sponza_sponza_fabric_a
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 12
      CullMode: 1
  - Entity: 14269506995403230443
    TagComponent:
      Tag: sponza_sponza_details
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 11
      CullMode: 1
  - Entity: 13138774069436837992
    TagComponent:
      Tag: sponza_sponza_column_c
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 10
      CullMode: 1
  - Entity: 16536703140984935075
    TagComponent:
      Tag: sponza_sponza_column_b
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 9
      CullMode: 1
  - Entity: 4105027073037869382
    TagComponent:
      Tag: sponza_sponza_column_a
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 8
      CullMode: 1
  - Entity: 575903852694178809
    TagComponent:
      Tag: sponza_sponza_chain
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 7
      CullMode: 1
  - Entity: 5036459482807748356
    TagComponent:
      Tag: sponza_sponza_ceiling
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 6
      CullMode: 1
  - Entity: 14653948611369502714
    TagComponent:
      Tag: sponza_sponza_bricks
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 5
      CullMode: 1
  - Entity: 5794960747746557335
    TagComponent:
      Tag: sponza_sponza_arch
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 4
      CullMode: 1
  - Entity: 15751752526908026905
    TagComponent:
      Tag: sponza_sponza_VasePlant
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 3
      CullMode: 1
  - Entity: 861998861965754813
    TagComponent:
      Tag: sponza_sponza_Material__47
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 2
      CullMode: 1
  - Entity: 9838241152144991898
    TagComponent:
      Tag: sponza_sponza_Material__298
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 1
      CullMode: 1
  - Entity: 16484834023453057403
    TagComponent:
      Tag: sponza_sponza_Material__25
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
      Filepath: Assets\models\sponza\sponza.assbin
      SubmeshIndex: 0
      CullMode: 1
  - Entity: 1900647786115943484
    TagComponent:
      Tag: sponza
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