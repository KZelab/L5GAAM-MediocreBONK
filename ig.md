# MediocreBONK - Implementation Guide

**A step-by-step guide for building this game from scratch, designed for live demonstration and teaching.**

This guide presents the implementation order that minimizes dependencies and allows for incremental building.

---

## Table of Contents

1. [Phase 0: Project Setup](#phase-0-project-setup)
2. [Phase 1: Foundation Layer](#phase-1-foundation-layer)
3. [Phase 2: Core Game Loop](#phase-2-core-game-loop)
4. [Phase 3: State Management](#phase-3-state-management)
5. [Phase 4: ECS Architecture](#phase-4-ecs-architecture)
6. [Phase 5: Basic Gameplay](#phase-5-basic-gameplay)
7. [Phase 6: World & Camera](#phase-6-world--camera)
8. [Phase 7: Combat Systems](#phase-7-combat-systems)
9. [Phase 8: Progression Systems](#phase-8-progression-systems)
10. [Phase 9: Polish & Juice](#phase-9-polish--juice)

---

## Phase 0: Project Setup

```cpp
// main.cpp - Minimal version
#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Test Window");

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        window.display();
    }

    return 0;
}
```

## Phase 1: Foundation Layer (Pre-provided)

**Goal**: Import utility systems to save time.

**Action**: Copy the `src/Utils` folder (excluding `SpatialGrid.h`) into your project.
- `Logger.h`
- `Math.h`
- `Random.h`

These are utility classes that are boring to implement live.

---

## Phase 2: Core Game Loop

**Goal**: Implement fixed timestep game loop

### Files to Create:

#### 2.1 Game Class (Skeleton)
```
src/Core/Game.h
```

**Why now?** Foundation for entire architecture.

**Key Features**:
- SFML window management
- Fixed timestep loop (60 Hz)
- Update/render separation

**Implementation Order**:
1. Constructor (create window)
2. `run()` method (game loop)
3. `processInput()` (event handling)
4. `update()` (game logic - empty for now)
5. `render()` (draw - black screen for now)

---

#### 2.2 Resource Manager (Singleton)
```
src/Core/ResourceManager.h
```

**Why now?** Needed before loading any assets.

**Key Features**:
- Meyer's Singleton pattern
- Texture/Font/Sound caching
- Lazy loading

---

## Phase 3: State Management

**Goal**: Implement state machine for game flow (Menu → Game → Death)

### Files to Create:

#### 3.1 State Base Class
```
src/States/State.h
```

**Why first?** Interface that all states implement.

**Key Features**:
- Pure virtual methods (interface)
- `enter()`/`exit()` lifecycle hooks
- `update()`/`render()`/`handleInput()` methods

---

#### 3.2 State Machine
```
src/Core/StateMachine.h
```

**Why now?** Manages state transitions.

**Key Features**:
- Stack-based states
- `pushState()` - Add state on top
- `popState()` - Remove current state
- `changeState()` - Replace current state

**Dependencies**: State.h

---

#### 3.3 Menu State (First Concrete State)
```
src/States/MenuState.h
```

**Why now?** Simplest state to implement, tests state machine.

**Key Features**:
- Display title text
- "Press SPACE to start"
- ESC to exit

---

## Phase 4: ECS Architecture

**Goal**: Build Entity-Component-System foundation

### Files to Create:

#### 4.1 Component Base Class
```
src/ECS/Component.h
```

**Why first?** Base class for all components.

**Key Features**:
- Virtual update/render methods
- `onAttach()`/`onDetach()` hooks
- `owner` pointer to entity
- `active` flag

---

#### 4.2 Entity Class
```
src/ECS/Entity.h
```

**Why now?** Container for components.

**Key Features**:
- `addComponent<T>()` - Template method
- `getComponent<T>()` - Type-safe retrieval
- `hasComponent<T>()` - Type checking
- Component storage (std::unordered_map)

**Dependencies**: Component.h

---

#### 4.3 Entity Manager
```
src/ECS/EntityManager.h
```

**Why now?** Factory for creating entities.

**Key Features**:
- `createEntity()` - Factory method
- `destroyEntity()` - Mark inactive (pooling)
- Query methods (by tag, by component)
- Periodic cleanup (prevent memory bloat)

**Dependencies**: Entity.h

---

#### 4.4 Basic Components (Data Only)
```
src/ECS/Components/Transform.h    (position, rotation, scale)
src/ECS/Components/Health.h       (currentHealth, maxHealth)
src/ECS/Components/Collider.h     (radius, layer)
```

**Why now?** Needed for player and enemies.

**Key Features**:
- Pure data structures
- Inherit from Component
- Simple constructors

---

## Phase 5: Basic Gameplay

**Goal**: Get a player on screen that can move

### Files to Create:

#### 5.1 Game State (Skeleton)
```
src/States/GameState.h
```

**Why now?** Main gameplay state.

**Implementation Order**:
1. Create EntityManager
2. `enter()` - Setup game
3. `exit()` - Cleanup
4. `update()` - Update entities (empty for now)
5. `render()` - Draw entities (placeholder circles)
6. `handleInput()` - ESC to return to menu

**Dependencies**: State.h, EntityManager.h

---

#### 5.2 Player Entity
```
src/Entities/Player.h
```

**Why now?** Need something to test with.

**Key Features**:
- Create entity with Transform, Health, Collider
- `handleInput()` - WASD movement
- `update()` - Apply movement
- `dash()` - Shift key for dash

---

#### 5.3 Connect Menu to Game
```
Update src/States/MenuState.h
```

**Why now?** Allow transition from menu to game.

**Implementation**:
```cpp
// MenuState::handleInput()
if (keyPressed->code == sf::Keyboard::Key::Space)
{
    stateMachine->changeState(std::make_unique<GameState>());
}
```

---

## Phase 6: World & Camera

**Goal**: Infinite world with camera following player

### Files to Create:

#### 6.1 Camera Manager (Singleton)
```
src/Managers/CameraManager.h
```

**Why now?** Before enemies (need camera for proper spawning).

**Key Features**:
- Smooth camera follow (lerp)
- Screen shake effect
- World ↔ screen coordinate conversion

**Implementation Order**:
1. `initialize()` - Setup views
2. `setFollowTarget()` - Track entity
3. `update()` - Smooth follow with lerp
4. `getGameView()` / `getUIView()` - Separate views

---

#### 6.2 World Generator
```
src/Systems/WorldGenerator.h
```

**Why now?** Makes world feel alive.

**Key Features**:
- Infinite tiling background
- Generate tiles based on camera position
- Simple visual (colored rectangles)

---

## Phase 7: Combat Systems

**Goal**: Enemies that chase and collide with player

### Files to Create:

#### 7.1 AI Component
```
src/ECS/Components/AI.h
```

**Why first?** Data for enemy behavior.

**Key Features**:
- AI type enum (Chase, Ranged, Boss)
- Speed, detection range
- State data

---

#### 7.2 Enemy Entity
```
src/Entities/Enemy.h
```

**Why now?** Something to fight.

**Key Features**:
- Create entity with Transform, Health, Collider, AI
- `update()` - Move toward player
- Different enemy types

---

#### 7.3 Spawn System
```
src/Systems/SpawnSystem.h
```

**Why now?** Automated enemy spawning.

**Key Features**:
- Spawn enemies off-screen
- Despawn enemies far from player
- Difficulty scaling over time

**Dependencies**: Enemy.h, CameraManager.h

---

#### 7.4 Spatial Grid (Optimization)
```
src/Utils/SpatialGrid.h
```

**Why now?** Before collision system (performance optimization).
**Note**: This is the only Utils class we implement live because it's crucial for understanding the optimization.

**Key Features**:
- Grid-based spatial partitioning
- O(n²) → O(n) collision detection
- Cell size = 100 pixels

---

#### 7.5 Collision System
```
src/Systems/CollisionSystem.h
```

**Why now?** Detect player-enemy collisions.

**Key Features**:
- Use SpatialGrid for broad phase
- Circle-circle collision (narrow phase)
- Damage on collision

**Dependencies**: SpatialGrid.h, Collider.h

---

#### 7.6 Death State
```
src/States/DeathState.h
```

**Why now?** Handle player death.

**Key Features**:
- Display stats (time survived, kills, level)
- SPACE to restart
- ESC to return to menu

---

#### 7.7 Weapon System
```
src/ECS/Components/Weapon.h
src/ECS/Components/Projectile.h
src/Systems/WeaponSystem.h
```

**Why now?** Player needs to fight back.

**Implementation Order**:
1. Weapon component (data)
2. Projectile component (data)
3. WeaponSystem (spawns projectiles)

**Key Features**:
- Auto-fire toward nearest enemy
- Multiple weapon types
- Projectile lifetime

---

## Phase 8: Progression Systems

**Goal**: XP, leveling, and upgrades

### Files to Create:

#### 8.1 Experience System
```
src/ECS/Components/Experience.h
src/ECS/Components/XPPickup.h
src/Systems/XPSystem.h
```

**Why now?** Core progression mechanic.

**Implementation Order**:
1. Experience component (level, XP data)
2. XPPickup component (XP value)
3. XPSystem (spawn gems, collect, level up)

**Key Features**:
- XP gems spawn when enemies die
- Gems attracted to player
- Level up triggers event

---

#### 8.2 Event Manager (Singleton)
```
src/Managers/EventManager.h
```

**Why now?** Decouple systems (level up → UI notification).

**Key Features**:
- Observer pattern / Pub-Sub
- Subscribe to event types
- Queue events (process end of frame)

---

#### 8.3 Upgrade System
```
src/Managers/UpgradeManager.h
```

**Why now?** Reward player for leveling up.
**Note**: We skip the UI implementation (`LevelUpMenu.h`) and just log the upgrades or auto-select for the demo.

**Key Features**:
- Upgrade definitions (damage, speed, health, etc.)
- Random 3 upgrade choices
- Apply upgrade to player

---

## Phase 9: Polish & Juice

**Goal**: Make game feel good

### Files to Create:

#### 9.1 Particle System
```
src/ECS/Components/Particle.h
src/Systems/ParticleSystem.h
```

**Why now?** Visual feedback.

**Key Features**:
- Spawn particle bursts on events
- Level up particles
- Damage particles
- Collection particles

---

#### 9.2 Audio Manager (Singleton)
```
src/Managers/AudioManager.h
```

**Why now?** Audio feedback.

**Key Features**:
- Object pooling (32 concurrent sounds)
- Sound effects (shoot, hit, collect)
- Background music
- Volume controls

---

#### 9.3 Difficulty Manager
```
src/Managers/DifficultyManager.h
```

**Why now?** Progressive difficulty curve.

**Key Features**:
- Enemy stat scaling over time
- Spawn rate increases
- Boss waves

---

#### 9.4 Power-Up System
```
src/Entities/PowerUp.h
src/ECS/Components/Buff.h
src/Systems/PowerUpSystem.h
```

**Why now?** Temporary power spikes.

**Key Features**:
- Spawn power-ups randomly
- Temporary buffs (speed, damage, invincibility)
- Visual effect when active

---

### Code Snippets to Prepare:

- Minimal game loop
- ECS entity creation
- State transition
- Collision detection
- Event system usage

### What to Skip in Live Demo:

- Utility functions (pre-provided)
- UI code (tedious)
- All components (show 1-2 examples)
- Particle system (visual but not architectural)

### What to Emphasize:

- ✓ Fixed timestep game loop
- ✓ State machine pattern
- ✓ ECS architecture
- ✓ Object pooling
- ✓ Spatial partitioning
- ✓ Event system
