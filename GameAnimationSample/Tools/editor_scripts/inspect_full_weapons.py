import unreal
import json

out = {}

# 1. Inspect SM_LongSword
ls = unreal.load_asset("/Game/RR/Weapons/Fab/long_sword/StaticMeshes/SM_LongSword")
if ls:
    b = ls.get_bounds()
    out["SM_LongSword"] = {
        "origin": [b.origin.x, b.origin.y, b.origin.z],
        "extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z],
        "sockets": [s.get_name() for s in ls.get_editor_property("sockets")] if hasattr(ls, "sockets") else []
    }
    # Check find_socket
    g = ls.find_socket("Grip")
    out["SM_LongSword"]["has_grip"] = g is not None
    if g:
        loc = g.relative_location
        rot = g.relative_rotation
        out["SM_LongSword"]["grip_loc"] = [loc.x, loc.y, loc.z]
        out["SM_LongSword"]["grip_rot"] = [rot.pitch, rot.yaw, rot.roll]

# 2. Inspect SM_VikingShield
vs = unreal.load_asset("/Game/RR/Weapons/Fab/viking_shield/StaticMeshes/SM_VikingShield")
if vs:
    b = vs.get_bounds()
    out["SM_VikingShield"] = {
        "origin": [b.origin.x, b.origin.y, b.origin.z],
        "extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z],
    }

# 3. Inspect SM_Sword_A
sa = unreal.load_asset("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A")
if sa:
    b = sa.get_bounds()
    out["SM_Sword_A"] = {
        "origin": [b.origin.x, b.origin.y, b.origin.z],
        "extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z],
    }
    g = sa.find_socket("Grip")
    if g:
        out["SM_Sword_A"]["grip_loc"] = [g.relative_location.x, g.relative_location.y, g.relative_location.z]
        out["SM_Sword_A"]["grip_rot"] = [g.relative_rotation.pitch, g.relative_rotation.yaw, g.relative_rotation.roll]

# 4. Inspect SK_Robot_scout_R21
robot = unreal.load_asset("/Game/Robot_scout_R_21/Mesh/SK_Robot_scout_R21")
if robot:
    out["robot_sockets"] = []
    for sname in ["weapon_r", "hand_r", "Grip", "hand_l", "weapon_l"]:
        s = robot.find_socket(sname)
        if s:
            loc = s.relative_location
            rot = s.relative_rotation
            out["robot_sockets"].append({
                "name": sname,
                "bone": str(s.get_editor_property("bone_name")),
                "loc": [loc.x, loc.y, loc.z],
                "rot": [rot.pitch, rot.yaw, rot.roll]
            })

# 5. Inspect DA_PlayerWeapon
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
if da:
    vis = da.get_editor_property("visuals")
    ho = vis.get_editor_property("hand_offset")
    r = ho.rotation.rotator()
    out["DA_PlayerWeapon"] = {
        "socket": str(vis.get_editor_property("attachment_socket")),
        "loc": [ho.translation.x, ho.translation.y, ho.translation.z],
        "rot": [r.pitch, r.yaw, r.roll],
        "scale": [ho.scale3d.x, ho.scale3d.y, ho.scale3d.z]
    }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/inspect_full_weapons.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_FULL_WEAPONS_COMPLETE")
