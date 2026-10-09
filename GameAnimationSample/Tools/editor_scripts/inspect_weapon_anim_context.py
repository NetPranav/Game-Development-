import unreal, json

out = {}

# Check AM_P_UAL_L1
m1 = unreal.load_asset("/Game/RR/Player/AM_P_UAL_L1")
if m1:
    skel = m1.get_editor_property("skeleton")
    out["m1_skeleton"] = skel.get_path_name()
    
# Let's check the base animation sequence in AM_P_UAL_L1
for track in m1.get_editor_property("slot_anim_tracks"):
    for anim in track.anim_track.anim_segments:
        seq = anim.anim_reference
        if seq:
            out["seq_name"] = seq.get_name()
            out["seq_path"] = seq.get_path_name()

# Let's check Greystone's weapon attachment
gs_da = unreal.load_asset("/Game/RR/Greystone/DA_GreystoneWeapon")
if gs_da:
    vis = gs_da.get_editor_property("visuals")
    out["gs_socket"] = str(vis.get_editor_property("attachment_socket"))
    t = vis.get_editor_property("hand_offset")
    out["gs_hand_offset"] = {
        "loc": [t.translation.x, t.translation.y, t.translation.z],
        "rot": [t.rotation.rotator().pitch, t.rotation.rotator().yaw, t.rotation.rotator().roll],
    }

# Also let's inspect Greystone character mesh to see where Greystone holds his sword!
gs_mesh = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Meshes/Greystone")
if gs_mesh:
    gs_socks = []
    # Greystone has sockets!
    for i in range(gs_mesh.num_sockets()):
        s = gs_mesh.get_socket_by_index(i)
        gs_socks.append({
            "name": str(s.get_editor_property("socket_name")),
            "bone": str(s.get_editor_property("bone_name")),
            "loc": [s.relative_location.x, s.relative_location.y, s.relative_location.z],
            "rot": [s.relative_rotation.pitch, s.relative_rotation.yaw, s.relative_rotation.roll]
        })
    out["greystone_all_sockets"] = gs_socks

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_weapon_anim_context.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_WEAPON_ANIM_CONTEXT_DONE")
