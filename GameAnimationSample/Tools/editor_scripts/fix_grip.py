import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
# SM_Sword_A/B pivot = end of the pommel, blade along +Z; HandOffset rotates mesh +Z onto hand_r +Y.
# Shift 9 cm back along the blade so the fist closes around the middle of the grip instead of the pommel.
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
v = da.get_editor_property("visuals")
t = v.get_editor_property("hand_offset")
t = unreal.Transform(unreal.Vector(0.0, -9.0, 0.0), t.rotation.rotator(), t.scale3d)
v.set_editor_property("hand_offset", t)
da.set_editor_property("visuals", v)
unreal.EditorAssetLibrary.save_loaded_asset(da)
print("GRIPFIX", da.get_editor_property("visuals").get_editor_property("hand_offset"))
