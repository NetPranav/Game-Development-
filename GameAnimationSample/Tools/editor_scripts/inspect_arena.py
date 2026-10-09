import unreal
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
acts = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
ps = [a for a in acts if a.get_class().get_name() == "PlayerStart"]
p0 = ps[0].get_actor_location() if ps else unreal.Vector()
print("ARENA playerstart", [str(a.get_actor_location()) for a in ps], "rot", ps[0].get_actor_rotation() if ps else None)
rows = []
for a in acts:
    o, e = a.get_actor_bounds(False)
    d = (a.get_actor_location() - p0).length()
    cn = a.get_class().get_name()
    label = a.get_actor_label()
    rows.append((d, cn, label, o, e))
rows.sort(key=lambda r: r[0])
for d, cn, label, o, e in rows[:60]:
    print("ARENA %6.0f %-28s %-40s c=(%.0f,%.0f,%.0f) ext=(%.0f,%.0f,%.0f)" % (d, cn[:28], label[:40], o.x, o.y, o.z, e.x, e.y, e.z))
print("ARENA total", len(acts))
