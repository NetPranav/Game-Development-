import unreal, json

eal = unreal.EditorAssetLibrary
assets = [str(x) for x in eal.list_assets("/Game/RR/ExternalAnims/ProSwordShield", recursive=False)]
out = {"assets": assets}

with open("A:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/pro_assets.json", "w") as f:
    json.dump(out, f, indent=2)

print(f"Total pro assets: {len(assets)}")
