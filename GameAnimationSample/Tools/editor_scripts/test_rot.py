import unreal
r1 = unreal.Rotator(10, 20, 30)
r2 = unreal.Rotator(pitch=10, yaw=20, roll=30)
print(f"TEST_ROT r1: pitch={r1.pitch}, yaw={r1.yaw}, roll={r1.roll}")
print(f"TEST_ROT r2: pitch={r2.pitch}, yaw={r2.yaw}, roll={r2.roll}")
