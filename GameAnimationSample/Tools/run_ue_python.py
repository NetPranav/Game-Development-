import sys
import time
sys.path.append(r"A:\UNREAL\UE_5.8\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python")
from remote_execution import RemoteExecution

def run_script(script_path_or_cmd, is_file=True):
    exec_mode = "ExecuteFile" if is_file else "ExecuteStatement"
    remote_exec = RemoteExecution()
    remote_exec.start()
    time.sleep(0.5)
    
    nodes = remote_exec.remote_nodes
    if not nodes:
        print("ERROR: No running Unreal Editor nodes found for remote execution!")
        remote_exec.stop()
        return False
        
    node = nodes[0]
    print(f"Connecting to node: {node['node_id']} ({node.get('project_name', 'Unknown')})")
    remote_exec.open_command_connection(node)
    
    if is_file:
        res = remote_exec.run_command(f'exec(open(r"{script_path_or_cmd}").read())')
    else:
        res = remote_exec.run_command(script_path_or_cmd)
        
    print("OUTPUT:")
    for out in res.get("output", []):
        print(f"[{out.get('type')}] {out.get('output')}")
    if not res.get("success", False):
        print("FAILED:", res.get("result"))
    else:
        print("SUCCESS:", res.get("result"))
    remote_exec.stop()
    return res.get("success", False)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        target = sys.argv[1]
        is_f = not target.startswith("-c:")
        cmd = target[3:] if not is_f else target
        run_script(cmd, is_file=is_f)
    else:
        run_script('import unreal; print("HELLO FROM RUN_UE_PYTHON")', is_file=False)
