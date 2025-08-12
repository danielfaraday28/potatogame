# Brotato Feature Parity List

Based on research of the original Brotato game, here's what we'd need to implement to achieve feature parity:

## Core Game Systems

### Character System
- Multiple playable characters (44+ characters in Brotato)
- Each character has unique starting stats and bonuses/penalties
- Character-specific starting weapons and items
- Character unlock conditions and progression
- Character rewards (items unlocked after completing runs)

### Weapon System
- 6 weapon slots maximum
- Weapon tiers (I-IV, grey to red rarity)
- Weapon categories: Melee, Ranged, Precise, Primitive, Medical, Elemental, etc.
- Weapon scaling with character stats
- Weapon upgrading/combining system
- Massive weapon variety (100+ weapons)

### Item/Equipment System
- Items that modify character stats
- Item tiers and rarity system
- Negative stat items (trade-offs)
- Item synergies with weapon types
- Equipment slots and inventory management

### Stats System
- Core stats: Health, Armor, Dodge, Speed, Damage, Attack Speed, Range, Luck, Harvesting, Engineering
- Stat scaling and diminishing returns
- Percentage vs flat bonuses
- Critical hit system
- Life steal mechanics

### Shop System
- Between-wave shop interface
- Item/weapon purchasing with materials
- Shop reroll mechanics
- Weapon recycling for materials
- Price scaling based on character/items
- Item locking system

### Wave/Combat System
- 20 waves per run (+ boss wave)
- Wave-based enemy spawning
- Different enemy types with unique behaviours
- Elite enemies and bosses
- Enemy scaling with difficulty and wave number
- Experience orb collection

### Progression System
- Material (currency) collection
- Experience and leveling
- Stat upgrades between waves
- Character unlocks through achievements
- Item unlocks through character completion

### Difficulty System
- 5 difficulty levels (Danger 0-5)
- Difficulty sliders for enemy HP, speed, and damage
- Scaling enemy stats with difficulty

### Map/Arena System
- Single arena-based combat
- Tree destruction for materials
- Map size variations
- Spawn point management

### Audio/Visual Polish
- Damage numbers and visual feedback
- Screen effects (hit feedback, explosions)
- UI animations and polish
- Sound effects for all actions

## Advanced Features

### Build Diversity
- Tag system for weapons/items (affects shop spawning)
- Synergy bonuses between items
- Multiple viable build paths per character

### Endless Mode
- Post-wave 20 infinite scaling
- Endless-specific balancing

### Modding Support
- Steam Workshop integration
- Custom characters, weapons, items
- Gameplay modifications

### Quality of Life
- Run statistics and tracking
- Build screenshots/sharing
- Pause functionality
- Settings and key bindings

### Balance Systems
- Weapon effectiveness curves
- Stat scaling formulas
- Enemy spawn algorithms
- Economic balance (material costs vs rewards)

## Technical Requirements

### Engine Features
- Efficient entity management for hundreds of enemies
- Particle systems for effects
- UI framework for complex menus
- Save/load system for progression
- Performance optimisation for bullet-hell scenarios

### Data Systems
- JSON-based item/weapon/character definitions
- Localisation support
- Achievement system
- Statistics tracking
- Mod loading framework

