import re, json

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/Greystone_AnimBlueprint.t3d", "r") as f:
    text = f.read()

# Let's find the Ground Locomotion graph block
sm_match = re.search(r'Begin Object Class=/Script/AnimGraph\.AnimationStateMachineGraph Name="Ground Locomotion"(.*?)End Object\s+Begin Object Class=/Script/AnimGraph\.AnimGraphNode_StateMachine Name="AnimGraphNode_StateMachine_0"', text, re.DOTALL)
if not sm_match:
    sm_match = re.search(r'Begin Object Class=/Script/AnimGraph\.AnimationStateMachineGraph Name="Ground Locomotion"(.*?)Begin Object Class=/Script/AnimGraph\.AnimGraphNode_StateMachine', text, re.DOTALL)

ground_text = sm_match.group(1) if sm_match else text

# Let's find all transitions:
trans_blocks = re.findall(r'Begin Object Class=/Script/AnimGraph\.AnimStateTransitionNode Name="([^"]+)"(.*?)End Object\s+Begin Object Class=/Script/AnimGraph\.AnimState', ground_text, re.DOTALL)
print(f"Found {len(trans_blocks)} transitions in Ground Locomotion")

# Also find all state definitions and their properties later in the file:
# In Unreal t3d, the outer definitions are at the top, then property assignments are lower down.
# Let's search for "Begin Object Name=\"AnimStateTransitionNode_"
lower_trans = re.findall(r'Begin Object Name="(AnimStateTransitionNode_\d+)"(.*?)End Object', text, re.DOTALL)
print(f"Found {len(lower_trans)} lower transition definitions")

trans_info = {}
for name, block in lower_trans:
    prev_m = re.search(r'PrevStateNode=[^\'"]*[\'"]([^\'"]+)[\'"]', block)
    next_m = re.search(r'NextStateNode=[^\'"]*[\'"]([^\'"]+)[\'"]', block)
    if prev_m or next_m:
        trans_info[name] = {
            "prev": prev_m.group(1) if prev_m else "None",
            "next": next_m.group(1) if next_m else "None",
            "logic": []
        }

# Also find state names (e.g. AnimStateNode_0 -> JogStart, AnimStateNode_1 -> Idle, AnimStateNode_2 -> Run, AnimStateNode_5 -> JogStop)
states_map = {
    "AnimStateNode_0": "JogStart",
    "AnimStateNode_1": "Idle",
    "AnimStateNode_2": "Run",
    "AnimStateNode_5": "JogStop",
}

# Now for each transition, let's find its Transition graph nodes in the text
for trans_name, info in trans_info.items():
    # search for Transition graph inside this node
    t_graph_match = re.search(rf'AnimStateTransitionNode Name="{trans_name}".*?AnimationTransitionGraph Name="([^"]+)".*?End Object\s+End Object', text, re.DOTALL)
    if t_graph_match:
        t_body = t_graph_match.group(0)
        # extract nodes in t_body
        var_gets = re.findall(r'VariableReference=\(MemberName="([^"]+)"', t_body)
        funcs = re.findall(r'FunctionReference=\([^\)]*MemberName="([^"]+)"', t_body)
        consts = re.findall(r'DefaultValue="([^"]+)"', t_body)
        info["vars"] = var_gets
        info["funcs"] = funcs
        info["consts"] = consts
        # Map state names
        p = info["prev"].split(".")[-1]
        n = info["next"].split(".")[-1]
        info["from_state"] = states_map.get(p, p)
        info["to_state"] = states_map.get(n, n)

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/ground_locomotion_details.json", "w") as out_f:
    json.dump(trans_info, out_f, indent=2)

print("PARSED_GROUND_LOCOMOTION_DONE")
