import urllib.request
import json
import sys

BASE_URL = "http://127.0.0.1:30010"

def remote_call(endpoint, payload=None, method='PUT'):
    """Send a request to the Unreal Remote Control API."""
    url = f"{BASE_URL}{endpoint}"
    data = json.dumps(payload).encode('utf-8') if payload else None
    req = urllib.request.Request(url, data=data, method=method)
    req.add_header('Content-Type', 'application/json')
    try:
        with urllib.request.urlopen(req, timeout=10) as response:
            return json.loads(response.read().decode('utf-8'))
    except Exception as e:
        return {"error": str(e)}

def get_info():
    """Get server info."""
    url = f"{BASE_URL}/remote/info"
    req = urllib.request.Request(url, method='GET')
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            return json.loads(response.read().decode('utf-8'))
    except Exception as e:
        return {"error": str(e)}

def search_assets(class_name, search_path="/Game/"):
    """Search for assets of a given class."""
    payload = {
        "objectPath": "/Script/AssetRegistry.Default__AssetRegistryHelpers",
        "functionName": "GetAssets",
        "parameters": {
            "AssetRegistryFilter": {
                "ClassPaths": [class_name],
                "PackagePaths": [search_path],
                "bRecursivePaths": True
            }
        }
    }
    return remote_call("/remote/object/call", payload)

def list_assets_in_path(path="/Game/AAA/"):
    """List all assets under a given path using the search endpoint."""
    payload = {
        "Query": "",
        "Filter": {
            "PackagePaths": [path],
            "RecursivePaths": True
        },
        "Limit": 200
    }
    return remote_call("/remote/search/assets", payload)

def get_all_level_actors():
    """Get all actors in the current level."""
    payload = {
        "objectPath": "/Script/EditorScriptingUtilities.Default__EditorLevelLibrary",
        "functionName": "GetAllLevelActors",
        "parameters": {}
    }
    return remote_call("/remote/object/call", payload)

def get_current_level():
    """Get the current level name."""
    payload = {
        "objectPath": "/Script/UnrealEd.Default__EditorLevelLibrary",
        "functionName": "GetEditorWorld",
        "parameters": {}
    }
    return remote_call("/remote/object/call", payload)

def run_python_in_editor(python_code):
    """Execute Python code directly inside the Unreal Editor."""
    payload = {
        "objectPath": "/Script/PythonScriptPlugin.Default__PythonScriptLibrary",
        "functionName": "ExecutePythonCommand",
        "parameters": {
            "PythonCommand": python_code
        }
    }
    return remote_call("/remote/object/call", payload)

def main():
    print("=" * 70)
    print("UNREAL ENGINE EDITOR AUDIT")
    print("=" * 70)

    # 1. Server info
    print("\n--- SERVER INFO ---")
    info = get_info()
    print(json.dumps(info, indent=2, default=str)[:2000])

    # 2. Search for assets under /Game/AAA/
    print("\n--- ASSETS UNDER /Game/AAA/ ---")
    aaa_assets = list_assets_in_path("/Game/AAA/")
    if "error" not in aaa_assets:
        if isinstance(aaa_assets, dict) and "Assets" in aaa_assets:
            for asset in aaa_assets["Assets"]:
                name = asset.get("Name", asset.get("AssetName", "?"))
                path = asset.get("Path", asset.get("ObjectPath", "?"))
                cls = asset.get("Class", asset.get("AssetClass", "?"))
                print(f"  [{cls}] {name} -> {path}")
            print(f"  Total: {len(aaa_assets['Assets'])} assets")
        else:
            print(json.dumps(aaa_assets, indent=2, default=str)[:3000])
    else:
        print(f"  Error: {aaa_assets['error']}")

    # 3. Search for ALL assets under /Game/
    print("\n--- ALL PROJECT ASSETS ---")
    all_assets = list_assets_in_path("/Game/")
    if "error" not in all_assets:
        if isinstance(all_assets, dict) and "Assets" in all_assets:
            print(f"  Total assets in project: {len(all_assets['Assets'])}")
            # Group by class
            class_counts = {}
            for asset in all_assets["Assets"]:
                cls = asset.get("Class", asset.get("AssetClass", "Unknown"))
                class_counts[cls] = class_counts.get(cls, 0) + 1
            print("  Asset types:")
            for cls, count in sorted(class_counts.items(), key=lambda x: -x[1]):
                print(f"    {cls}: {count}")
        else:
            print(json.dumps(all_assets, indent=2, default=str)[:3000])
    else:
        print(f"  Error: {all_assets['error']}")

    # 4. Try to list level actors
    print("\n--- LEVEL ACTORS (current map) ---")
    actors = get_all_level_actors()
    if "error" not in actors:
        if isinstance(actors, dict) and "ReturnValue" in actors:
            actor_list = actors["ReturnValue"]
            print(f"  Total actors in level: {len(actor_list)}")
            # Show first 20
            for a in actor_list[:20]:
                print(f"    {a}")
        else:
            print(json.dumps(actors, indent=2, default=str)[:3000])
    else:
        print(f"  Error: {actors['error']}")

    # 5. Try running Python inside the editor to get more info
    print("\n--- EDITOR PYTHON QUERY ---")
    python_code = """
import unreal
# Get the current world
world = unreal.EditorLevelLibrary.get_editor_world()
level_name = world.get_path_name() if world else 'No world loaded'
unreal.log(f'AUDIT_LEVEL: {level_name}')

# Get project settings
settings = unreal.get_default_object(unreal.GameMapsSettings)
gm = settings.get_editor_property('global_default_game_mode')
unreal.log(f'AUDIT_GAMEMODE: {gm}')

# Count all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()
unreal.log(f'AUDIT_ACTOR_COUNT: {len(actors)}')

# Categorize actors
actor_classes = {}
for a in actors:
    cls_name = a.get_class().get_name()
    actor_classes[cls_name] = actor_classes.get(cls_name, 0) + 1

for cls_name, count in sorted(actor_classes.items(), key=lambda x: -x[1])[:30]:
    unreal.log(f'AUDIT_ACTOR_TYPE: {cls_name} = {count}')
"""
    result = run_python_in_editor(python_code)
    if "error" not in result:
        print(json.dumps(result, indent=2, default=str)[:3000])
    else:
        print(f"  Could not run Python in editor: {result['error']}")
        print("  (This is OK - we'll get data through other endpoints)")

    # 6. Check for specific blueprint classes
    print("\n--- CHECKING KEY BLUEPRINTS ---")
    key_paths = [
        "/Game/AAA/Blueprints/GameFramework/BP_AAAGameMode",
        "/Game/AAA/Blueprints/GameFramework/BP_AAAGameState",
        "/Game/AAA/Blueprints/GameFramework/BP_AAAPlayerController",
        "/Game/AAA/Blueprints/GameFramework/BP_AAAPlayerState",
        "/Game/AAA/Blueprints/GameFramework/BP_AAAHUD",
        "/Game/AAA/Characters/BP_PlayerCharacter",
        "/Game/AAA/Gameplay/BP_AAAGameMode",
        "/Game/AAA/DataAssets/Asset/Sword/DA_TestSword",
        "/Game/AAA/DataAssets/Characters/DA_PlayerCharacterSettings",
    ]
    for p in key_paths:
        payload = {
            "objectPath": p,
            "access": "READ_ACCESS"
        }
        result = remote_call("/remote/object/property", payload)
        if "error" not in result:
            print(f"  [EXISTS] {p}")
        else:
            err_str = str(result.get("error", ""))
            if "404" in err_str or "not found" in err_str.lower():
                print(f"  [MISSING] {p}")
            else:
                print(f"  [UNKNOWN] {p} -> {err_str[:100]}")

    print("\n" + "=" * 70)
    print("AUDIT COMPLETE")
    print("=" * 70)

if __name__ == "__main__":
    main()
