import unreal, json, traceback

out = {}

def safe_prop(obj, name):
    try:
        return str(obj.get_editor_property(name))
    except Exception as e:
        return None

bp_scout = unreal.load_asset("/Game/RR/Blueprints/BP_RobotScout")
if bp_scout:
    sub = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = sub.k2_gather_subobject_data_for_blueprint(bp_scout)
    comps_data = []
    for h in handles:
        d = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
        o = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(d)
        if o:
            item = {
                "name": o.get_name(),
                "class": o.get_class().get_name(),
            }
            if isinstance(o, unreal.SceneComponent):
                item["rel_loc"] = safe_prop(o, "relative_location")
                item["rel_rot"] = safe_prop(o, "relative_rotation")
            if isinstance(o, unreal.SkeletalMeshComponent):
                item["skel_mesh"] = safe_prop(o, "skeletal_mesh_asset")
                item["anim_class"] = safe_prop(o, "anim_class")
            comps_data.append(item)
    out["comps"] = comps_data

# Inspect robot skeletal mesh sockets
robot_sk = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
if robot_sk:
    # check bone names
    skel = robot_sk.get_editor_property("skeleton")
    out["robot_skeleton"] = str(skel)

# Check sword mesh bounds and sockets
sword = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
if sword:
    b = sword.get_bounds()
    out["sword_bounds"] = {
        "origin": [b.origin.x, b.origin.y, b.origin.z],
        "box_extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z],
        "sphere_radius": b.sphere_radius
    }
    sockets = []
    try:
        for s in sword.get_editor_property("sockets"):
            sockets.append({
                "name": str(s.get_editor_property("socket_name")),
                "loc": str(s.get_editor_property("relative_location")),
                "rot": str(s.get_editor_property("relative_rotation")),
            })
    except Exception as e:
        sockets = str(e)
    out["sword_sockets"] = sockets

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_scout_hierarchy.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_SCOUT_HIERARCHY_SUCCESS")
