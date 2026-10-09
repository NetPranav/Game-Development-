import json, os

t3d_path = "A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/Sandbox_ABP.t3d"
found = []
if os.path.exists(t3d_path):
    with open(t3d_path, "r", encoding="utf-16", errors="ignore") as f:
        for i, line in enumerate(f):
            if any(k in line for k in ["Slot", "slot", "UpperBody", "Blend", "Layer"]):
                found.append(f"L{i}: {line.strip()}")

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/abp_slots_utf16.json", "w") as f:
    json.dump(found[:100], f, indent=2)

print(f"Total matching lines in Sandbox_ABP.t3d: {len(found)}")
