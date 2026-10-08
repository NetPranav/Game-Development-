import sys
with open(sys.argv[1], 'r') as f:
    text = f.read()

text = text.replace(
    "\t\t\t\tFGameplayCueParameters CueParams;\n\t\t\t\tTargetASC->ExecuteGameplayCue(Tags.GameplayCue_Combat_Hit_Parried, CueParams);",
    "\t\t\t\tFGameplayEventData DefPayload;\n\t\t\t\tDefPayload.EventTag = Tags.GameplayEvent_Combat_Blocked;\n\t\t\t\tTargetASC->HandleGameplayEvent(Tags.GameplayEvent_Combat_Blocked, &DefPayload);\n\t\t\t\tFGameplayCueParameters CueParams;\n\t\t\t\tTargetASC->ExecuteGameplayCue(Tags.GameplayCue_Combat_Hit_Parried, CueParams);"
)

with open(sys.argv[1], 'w') as f:
    f.write(text)
