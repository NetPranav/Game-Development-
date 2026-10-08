import urllib.request
import json
import traceback

def test_connection():
    url = "http://127.0.0.1:30010/remote/info"
    req = urllib.request.Request(url, method='GET')
    
    print(f"Testing connection to Unreal Engine Remote Control API at {url}...")
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            if response.status == 200:
                data = json.loads(response.read().decode('utf-8'))
                print("\n[SUCCESS] CONNECTION SUCCESSFUL!")
                print(f"Engine Version: {data.get('engineVersion', 'Unknown')}")
                print(f"Project Name: {data.get('projectDir', 'Unknown')}")
            else:
                print(f"[ERROR] Connected, but received status code: {response.status}")
    except Exception as e:
        print("\n[FAILED] CONNECTION FAILED!")
        print("Ensure the Unreal Editor is open and 'Remote Control API' plugin is enabled.")
        print(f"Error details: {e}")
        # traceback.print_exc()

if __name__ == "__main__":
    test_connection()
