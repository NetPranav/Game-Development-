import unreal, json, traceback
log = {}
eal = unreal.EditorAssetLibrary
SW = {"A": "/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A", "B": "/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B"}
try:
    for k, path in SW.items():
        mesh = unreal.load_asset(path)
        b = mesh.get_bounds()
        top = b.origin.z + b.box_extent.z
        bottom = b.origin.z - b.box_extent.z
        existing = [str(s.get_editor_property("socket_name")) for s in []]
        for name, z in (("TraceStart", bottom + (top - bottom) * 0.15), ("TraceEnd", top * 0.97)):
            sock = unreal.StaticMeshSocket(outer=mesh)
            sock.set_editor_property("socket_name", name)
            sock.set_editor_property("relative_location", unreal.Vector(0, 0, z))
            mesh.add_socket(sock)
        eal.save_loaded_asset(mesh)
        log["sockets_" + k] = "added, z range %.1f..%.1f" % (bottom, top)
except Exception as e:
    log["sockets_ERR"] = str(e)[:200] + traceback.format_exc()[-200:]
try:
    da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
    vis = da.get_editor_property("visuals")
    vis.set_editor_property("static_mesh", unreal.load_asset(SW["A"]))
    vis.set_editor_property("attachment_socket", "hand_r")
    vis.set_editor_property("hand_offset", unreal.Transform(unreal.Vector(0, 0, 0), unreal.Rotator(0, 90, 0), unreal.Vector(1, 1, 1)))
    vis.set_editor_property("holster_offset", unreal.Transform(unreal.Vector(0, -12, 10), unreal.Rotator(0, 0, 90), unreal.Vector(1, 1, 1)))
    da.set_editor_property("visuals", vis)
    eal.save_asset("/Game/RR/Player/DA_PlayerWeapon")
    log["da_visuals"] = "ok"
except Exception as e:
    log["da_ERR"] = str(e)[:200] + traceback.format_exc()[-200:]
open(unreal.Paths.project_saved_dir() + "RRTools/wire_sword_out.json", "w").write(json.dumps(log, indent=1))
