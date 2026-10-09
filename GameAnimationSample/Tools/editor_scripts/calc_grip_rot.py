import numpy as np
import math

# We want transform from Sword mesh space to Socket space:
# Vector v_sword in mesh space -> v_socket = R * v_sword
# We want:
# Sword [1, 0, 0] (blade) -> [0, 0, 1] (socket +Z, out of fist)
# Sword [0, 1, 0] (edge)  -> [1, 0, 0] (socket +X, forward towards knuckles)
# Sword [0, 0, 1] (flat)  -> [0, 1, 0] (socket +Y, to the right)

# Matrix R where col 0 is image of [1,0,0], col 1 is image of [0,1,0], col 2 is image of [0,0,1]:
R = np.array([
    [0, 1, 0],
    [0, 0, 1],
    [1, 0, 0]
], dtype=float)

print("Det R:", np.linalg.det(R))

# In Unreal Engine:
# If mesh has socket "Grip", our C++ does:
# FTransform GripTransform(Grip->RelativeRotation, Grip->RelativeLocation, FVector::OneVector);
# FTransform InvGrip = GripTransform.Inverse();
# StaticMeshComponent->SetRelativeLocationAndRotation(InvGrip.GetLocation(), InvGrip.Rotator());
# 
# This means:
# If Grip socket is placed at location L_grip and rotation R_grip on the mesh:
# In the weapon actor, the mesh is transformed by Inv(R_grip, L_grip).
# So a point at L_grip on the mesh ends up at (0,0,0) in weapon actor space.
# And a direction in mesh space is transformed by Inv(R_grip)!
# So if we want:
# v_weapon_actor = R_target * v_mesh
# Then Inv(R_grip) = R_target
# => R_grip = Inv(R_target) = R_target.T !

R_grip = R.T
print("R_grip matrix:")
print(R_grip)

# Let's compute UE Pitch, Yaw, Roll from R_grip:
# In UE FRotator(Pitch, Yaw, Roll):
# UE rotation matrix convention:
# R = R_yaw(Z) * R_pitch(Y) * R_roll(X)
# X_axis = [cos(p)*cos(y), cos(p)*sin(y), sin(p)]
# Y_axis = [-sin(r)*sin(p)*cos(y) - cos(r)*sin(y), -sin(r)*sin(p)*sin(y) + cos(r)*cos(y), sin(r)*cos(p)]
# Z_axis = [cos(r)*sin(p)*cos(y) - sin(r)*sin(y), cos(r)*sin(p)*sin(y) + sin(r)*cos(y), cos(r)*cos(p)]

# Let's test all 24 90-degree rotations to find the exact pitch, yaw, roll that gives R_grip and R_target:
import itertools

results = {}
for p in [-180, -90, 0, 90, 180]:
    for y in [-180, -90, 0, 90, 180]:
        for r in [-180, -90, 0, 90, 180]:
            # UE FRotator to matrix:
            cp, sp = math.cos(math.radians(p)), math.sin(math.radians(p))
            cy, sy = math.cos(math.radians(y)), math.sin(math.radians(y))
            cr, sr = math.cos(math.radians(r)), math.sin(math.radians(r))
            
            # UE Rotator forward, right, up vectors (rows or cols):
            # In UE:
            # X_col = [cp*cy, cp*sy, sp]
            # Y_col = [-sr*sp*cy - cr*sy, -sr*sp*sy + cr*cy, sr*cp]
            # Z_col = [cr*sp*cy - sr*sy, cr*sp*sy + sr*cy, cr*cp]
            # Note: in UE, pitch is rotation around Y (up is positive), roll is rotation around X, yaw is around Z.
            M = np.array([
                [cp*cy, -sr*sp*cy - cr*sy, cr*sp*cy - sr*sy],
                [cp*sy, -sr*sp*sy + cr*cy, cr*sp*sy + sr*cy],
                [sp,     sr*cp,            cr*cp           ]
            ])
            
            diff_grip = np.max(np.abs(M - R_grip))
            if diff_grip < 1e-4:
                print(f"MATCH for R_grip: Pitch={p}, Yaw={y}, Roll={r}")
                
            diff_target = np.max(np.abs(M - R))
            if diff_target < 1e-4:
                print(f"MATCH for R_target: Pitch={p}, Yaw={y}, Roll={r}")
