import unreal, json

m = unreal.load_asset("/Game/RR/Player/AM_P_SwordBlock")
methods = [x for x in dir(m) if "section" in x.lower() or "loop" in x.lower()]

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/montage_methods.json", "w") as f:
    json.dump(methods, f, indent=2)

print("SAVED_METHODS")
