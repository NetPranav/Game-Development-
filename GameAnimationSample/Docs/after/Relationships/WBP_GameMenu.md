# WBP_GameMenu

**Asset Path:** `/Game/AAA/UI/WBP_GameMenu`
**C++ Base Class:** `UUserWidget`
**Origin:** Custom Widget Blueprint.

## Summary
The master UI container for the game (RE2-style asymmetric layout). It handles background blurring, tabs, inventory grid layout, and item details. 

## Upstream Connections (What uses this?)
- **BP_AAAHUD:** Set as the `Game Menu Class` in Class Defaults. The HUD is responsible for creating this widget, adding it to the viewport, and removing it. 

## Downstream Connections (What does this require?)
- **FadeInAnim:** A UMG Timeline animation that must be configured to `Play When Paused` on `Event Construct`. 
- **Inventory Component:** (Future) Will pull item data from the player's `AAAInventoryComponent` to populate the grid.
