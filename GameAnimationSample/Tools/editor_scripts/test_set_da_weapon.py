import unreal, json

da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis = da.get_editor_property("visuals")

# Let's check constructor of Rotator
r = unreal.Rotator(pitch=-3.176, yaw=-180.0, roll=0.0)
print(f"r: pitch={r.pitch}, yaw={r.yaw}, roll={r.roll}")

loc = unreal.Vector(-10.764, 5.811, -0.728)
t = unreal.Transform(loc, r, unreal.Vector(1, 1, 1))

print(f"t loc: {t.translation}")
print(f"t rot: {t.rotation.rotator()}")

vis.set_editor_property("attachment_socket", "hand_r")
vis.set_editor_property("hand_offset", t)
da.set_editor_property("visuals", vis)

unreal.EditorAssetLibrary.save_loaded_asset(da)
print("SAVED DA_PlayerWeapon!")

# Re-read and verify
da2 = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
vis2 = da2.get_editor_property("visuals")
ho2 = vis2.get_editor_property("hand_offset")
r2 = ho2.rotation.rotator()
print(f"Verified loc: {ho2.translation}")
print(f"Verified rot: pitch={r2.pitch}, yaw={r2.yaw}, roll={r2.roll}")

out = {
    "attachment_socket": str(vis2.get_editor_property("attachment_socket")),
    "loc": [ho2.translation.x, ho2.translation.y, ho2.translation.z],
    "rot": [r2.pitch, r2.yaw, r2.roll]
}

with open(unreal.Paths.project_saved_dir() + "RRTools/verify_da_weapon.json", "w") as f:
    json.dump(out, f, indent=2)

print("VERIFY_DA_WEAPON_DONE")
