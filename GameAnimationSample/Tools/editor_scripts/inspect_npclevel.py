import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
unreal.EditorLoadingAndSavingUtils.load_map("/Game/Levels/NPCLevel")
acts = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
from collections import Counter
c = Counter(a.get_class().get_path_name() for a in acts)
for k, v in c.most_common(40): print("NPCL class", v, k)
for a in acts:
    cn = a.get_class().get_name()
    if "Sandbox" in cn or "NPC" in cn or "Character" in cn:
        info = [cn, a.get_name()]
        for p in ("ai_controller_class", "auto_possess_ai"):
            try: info.append("%s=%s" % (p, a.get_editor_property(p)))
            except Exception: pass
        for comp in a.get_components_by_class(unreal.ActorComponent):
            if "Visual" in comp.get_class().get_name() or "Override" in comp.get_class().get_name():
                info.append("comp=" + comp.get_class().get_name())
        print("NPCL actor", " | ".join(str(x) for x in info))
        break
