import urllib.request
import json

def main():
    payload = {
        'objectPath': '/Script/Engine.Default__GameplayStatics',
        'functionName': 'GetAllActorsOfClass',
        'parameters': {
            'WorldContextObject': '/Game/RR/Levels/L_RRArena.L_RRArena:PersistentLevel.RRArena_0',
            'ActorClass': '/Script/Engine.StaticMeshActor'
        }
    }
    req = urllib.request.Request('http://127.0.0.1:30010/remote/object/call', data=json.dumps(payload).encode('utf-8'), headers={'Content-Type': 'application/json'}, method='PUT')
    with urllib.request.urlopen(req, timeout=10) as r:
        res = json.loads(r.read().decode('utf-8'))
        actors = res.get('OutActors', [])
        print(f'Total StaticMeshActors: {len(actors)}')
        for a in actors:
            p_act = {'objectPath': a, 'access': 'READ_ACCESS'}
            r_act = urllib.request.Request('http://127.0.0.1:30010/remote/object/property', data=json.dumps(p_act).encode('utf-8'), headers={'Content-Type': 'application/json'}, method='PUT')
            try:
                with urllib.request.urlopen(r_act, timeout=2) as ra:
                    d_act = json.loads(ra.read().decode('utf-8'))
                    rc = d_act.get('RootComponent')
                    if rc:
                        p_rc = {'objectPath': rc, 'access': 'READ_ACCESS'}
                        r_rc = urllib.request.Request('http://127.0.0.1:30010/remote/object/property', data=json.dumps(p_rc).encode('utf-8'), headers={'Content-Type': 'application/json'}, method='PUT')
                        with urllib.request.urlopen(r_rc, timeout=2) as rrc:
                            d_rc = json.loads(rrc.read().decode('utf-8'))
                            mesh = d_rc.get('StaticMesh', 'None')
                            loc = d_rc.get('RelativeLocation', {})
                            scale = d_rc.get('RelativeScale3D', {})
                            name = a.split('.')[-1]
                            mesh_name = mesh.split('/')[-1] if mesh else 'None'
                            x, y, z = loc.get('X', 0), loc.get('Y', 0), loc.get('Z', 0)
                            sx, sy, sz = scale.get('X', 1), scale.get('Y', 1), scale.get('Z', 1)
                            print(f"{name:22} | {mesh_name:30} | Loc: ({x:7.1f}, {y:7.1f}, {z:7.1f}) | Scale: ({sx:5.1f}, {sy:5.1f}, {sz:5.1f})")
            except Exception as e:
                print(f"Error reading {a}: {e}")

if __name__ == '__main__':
    main()
