# P13b - Intro video after the loading screen

Only edit/create the files named here. Report to Docs/reports/P13b_report.md.

Context: `ARRHUD` (Source/RRGame/Public/RRHUD.h + Source/RRGame/Private/RRHUD_*.cpp) is a Canvas-only HUD with a screen
state enum `ERRScreen` (RRHUD.h line ~18). `ARRHUD::FinishLoading()` in Source/RRGame/Private/RRHUD_FrontEnd.cpp (line ~97)
is called when the loading bar completes; it calls GM->BeginGameplay() and `SetScreen(ERRScreen::Playing)`.
Drawing helpers used by the other screens: `Box(x,y,w,h,color)`, `Txt(text, x, y, size, color, align, tracking)`, `U(px)`
(1920x1080 reference scaling), `CanvasW/CanvasH`, `Now` (FPlatformTime seconds), input flags `bConfirm`, `bBack`, `bClickL`
filled by `PollInput()`. Read RRHUD.h and RRHUD_FrontEnd.cpp first to match the style.

## Task
1. Add `IntroVideo` to `ERRScreen` (before `Playing`).
2. New file Source/RRGame/Private/RRHUD_Intro.cpp with:
   - `bool ARRHUD::StartIntroVideo()`: path = `FPaths::ProjectContentDir() / TEXT("Movies/RR_Intro.mp4")`. If the file
     does not exist (`FPaths::FileExists`) return false. Otherwise create (NewObject, owned by this HUD, stored in
     `UPROPERTY(Transient)` members) a `UMediaPlayer` (PlayOnOpen = true, SetLooping(false)), a `UFileMediaSource`
     (`SetFilePath`), a `UMediaTexture` (`SetMediaPlayer`, `UpdateResource()`), and a `UMediaSoundComponent`
     (NewObject<UMediaSoundComponent>(this), `SetMediaPlayer`, `RegisterComponent()`, `Activate()`), then
     `MediaPlayer->OpenSource(Source)`. Record `IntroStart = Now`. Return true.
   - `void ARRHUD::DrawIntroVideo()`: black full-screen box, then draw the media texture letterboxed (keep its aspect,
     `Canvas->DrawTile(MediaTexture, ...)` with full UVs), a small pulsing "ENTER  -  SKIP" text bottom-right.
     End the video (call `EndIntroVideo()`) when: `bConfirm || bBack || bClickL` (only after 0.5 s), or the player
     reports end (`MediaPlayer->IsReady()` was true earlier and now `!MediaPlayer->IsPlaying()` and Now-IntroStart > 1),
     or 120 s safety timeout, or the player failed to open within 5 s.
   - `void ARRHUD::EndIntroVideo()`: Close the player, deactivate/destroy the sound component, null the members,
     then do exactly what FinishLoading did before: GM->BeginGameplay() (if GM) and `SetScreen(ERRScreen::Playing)`
     plus the same fade-in lines FinishLoading uses.
3. In `FinishLoading()`: if `StartIntroVideo()` returns true, `SetScreen(ERRScreen::IntroVideo)` and return; else keep the
   old behaviour. Make sure the game does NOT start (no BeginGameplay) until the video ends.
4. In the HUD's DrawHUD switch (RRHUD.cpp, `switch (Screen)`) add `case ERRScreen::IntroVideo: DrawIntroVideo(); break;`
   and treat IntroVideo like Loading for pausing/input (look at how SetScreen/EnterUIMode handle Loading and copy it).
5. RRHUD.h: declare the 3 functions, `double IntroStart = 0;`, and the members
   `UPROPERTY(Transient) TObjectPtr<class UMediaPlayer> IntroPlayer;` `... UFileMediaSource ... IntroSource;`
   `... UMediaTexture ... IntroTexture;` `... UMediaSoundComponent ... IntroSound;` (forward declare the classes).
6. Source/RRGame/RRGame.Build.cs: add "MediaAssets" to PublicDependencyModuleNames.
Includes: "MediaPlayer.h", "FileMediaSource.h", "MediaTexture.h", "MediaSoundComponent.h", "Misc/Paths.h".
Verify each API in A:/UNREAL/UE_5.8/Engine/Plugins/Media/MediaAssets/... or Engine/Source/Runtime/MediaAssets before use.
Do not edit any other file.
