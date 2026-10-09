import unreal, json

montage = unreal.load_asset("/Game/RR/Player/AM_P_UAL_L1")
print("Montage:", montage)

# Check the underlying sequence in the montage
tracks = montage.get_editor_property("slot_anim_tracks")
seq = None
for t in tracks:
    anim_track = t.get_editor_property("anim_track")
    for seg in anim_track.get_editor_property("anim_segments"):
        anim_ref = seg.get_editor_property("anim_reference")
        print("Segment anim:", anim_ref)
        seq = anim_ref
        break

# In seq, let's check frame at peak attack
out = {}
if seq:
    num_frames = seq.get_editor_property("number_of_sampled_keys")
    out["num_frames"] = num_frames
    out["name"] = seq.get_name()
    
with open(unreal.Paths.project_saved_dir() + "RRTools/inspect_attack_seq.json", "w") as f:
    json.dump(out, f, indent=2)

print("INSPECT_ATTACK_SEQ_DONE")
