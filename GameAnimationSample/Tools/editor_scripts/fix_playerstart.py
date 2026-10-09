import unreal, math
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
unreal.EditorLoadingAndSavingUtils.load_map("/Game/RR/Levels/L_RRArena")
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
acts = eas.get_all_level_actors()
ps = [a for a in acts if a.get_class().get_name() == "PlayerStart"][0]
col = [a for a in acts if "Colosseum" in a.get_actor_label()]
p = ps.get_actor_location()
c = col[0].get_actor_location() if col else unreal.Vector(6280, 340, 0)
yaw = math.degrees(math.atan2(c.y - p.y, c.x - p.x))
ps.set_actor_rotation(unreal.Rotator(0.0, 0.0, yaw), False)   # Rotator(roll, pitch, yaw) in Python
print("PS rot now", ps.get_actor_rotation())
unreal.EditorLoadingAndSavingUtils.save_current_level()
print("PS_DONE")
