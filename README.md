# MediocreBONK

A Vampire Survivors-style game built with C++ and SFML 3

## Project Purpose

This project demonstrates fundamental game development patterns and architecture:
- **Entity-Component-System (ECS)** architecture
- **State Machine** pattern for game flow
- **Singleton** pattern for manager classes
- **Object Pooling** for performance optimization
- **Observer Pattern** for event-driven communication
- **Fixed Timestep** game loop
- **Spatial Partitioning** for collision optimization

##  Gameplay

- Top-down arena shooter
- Automatic weapon firing
- Wave-based enemy spawning
- XP and leveling system
- Upgrade selection on level up
- Progressive difficulty scaling
- Survival-focused gameplay


## 🏗️ Architecture Overview

```
MediocreBONK/
│
├── src/
│   ├── Core/              # Game loop, state machine, resource management
│   │   ├── Game.h         # Fixed timestep game loop
│   │   ├── StateMachine.h # Stack-based state management
│   │   └── ResourceManager.h # Singleton resource caching
│   │
│   ├── ECS/               # Entity-Component-System
│   │   ├── Entity.h       # Component container
│   │   ├── Component.h    # Base component class
│   │   ├── EntityManager.h # Entity factory with object pooling
│   │   └── Components/    # Game-specific components
│   │
│   ├── States/            # Game states (Menu, Game, Death)
│   ├── Systems/           # ECS systems (Collision, Weapon, Spawn, etc.)
│   ├── Managers/          # Singleton managers (Audio, Camera, Events, etc.)
│   ├── Entities/          # Entity prefabs (Player, Enemy, PowerUp)
│   ├── UI/                # HUD, menus, notifications
│   └── Utils/             # Utilities (Logger, Math, Random, SpatialGrid)
│
└── assets/
    └── fonts/             # UI fonts
```

## 🔧 Design Patterns Used

| Pattern | Location | Purpose |
|---------|----------|---------|
| **Singleton** | ResourceManager, AudioManager, EventManager, CameraManager | Global access to shared systems |
| **State Machine** | StateMachine, State classes | Manage game flow (Menu → Game → Death) |
| **ECS** | Entity, Component, EntityManager | Composition over inheritance for game objects |
| **Object Pooling** | EntityManager, AudioManager | Reuse objects to avoid allocations |
| **Observer/Pub-Sub** | EventManager | Decouple systems via events |
| **Spatial Partitioning** | SpatialGrid | O(n) collision detection |
| **Fixed Timestep** | Game::run() | Deterministic physics simulation |
| **Template Method** | Component lifecycle hooks | Extensible component behavior |



### Quick Summary:

1. **Phase 0**: Project setup (SFML window)
2. **Phase 1**: Foundation (Logger, Math, Random)
3. **Phase 2**: Game loop (Fixed timestep, ResourceManager)
4. **Phase 3**: State machine (Menu, state transitions)
5. **Phase 4**: ECS architecture (Entity, Component, EntityManager)
6. **Phase 5**: Basic gameplay (Player, movement, GameState)
7. **Phase 6**: World & camera (Infinite world, camera follow)
8. **Phase 7**: Combat (Enemies, spawning, collision, weapons)
9. **Phase 8**: Progression (XP, leveling, upgrades)
10. **Phase 9**: Polish (Particles, audio, difficulty)


## 📖 Key Learning Points

### 1. Fixed Timestep Game Loop
```cpp
while (timeSinceLastUpdate > timePerFrame) {
    timeSinceLastUpdate -= timePerFrame;
    update(fixedDeltaTime); // Always 16.67ms
}
render(); // Variable rate
```
**Why?** Deterministic simulation, network sync, stable physics.

### 2. Entity-Component-System
```cpp
auto* entity = entityManager->createEntity();
entity->addComponent<Transform>(x, y);
entity->addComponent<Health>(100.f);
entity->addComponent<Sprite>("player.png");
```
**Why?** Composition over inheritance, flexible, data-oriented.

### 3. Object Pooling
```cpp
// Instead of: delete entity; new Entity();
entity->setActive(false); // Deactivate
// Later: reuse inactive entity
entity->setActive(true);  // Reactivate
```
**Why?** No allocations during gameplay, cache-friendly, predictable memory.

### 4. Spatial Partitioning
```cpp
// Without: O(n²) - check every pair
for (entity1 : entities)
    for (entity2 : entities)
        checkCollision(entity1, entity2); // 500,000 checks!

// With grid: O(n) - check nearby only
auto nearby = grid.query(position, radius); // ~50 checks
```
**Why?** 10-100x faster collision detection.

### 5. Observer Pattern
```cpp
// Subscribe
EventManager::subscribe(PlayerLevelUp, [](data) {
    ParticleSystem::spawnLevelUpEffect();
});

// Publish (somewhere else in code)
EventManager::emit(PlayerLevelUp, levelData);
```
**Why?** Decoupled systems, easy to extend, no hard dependencies.





### Skip in Live Demo:
❌ Utility functions (boring)
❌ All UI code (tedious)
❌ Particle systems (visual but not architectural)








## 🎯 Future Enhancements

- [ ] Multiple weapon types
- [ ] Boss enemies
- [ ] Procedural level generation
- [ ] Save/load system
- [ ] Achievements
- [ ] Particle editor
- [ ] Level editor


