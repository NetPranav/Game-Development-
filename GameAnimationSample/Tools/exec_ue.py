import urllib.request
import json
import sys

def execute_unreal_python(code_or_file):
    if code_or_file.endswith(".py"):
        with open(code_or_file, "r", encoding="utf-8") as f:
            cmd = f.read()
    else:
        cmd = code_or_file
        
    payload = {
        "objectPath": "/Script/PythonScriptPlugin.Default__PythonScriptLibrary",
        "functionName": "ExecutePythonCommand",
        "parameters": {
            "PythonCommand": cmd
        }
    }
    
    req = urllib.request.Request(
        "http://127.0.0.1:30010/remote/object/call",
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="PUT"
    )
    
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            body = r.read().decode("utf-8")
            print("HTTP Status:", r.status)
            print("Response:", body)
            return True
    except urllib.error.HTTPError as e:
        print("HTTP Error:", e.code, e.reason)
        print("Body:", e.read().decode("utf-8"))
        return False
    except Exception as e:
        print("Error executing:", e)
        return False

if __name__ == "__main__":
    if len(sys.argv) > 1:
        execute_unreal_python(sys.argv[1])
    else:
        execute_unreal_python('import unreal; unreal.log("HELLO FROM EXEC_UE!")')
