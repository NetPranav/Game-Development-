import unreal
# M_RR_Glow: unlit, additive, two-sided. Params: Color (vector), Intensity (scalar), Ring (0 = flat glow, 1 = disc with a
# bright rim + soft fill, drawn on the /Engine/BasicShapes/Plane UVs). Used by ARRArena's entry circle.
print = lambda *a: unreal.log_warning(" ".join(str(x) for x in a))
MEL = unreal.MaterialEditingLibrary
path = "/Game/RR/Materials/M_RR_Glow"
if unreal.EditorAssetLibrary.does_asset_exist(path):
    unreal.EditorAssetLibrary.delete_asset(path)
m = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_RR_Glow", "/Game/RR/Materials", unreal.Material, unreal.MaterialFactoryNew())
m.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
m.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
m.set_editor_property("two_sided", True)

def E(cls, x, y):
    return MEL.create_material_expression(m, cls, x, y)

color = E(unreal.MaterialExpressionVectorParameter, -900, -200); color.set_editor_property("parameter_name", "Color"); color.set_editor_property("default_value", unreal.LinearColor(0.25, 0.75, 1.0, 1.0))
inten = E(unreal.MaterialExpressionScalarParameter, -900, 0); inten.set_editor_property("parameter_name", "Intensity"); inten.set_editor_property("default_value", 6.0)
ring = E(unreal.MaterialExpressionScalarParameter, -900, 200); ring.set_editor_property("parameter_name", "Ring"); ring.set_editor_property("default_value", 1.0)

uv = E(unreal.MaterialExpressionTextureCoordinate, -1500, 400)
c05 = E(unreal.MaterialExpressionConstant2Vector, -1500, 520); c05.set_editor_property("r", 0.5); c05.set_editor_property("g", 0.5)
dist = E(unreal.MaterialExpressionDistance, -1300, 450)
MEL.connect_material_expressions(uv, "", dist, "A"); MEL.connect_material_expressions(c05, "", dist, "B")
two = E(unreal.MaterialExpressionMultiply, -1150, 450); two.set_editor_property("const_b", 2.0)
MEL.connect_material_expressions(dist, "", two, "A")                     # D: 0 centre .. 1 edge
# rim = saturate(1 - abs(D - 0.86) * 9)
sub = E(unreal.MaterialExpressionSubtract, -1000, 400); sub.set_editor_property("const_b", 0.86)
MEL.connect_material_expressions(two, "", sub, "A")
ab = E(unreal.MaterialExpressionAbs, -880, 400); MEL.connect_material_expressions(sub, "", ab, "Input")
m9 = E(unreal.MaterialExpressionMultiply, -760, 400); m9.set_editor_property("const_b", 9.0); MEL.connect_material_expressions(ab, "", m9, "A")
om = E(unreal.MaterialExpressionOneMinus, -640, 400); MEL.connect_material_expressions(m9, "", om, "Input")
rim = E(unreal.MaterialExpressionSaturate, -520, 400); MEL.connect_material_expressions(om, "", rim, "Input")
# fill = saturate(1 - D) * 0.22 ; inside mask = saturate((1 - D) * 40)
om2 = E(unreal.MaterialExpressionOneMinus, -1000, 600); MEL.connect_material_expressions(two, "", om2, "Input")
sat2 = E(unreal.MaterialExpressionSaturate, -880, 600); MEL.connect_material_expressions(om2, "", sat2, "Input")
fill = E(unreal.MaterialExpressionMultiply, -760, 600); fill.set_editor_property("const_b", 0.22); MEL.connect_material_expressions(sat2, "", fill, "A")
m40 = E(unreal.MaterialExpressionMultiply, -760, 720); m40.set_editor_property("const_b", 40.0); MEL.connect_material_expressions(om2, "", m40, "A")
mask = E(unreal.MaterialExpressionSaturate, -640, 720); MEL.connect_material_expressions(m40, "", mask, "Input")
add = E(unreal.MaterialExpressionAdd, -400, 500); MEL.connect_material_expressions(rim, "", add, "A"); MEL.connect_material_expressions(fill, "", add, "B")
shape = E(unreal.MaterialExpressionMultiply, -280, 550); MEL.connect_material_expressions(add, "", shape, "A"); MEL.connect_material_expressions(mask, "", shape, "B")
# Ring=0 -> flat 0.35 (light column), Ring=1 -> disc shape
flat = E(unreal.MaterialExpressionConstant, -280, 700); flat.set_editor_property("r", 0.35)
lerp = E(unreal.MaterialExpressionLinearInterpolate, -150, 600)
MEL.connect_material_expressions(flat, "", lerp, "A"); MEL.connect_material_expressions(shape, "", lerp, "B"); MEL.connect_material_expressions(ring, "", lerp, "Alpha")
ci = E(unreal.MaterialExpressionMultiply, -500, -100); MEL.connect_material_expressions(color, "", ci, "A"); MEL.connect_material_expressions(inten, "", ci, "B")
out = E(unreal.MaterialExpressionMultiply, 0, 200); MEL.connect_material_expressions(ci, "", out, "A"); MEL.connect_material_expressions(lerp, "", out, "B")
MEL.connect_material_property(out, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
MEL.recompile_material(m)
unreal.EditorAssetLibrary.save_loaded_asset(m)
print("GLOW_DONE", m.get_path_name())
