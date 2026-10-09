import unreal, json

out = {}
ts = unreal.load_asset("/Game/AAA/DataAssets/Items/DA_TestSword")
if ts:
    vis = ts.get_editor_property("visuals")
    t = vis.get_editor_property("hand_offset")
    out["DA_TestSword"] = {
        "socket": str(vis.get_editor_property("attachment_socket")),
        "holster_socket": str(vis.get_editor_property("holster_socket")),
        "loc": [t.translation.x, t.translation.y, t.translation.z],
        "rot": [t.rotation.rotator().pitch, t.rotation.rotator().yaw, t.rotation.rotator().roll],
        "mesh": str(vis.get_editor_property("static_mesh")),
    }

with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_test_sword.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_TEST_SWORD_DONE")
