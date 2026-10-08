# P2 — Combat audit (READ-ONLY, no code changes)
PROMPT_ID = `P2`. Read `Docs/prompts/00_RULES.md` first. **Do not modify any source file in this prompt.** Output only `Docs/reports/P2_combat_audit.md` (≤ 400 lines, tables/bullets, `file:line` references). This report drives the combat implementation prompts, so precision matters more than prose.

## Questions to answer (read the actual code — `Source/AAACombat`, `AAAAbility`, `AAACharacter`, `AAAAI`, `AAAGame`, `AAAInventory`, `AAAItems`, `AAACore`, and `Docs/after/ARCHIVE_Gameplay_Architecture.md` for intent)
1. **Attack pipeline end-to-end**: input (`AAAAPlayerController::OnAttackAction`) → who plays the montage → `AnimNotifyState_WeaponTrace` → `AAAAWeaponActor::PerformTrace` → damage GameplayEffect → `UAAADamageExecutionCalc` → attribute change → death. Give a numbered call chain with file:line. Mark every place that is **stubbed, unfinished, or untested**.
2. **Gaps vs a good melee game.** For each item say: exists / partial / missing, and where: light combo chain with input buffer; heavy attack; block; parry; dodge i-frames; stamina cost+regen; poise/stagger; hit reactions (direction aware); lock-on targeting; hit-stop; camera shake; hit VFX/SFX hooks (GameplayCues); damage numbers; weapon draw/sheath; knockback; friendly-fire/team filtering (`UAAATeamComponent`).
3. **Attributes**: list every attribute in `UAAAAttributeSet` (name, replicated?, default, clamps). Is there Poise/Stamina/Armor/AttackPower? How are they initialised (GE? defaults?) and by whom?
4. **Data assets**: full field list of `UAAAWeaponSettings`, `UAAAItemWeaponSettings`, `UAAACharacterSettings`. Which are read by code vs dead fields? Where would combo montage arrays belong?
5. **Gameplay tags**: dump all native tags in `AAAGameplayTags` relevant to combat.
6. **Movement of pawns**: how does the player pawn move (Mover modes, input producer)? How would an **AI-controlled** `AAAAATestEnemy` move — does `Content/Blueprints` / the sample's NPC setup (see `Content/Levels/NPCLevel.umap`, `Locomotor` plugin usage, any `BP_*NPC*`/StateTree assets found by file name) already provide an AI locomotion path we can reuse? Give a recommendation with evidence; list candidate asset paths (filenames only, you cannot open .uasset).
7. **Montage/animation contract**: what montage sections, slots (e.g. `UpperBody`, `DefaultSlot`) or anim notifies does the code expect? List notifies in `Source/AAAAnimation`. What does the Animation Blueprint side need (cannot inspect .uasset; list what the C++ expects).
8. **Replication & prediction**: which combat actions are server RPC / predicted via GAS? Known desync risks.
9. **Compile-health**: any obvious compile/logic bugs, dead code, includes that would break a clean build, `UPROPERTY` misuse. Do not fix; list with file:line and severity.
10. **Recommended extension points**: exact functions/classes where each missing feature should be implemented with the smallest change, and any refactor that is unavoidable.

End with a section `## Proposed implementation order` (≤ 12 steps, each ≤ 2 lines).
