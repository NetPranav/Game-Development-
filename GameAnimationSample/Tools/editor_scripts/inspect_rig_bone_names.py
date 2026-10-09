import unreal, json

src_rig = unreal.load_asset("/Game/RR/ExternalAnims/ProSwordShield/IK_ProSwordShield")
tgt_rig = unreal.load_asset("/Game/Characters/UEFN_Mannequin/Rigs/IK_UEFN_Mannequin")

def bone_name(ref):
    return str(ref.get_editor_property("bone_name"))

out = {"src": {}, "tgt": {}}

c = unreal.IKRigController.get_controller(src_rig)
for ch in c.get_retarget_chains():
    name = str(ch.get_editor_property("chain_name"))
    out["src"][name] = {
        "start": bone_name(ch.get_editor_property("start_bone")),
        "end": bone_name(ch.get_editor_property("end_bone")),
        "ik_goal": str(ch.get_editor_property("ik_goal_name"))
    }

c = unreal.IKRigController.get_controller(tgt_rig)
for ch in c.get_retarget_chains():
    name = str(ch.get_editor_property("chain_name"))
    out["tgt"][name] = {
        "start": bone_name(ch.get_editor_property("start_bone")),
        "end": bone_name(ch.get_editor_property("end_bone")),
        "ik_goal": str(ch.get_editor_property("ik_goal_name"))
    }

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/rig_bone_names.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_BONE_NAMES_DONE")
