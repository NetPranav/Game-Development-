import unreal, json

out = {}

# 1. Inspect DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    vis = da.get_editor_property("visuals")
    out["DA_PlayerWeapon"] = {
        "attachment_socket": str(vis.get_editor_property("attachment_socket")),
        "hand_offset_loc": str(vis.get_editor_property("hand_offset").translation),
        "hand_offset_rot": str(vis.get_editor_property("hand_offset").rotation.rotator()),
        "hand_offset_scale": str(vis.get_editor_property("hand_offset").scale3d),
        "holster_socket": str(vis.get_editor_property("holster_socket")),
        "mesh": str(vis.get_editor_property("static_mesh")),
    }

# 2. Inspect Robot Scout SK
robot_sk = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
if robot_sk:
    skeleton = robot_sk.get_editor_property("skeleton")
    out["robot_sk"] = {
        "skeleton": str(skeleton),
        "num_bones": skeleton.get_reference_skeleton().get_num() if hasattr(skeleton, 'get_reference_skeleton') else -1,
    }
    # Check sockets on robot mesh and skeleton
    sk_sockets = [str(s.socket_name) for s in robot_sk.get_editor_property("mesh_clothing_assets")] if hasattr(robot_sk, 'mesh_clothing_assets') else []
    # Let's check sockets list
    try:
        mesh_sockets = [str(s.socket_name) for s in robot_sk.get_mesh_sockets()]
        out["robot_mesh_sockets"] = mesh_sockets
    except Exception as e:
        out["robot_mesh_sockets_err"] = str(e)
    try:
        skel_sockets = [str(s.socket_name) for s in skeleton.get_editor_property("sockets")]
        out["robot_skel_sockets"] = skel_sockets
    except Exception as e:
        out["robot_skel_sockets_err"] = str(e)

# 3. Inspect BP_RobotScout components
sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
bp_scout = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
if bp_scout:
    scout_comps = []
    for h in sub.k2_gather_subobject_data_for_blueprint(bp_scout):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if o:
            info = {
                "class": o.get_class().get_name(),
                "name": o.get_name(),
            }
            if isinstance(o, unreal.SkeletalMeshComponent):
                info["skel_mesh"] = str(o.get_editor_property("skeletal_mesh_asset"))
                info["anim_class"] = str(o.get_editor_property("anim_class"))
            scout_comps.append(info)
    out["scout_comps"] = scout_comps

# 4. Inspect BP_UE4_Mannequin components for comparison
bp_ue4 = unreal.load_asset("/Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin")
if bp_ue4:
    ue4_comps = []
    for h in sub.k2_gather_subobject_data_for_blueprint(bp_ue4):
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(d)
        if o:
            info = {
                "class": o.get_class().get_name(),
                "name": o.get_name(),
            }
            if isinstance(o, unreal.SkeletalMeshComponent):
                info["skel_mesh"] = str(o.get_editor_property("skeletal_mesh_asset"))
                info["anim_class"] = str(o.get_editor_property("anim_class"))
            ue4_comps.append(info)
    out["ue4_comps"] = ue4_comps

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_robot_sword.json", "w") as f:
    json.dump(out, f, indent=2)
print("INSPECT_ROBOT_SWORD_DONE")
