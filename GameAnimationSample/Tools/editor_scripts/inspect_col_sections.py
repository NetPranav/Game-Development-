import unreal, json

col = unreal.load_asset("/Game/RR/Environment/Colosseum/SM_Colosseum")
out = {}
if col:
    b = col.get_bounds()
    out["bounds_origin"] = [b.origin.x, b.origin.y, b.origin.z]
    out["bounds_extent"] = [b.box_extent.x, b.box_extent.y, b.box_extent.z]
    
    # Check materials
    mats = []
    for i in range(col.get_num_sections(0)):
        m = col.get_material(i)
        mats.append(m.get_name() if m else "None")
    out["materials"] = mats

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/col_sections.json", "w") as f:
    json.dump(out, f, indent=2)

print("COL_SECTIONS_DONE")
