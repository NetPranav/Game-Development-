import unreal, json, traceback
log = {}
eal = unreal.EditorAssetLibrary
try:
    for k, path in (("A", "/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A"), ("B", "/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B")):
        mesh = unreal.load_asset(path)
        sock = unreal.StaticMeshSocket(outer=mesh)
        sock.set_editor_property("socket_name", "Grip")
        sock.set_editor_property("relative_location", unreal.Vector(0, 0, 135))
        mesh.add_socket(sock)
        eal.save_loaded_asset(mesh)
        log["grip_" + k] = "ok"
    da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
    vis = da.get_editor_property("visuals")
    vis.set_editor_property("hand_offset", unreal.Transform(unreal.Vector(0, 0, 0), unreal.Rotator(90, 0, 0), unreal.Vector(1, 1, 1)))
    da.set_editor_property("visuals", vis)
    eal.save_asset("/Game/RR/Player/DA_PlayerWeapon")
    log["da"] = "ok"
except Exception as e:
    log["ERR"] = str(e)[:200] + traceback.format_exc()[-200:]
open(unreal.Paths.project_saved_dir() + "RRTools/wire_sword2_out.json", "w").write(json.dumps(log))
