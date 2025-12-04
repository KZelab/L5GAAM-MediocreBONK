#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include <SFML/System/Vector2.hpp>

namespace MediocreBONK::Managers
{
    /*
     * DESIGN PATTERN: OBSERVER PATTERN / PUBLISH-SUBSCRIBE (PUB-SUB)
     *
     * Purpose:
     * - Decouple event producers from event consumers
     * - One-to-many notification (one event, multiple listeners)
     * - Loose coupling between game systems
     *
     * Key Concepts:
     * - Publisher (emitter): Fires events when something happens
     * - Subscriber (listener): Registers callbacks to receive events
     * - Event: Data passed from publisher to subscribers
     *
     * Example Flow:
     * 1. ParticleSystem subscribes to PlayerLevelUp event
     * 2. XPSystem detects player leveled up
     * 3. XPSystem emits PlayerLevelUp event
     * 4. EventManager calls all subscribed callbacks
     * 5. ParticleSystem receives event, spawns level-up particles
     *
     * Benefits:
     * - Systems don't need direct references to each other
     * - Easy to add new listeners without modifying emitters
     * - Centralized event logging/debugging
     * - Can defer events (queue) or process immediately (emit)
     *
     * Trade-offs:
     * - Harder to trace event flow (implicit connections)
     * - Memory overhead from storing callbacks
     * - Can create "event storms" if not careful
     *
     * Queue vs Immediate:
     * - queueEvent(): Store for later (processed at end of frame)
     * - emit(): Process immediately (during current function)
     * - Queuing prevents mid-update modification issues
     */

    // Event types in the game
    enum class GameEventType
    {
        EnemyKilled,
        PlayerLevelUp,
        PlayerDamaged,
        PlayerHealed,
        XPCollected,
        ProjectileFired,
        PowerUpCollected,
        BuffApplied,
        BuffExpired,
        WaveCompleted,
        BossSpawned
    };

    // Base event data
    struct EventData
    {
        virtual ~EventData() = default;
    };

    // Specific event data types
    struct EnemyKilledData : EventData
    {
        float experienceValue;
        sf::Vector2f position;
    };

    struct PlayerLevelUpData : EventData
    {
        int newLevel;
        int previousLevel;
    };

    struct PlayerDamagedData : EventData
    {
        float damageAmount;
        float remainingHealth;
    };

    struct XPCollectedData : EventData
    {
        float xpAmount;
        float totalXP;
    };

    struct ProjectileFiredData : EventData
    {
        sf::Vector2f position;
        sf::Vector2f direction;
        float damage;
    };

    struct BuffAppliedData : EventData
    {
        std::string buffName;
        float duration;
    };

    // Event listener (callback) type
    // Function that takes event data and returns nothing
    using EventListener = std::function<void(const EventData*)>;

    /*
     * SINGLETON + OBSERVER PATTERN
     * - Singleton: Single event manager for entire game
     * - Observer: Manages subscriptions and notifications
     */
    class EventManager
    {
    public:
        // SINGLETON PATTERN: Global event bus
        static EventManager& getInstance()
        {
            static EventManager instance;
            return instance;
        }

        EventManager(const EventManager&) = delete;
        EventManager& operator=(const EventManager&) = delete;

        // OBSERVER PATTERN: Subscribe to event type
        // Returns: Unique listener ID (used for unsubscribing)
        int subscribe(GameEventType eventType, EventListener listener)
        {
            int listenerId = nextListenerId++;
            listeners[eventType].emplace_back(listenerId, listener);
            return listenerId;
        }

        // OBSERVER PATTERN: Unsubscribe from event type
        void unsubscribe(GameEventType eventType, int listenerId)
        {
            auto& eventListeners = listeners[eventType];
            eventListeners.erase(
                std::remove_if(eventListeners.begin(), eventListeners.end(),
                    [listenerId](const auto& pair) { return pair.first == listenerId; }),
                eventListeners.end()
            );
        }

        // PUB-SUB: Emit event immediately (synchronous)
        // Calls all subscribers right now (within this function call)
        // Danger: Can cause reentrancy issues if listener modifies event state
        void emit(GameEventType eventType, const EventData* data = nullptr)
        {
            auto it = listeners.find(eventType);
            if (it != listeners.end())
            {
                for (const auto& [id, listener] : it->second)
                {
                    listener(data); // Call subscriber's callback
                }
            }
        }

        // PUB-SUB: Queue event for later processing (deferred)
        // Safer: Events processed at controlled time (end of frame)
        // Avoids mid-update modification issues
        void queueEvent(GameEventType eventType, std::unique_ptr<EventData> data = nullptr)
        {
            eventQueue.emplace_back(eventType, std::move(data));
        }

        // Process all queued events (called once per frame)
        // This is when deferred events actually fire
        void processEvents()
        {
            for (auto& [eventType, data] : eventQueue)
            {
                emit(eventType, data.get()); // Emit each queued event
            }
            eventQueue.clear(); // Clear queue for next frame
        }

        // Clear all listeners
        void clearAll()
        {
            listeners.clear();
            eventQueue.clear();
        }

        // Clear listeners for a specific event type
        void clearEventListeners(GameEventType eventType)
        {
            listeners.erase(eventType);
        }

    private:
        // SINGLETON PATTERN: Private constructor
        EventManager()
            : nextListenerId(0)
        {}

        // OBSERVER PATTERN: Map of event type -> list of subscribers
        // Each subscriber has an ID (for unsubscribing) and callback function
        std::unordered_map<GameEventType, std::vector<std::pair<int, EventListener>>> listeners;

        // DEFERRED EXECUTION: Queue of events to process later
        // Prevents mid-update modification issues
        std::vector<std::pair<GameEventType, std::unique_ptr<EventData>>> eventQueue;

        // ID generator for unique listener IDs
        int nextListenerId;
    };
}
