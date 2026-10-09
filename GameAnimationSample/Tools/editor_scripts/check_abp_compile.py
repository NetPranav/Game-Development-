import unreal, json

abp = unreal.load_asset("/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint")
out = {}

if abp:
    # compile the anim blueprint
    unreal.KismetEditorUtilities.compile_blueprint(abp)
    out["status"] = str(abp.get_editor_property("status"))
    print("ABP_STATUS:", out["status"])

with open(unreal.Paths.project_saved_dir() + "RRTools/abp_compile_status.json", "w") as f:
    json.dump(out, f, indent=2)

print("ABP_COMPILE_CHECK_DONE")
