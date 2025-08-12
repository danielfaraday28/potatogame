## Brotato MVP – LLM Development Context (Always On)

This project is a C++17 + SDL2 top-down survival shooter with a wave system and a between-wave shop. Use this guide to implement new features safely and consistently.

### Architecture overview
- **Core loop**: `Game` manages init, run, update, render, cleanup.
  - Window/renderer: SDL2; text: bitmap fallback or SDL_ttf.
  - Assets: loaded via SDL_image; copied to executable dir by CMake post-build.
- **Player**: movement, stats, weapons, leveling.
  - Aiming uses mouse; weapons orbit player and auto-fire if ready.
- **Weapons/Bullets**: `Weapon` defines stats + firing, `Bullet` handles travel/collision.
  - Ranged (pistol/SMG) and melee (brick-on-stick) supported.
- **Enemies**: simple AI, animated sprites; spawned using telegraphed indicators.
- **Progression**: XP orbs and materials; wave-based timer increases per wave.
- **Shop**: opens after each wave; buy weapons; supports reroll/lock; shows owned weapons and allows selling for 50%.

### Key files and responsibilities
- `src/Game.{h,cpp}`: main loop, input routing, wave/shop state machine, collision systems, UI rendering helpers: `renderNumber`, `renderText`, `renderTTFText`.
- `src/Player.{h,cpp}`: movement/input, stats, level-ups, weapon inventory API: `addWeapon`, `removeWeaponAt`, `updateWeapons`, `renderWeapons`.
- `src/Weapon.{h,cpp}`: `WeaponType`, `WeaponTier`, stats init, firing, rendering. Melee uses a timed swing.
- `src/Bullet.{h,cpp}`: projectile lifetime, damage, speed, ownership.
- `src/Enemy.{h,cpp}` (+ `SlimeEnemy`, `PebblinEnemy`): movement, sprite animation, damage.
- `src/ExperienceOrb.{h,cpp}`, `src/Material.{h,cpp}`: pickups and bagging system.
- `src/Shop.{h,cpp}`: shop UI, generation, purchase/sell, reroll/lock, mouse and keyboard handling.

### Rendering and coordinates
- Fixed virtual resolution: `WINDOW_WIDTH = 1920`, `WINDOW_HEIGHT = 1080` in `Game`.
- UI text: prefer `Game::renderTTFText`; fallback to `renderText`/`renderNumber` if no TTF font.
- Keep UI draws in `Game::renderUI` and `Shop::render`.

### Assets and paths
- Sprites under `assets/` and `monsters/`.
- `CMakeLists.txt` copies `assets/` and `monsters/` next to the executable after build.
- Add new art under those directories and reference relative paths from the executable.

### Build and dependencies
- C++17, SDL2, SDL2_image, SDL2_ttf via vcpkg.
- Cross-platform: Windows and macOS guides provided (`README.md`, `README-mac.md`).
- CMake finds packages via `find_package(... CONFIG REQUIRED)`; do not hardcode library paths.

### Input flow
- `Game::handleEvents` routes:
  - If shop active: `Shop::handleInput` (keyboard) and `Shop::handleMouseInput` (mouse).
  - Else: `Player::handleInput` and `Player::updateShootDirection(mousePos)`.
- Avoid conflicting input when shop is open.

### Collision and updates
- `Game::checkCollisions`: bullet vs enemies, enemy bullets vs player, melee via `checkMeleeAttacks`.
- Use `deltaTime` for movement/animations; avoid blocking operations in update paths.

### Memory management and style
- Use `std::unique_ptr` for game objects (`enemies`, `bullets`, `weapons`, `experienceOrbs`, `materials`).
- Prefer `std::make_unique` and early returns; avoid raw `new/delete`.
- Keep functions small and focused; match current formatting; do not reformat unrelated code.

### Extension checklists

#### Add a new weapon
1. Extend `enum class WeaponType` in `Weapon.h`.
2. In `Weapon::loadWeaponTexture`, map the type (and tier if applicable) to a sprite under `assets/weapons/`.
3. Add a stats initializer (like `initializePistolStats`) and call it from the constructor switch.
4. Adjust `Weapon::fire` for special behaviors (inaccuracy, pierce, status effects, etc.).
5. Show the weapon in the shop: ensure `Shop::generateItems` can pick the type and `Shop::renderShopItem` displays it.
6. Provide icon(s) for `Shop` (`texWeapon...`) if needed.

#### Add a new enemy type
1. Create a subclass (or factory) similar to `SlimeEnemy`/`PebblinEnemy` with unique stats/behavior.
2. Provide idle/hit sprites in `monsters/...` and load via `IMG_Load`.
3. Add a spawn path in `Game::updateSpawnIndicators` and type selection in `Game::spawnEnemies`.
4. Ensure damage and collision work with `Bullet` and melee.

#### Add consumable items (not yet implemented)
1. Introduce `ItemType` and `ShopItemType::ITEM` flow in `Shop`.
2. Add a `Player` items/consumables inventory and a `useItem` pathway (e.g., keybind or click in shop).
3. Implement effects (e.g., heal, bomb AOE) in `Game`/`Player` with clear lifetimes and visuals.
4. Render consumables in the shop and owned items panel. Define pricing and sell rules.

#### Add a new UI element
1. Render in `Game::renderUI` or `Shop::render` using `renderTTFText` where possible.
2. Respect the 1920x1080 coordinate system and existing margins/panels.
3. Avoid per-frame allocations; precompute strings where necessary.

### Shop-specific rules
- Generation: `Shop::generateItems` (currently weapons only) with wave-based tiering.
- Purchase: `Shop::buyItem` checks materials, adds weapon, deducts cost, removes card.
- Sell: clicking an owned weapon in the bottom-right panel sells it for 50% of computed price via `sellOwnedWeapon`.
- Reroll/lock: `Shop::reroll`, `Shop::lockItem`; respect locked items across rerolls.

### UI/UX consistency
- Keep labels uppercased where current UI does (e.g., WEAPON SHOP, REROLL, CLOSE).
- Use existing colors and spacing; keep four-item layouts and stats panel sizing consistent.

### Performance tips
- Do not perform file I/O or expensive allocations inside per-frame loops.
- Use static RNGs (`std::mt19937`) where appropriate.
- Cap entity counts or add simple culling if needed.

### Definition references (quick API)
- `Game`: `getRenderer()`, `getPlayer()`, `getPlayerMaterials()`, `renderText`, `renderTTFText`.
- `Player`: `getStats()`, `addWeapon(...)`, `removeWeaponAt(int)`, `getWeaponCount()`, `getWeapons()`, `getWeapon(int)`.
- `Weapon`: `canFire()`, `calculateDamage(...)`, `isMeleeWeapon()`, `getAttackProgress()`, `getWeaponTipPosition(...)`.
- `Shop`: `openShop(int)`, `closeShop()`, `generateItems(...)`, `buyItem(int, Player&)`, `sellOwnedWeapon(int, Player&)`.

### Asset/convention reminders
- Place new weapon sprites in `assets/weapons/` and enemy sprites in `monsters/...`.
- Keep filenames lowercase and consistent; update loaders accordingly.
- Verify CMake post-build copies pick up your new assets (they copy entire directories by default).

### Cross-platform notes
- Use the vcpkg toolchain for dependencies; do not add manual include/link paths.
- Test Windows and macOS builds per `README.md` and `README-mac.md`.

### Done criteria for new features
- Compiles on both platforms with vcpkg toolchain.
- Assets load without runtime warnings.
- UI text readable with and without TTF font.
- No gameplay updates occur when the shop is active.
- No linter or obvious runtime errors introduced.


