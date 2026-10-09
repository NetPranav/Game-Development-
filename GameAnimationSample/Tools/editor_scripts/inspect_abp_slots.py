import unreal, json

abp = unreal.load_asset("/Game/Blueprints/SandboxCharacter_Mover_ABP")
out = {}

# Check properties and skeleton
skel = abp.get_editor_property("target_skeleton")
out["skeleton"] = str(skel.get_path_name()) if skel else None

# Check all montage slot names on this skeleton
slots = []
try:
    for sg in skel.get_editor_property("slot_groups"):
        gname = str(sg.get_editor_property("group_name"))
        for sname in sg.get_editor_property("slot_names"):
            slots.append(f"{gname}.{sname}")
except Exception as e:
    slots.append(f"ERR: {e}")
out["skeleton_slots"] = slots

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/abp_slots.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ABP_SLOTS_DONE")
