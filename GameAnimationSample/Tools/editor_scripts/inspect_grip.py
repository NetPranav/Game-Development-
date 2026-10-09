import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
da = unreal.load_asset("/Game/RR/Player/DA_PlayerWeapon")
v = da.get_editor_property("visuals")
for k in ("attachment_socket", "hand_offset", "holster_socket", "static_mesh", "weapon_mesh", "mesh"):
    try: print("GRIP", k, v.get_editor_property(k))
    except Exception as e: pass
for p in ("/Game/Stylized_Dark_Sword/Assets/SwordA/Meshes/SM_Sword_A", "/Game/Stylized_Dark_Sword/Assets/SwordB/Meshes/SM_Sword_B"):
    m = unreal.load_asset(p)
    if m:
        b = m.get_bounds()
        print("GRIP mesh", p, "origin", b.origin, "extent", b.box_extent)
