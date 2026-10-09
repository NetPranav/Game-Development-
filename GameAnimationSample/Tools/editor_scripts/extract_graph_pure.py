with open('Saved/RRTools/ABP_GenericRetarget.t3d', 'r') as f:
    lines = f.readlines()

in_func = False
func_lines = []
for line in lines:
    if 'Name="UpdateRetargetProfile"' in line:
        in_func = True
    elif in_func and line.startswith('   End Object'):
        func_lines.append(line)
        break
    if in_func:
        func_lines.append(line)

with open('Saved/RRTools/update_retarget_profile_graph.txt', 'w') as f:
    f.writelines(func_lines)

print('EXTRACTED', len(func_lines))
