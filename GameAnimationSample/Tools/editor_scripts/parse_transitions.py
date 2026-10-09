import re, json

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/Greystone_AnimBlueprint.t3d", "r") as f:
    text = f.read()

# Let's find Ground Locomotion transitions
# Look for AnimStateTransitionNode blocks
transitions = re.findall(r'Begin Object Class=/Script/AnimGraph\.AnimStateTransitionNode Name="([^"]+)".*?End Object', text, re.DOTALL)
print(f"Found {len(transitions)} transitions")

# Search for PrevState, NextState, and the logic inside
trans_details = []
matches = list(re.finditer(r'Begin Object Class=/Script/AnimGraph\.AnimStateTransitionNode Name="([^"]+)"(.*?)End Object\s+Begin Object', text, re.DOTALL))
for m in matches:
    name = m.group(1)
    body = m.group(2)
    # find PrevStateNode, NextStateNode
    prev = re.search(r'PrevStateNode=[^\'"]*[\'"]([^\'"]+)[\'"]', body)
    next_n = re.search(r'NextStateNode=[^\'"]*[\'"]([^\'"]+)[\'"]', body)
    # find variables used in this transition
    vars_used = re.findall(r'MemberName="([^"]+)"', body)
    # find any constants/comparison operators
    comparisons = re.findall(r'MemberName="(Greater|Less|Equal|NotEqual|Boolean)[^"]*"', body)
    # find default float values
    floats = re.findall(r'DefaultValue="([0-9\.]+)"', body)
    
    trans_details.append({
        "transition": name,
        "prev": prev.group(1) if prev else "None",
        "next": next_n.group(1) if next_n else "None",
        "vars": vars_used,
        "comparisons": comparisons,
        "floats": floats
    })

with open("a:/UNREAL PROJECT/Game/GameAnimationSample/Saved/RRTools/locomotion_transitions.json", "w") as out_f:
    json.dump(trans_details, out_f, indent=2)

print("PARSED_TRANSITIONS_DONE")
