/*
 * Copyright (C) 2008 - 2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2010 - 2011 Myth Project <http://bitbucket.org/sun/myth-core/>
 *
 * Myth Project's source is based on the Trinity Project source, you can find the
 * link to that easily in Trinity Copyrights. Myth Project is a private community.
 * To get access, you either have to donate or pass a developer test.
 * You can't share Myth Project's sources! Only for personal use.
 */

#ifndef SC_CREATURE_H
#define SC_CREATURE_H

#include "Creature.h"
#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "InstanceScript.h"

#define SCRIPT_CAST_TYPE dynamic_cast

#define MAX_AGGRO_PULSE_TIMER            5000

#define CAST_PLR(a)     (SCRIPT_CAST_TYPE<Player*>(a))
#define CAST_CRE(a)     (SCRIPT_CAST_TYPE<Creature*>(a))
#define CAST_SUM(a)     (SCRIPT_CAST_TYPE<TempSummon*>(a))
#define CAST_PET(a)     (SCRIPT_CAST_TYPE<Pet*>(a))
#define CAST_AI(a,b)    (SCRIPT_CAST_TYPE<a*>(b))
#define CAST_INST(a,b)  (SCRIPT_CAST_TYPE<a*>(b))

#define GET_SPELL(a)    (const_cast<SpellEntry*>(GetSpellStore()->LookupEntry(a)))

class InstanceScript;

class SummonList : public std::list<uint64>
{
    public:
        explicit SummonList(Creature* creature) : me(creature) {}
        void Summon(Creature* summon) { push_back(summon->GetGUID()); }
        void Despawn(Creature* summon) { remove(summon->GetGUID()); }
        void DespawnEntry(uint32 entry);
        void DespawnAll();
        void DoAction(uint32 entry, int32 info);
        void DoZoneInCombat(uint32 entry = 0);
        void RemoveNotExisting();
        bool HasEntry(uint32 entry);
    private:
        Creature* me;
};

struct ScriptedAI : public CreatureAI
{
    explicit ScriptedAI(Creature* creature);
    virtual ~ScriptedAI() {}

    // *************
    //CreatureAI Functions
    // *************

    void AttackStartNoMove(Unit* target);

    // Called at any Damage from any attacker (before damage apply)
    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) {}

    //Called at World update tick
    virtual void UpdateAI(uint32 const diff);

    //Called at creature death
    void JustDied(Unit* /*killer*/) {}

    //Called at creature killing another unit
    void KilledUnit(Unit* /*victim*/) {}

    // Called when the creature summon successfully other creature
    void JustSummoned(Creature* /*summon*/) {}

    // Called when a summoned creature is despawned
    void SummonedCreatureDespawn(Creature* /*summon*/) {}

    // Called when hit by a spell
    void SpellHit(Unit* /*caster*/, SpellEntry const* /*spell*/) {}

    // Called when spell hits a target
    void SpellHitTarget(Unit* /*target*/, SpellEntry const* /*spell*/) {}

    //Called at waypoint reached or PointMovement end
    void MovementInform(uint32 /*type*/, uint32 /*id*/) {}

    // Called when AI is temporarily replaced or put back when possess is applied or removed
    void OnPossess(bool /*apply*/) {}

    // *************
    // Variables
    // *************

    //Pointer to creature we are manipulating
    Creature* me;

    //For fleeing
    bool IsFleeing;

    // *************
    //Pure virtual functions
    // *************

    //Called at creature reset either by death or evade
    void Reset() {}

    //Called at creature aggro either by MoveInLOS or Attack Start
    void EnterCombat(Unit* /*victim*/) {}

    // *************
    //AI Helper Functions
    // *************

    //Start movement toward victim
    void DoStartMovement(Unit* target, float distance = 0.0f, float angle = 0.0f);

    //Start no movement on victim
    void DoStartNoMovement(Unit* target);

    //Stop attack of current victim
    void DoStopAttack();

    //Cast spell by spell info
    void DoCastSpell(Unit* target, SpellEntry const* spellInfo, bool triggered = false);

    //Plays a sound to all nearby players
    void DoPlaySoundToSet(WorldObject* source, uint32 soundId);

    //Drops all threat to 0%. Does not remove players from the threat list
    void DoResetThreat();

    float DoGetThreat(Unit* unit);
    void DoModifyThreatPercent(Unit* unit, int32 pct);

    void DoTeleportTo(float x, float y, float z, uint32 time = 0);
    void DoTeleportTo(float const pos[4]);

    //Teleports a player without dropping threat (only teleports to same map)
    void DoTeleportPlayer(Unit* unit, float x, float y, float z, float o);
    void DoTeleportAll(float x, float y, float z, float o);

    //Returns friendly unit with the most amount of hp missing from max hp
    Unit* DoSelectLowestHpFriendly(float range, uint32 minHPDiff = 1);

    //Returns a list of friendly CC'd units within range
    std::list<Creature*> DoFindFriendlyCC(float range);

    //Returns a list of all friendly units missing a specific buff within range
    std::list<Creature*> DoFindFriendlyMissingBuff(float range, uint32 spellId);

    //Return a player with at least minimumRange from me
    Player* GetPlayerAtMinimumRange(float minRange);

    //Spawns a creature relative to me
    Creature* DoSpawnCreature(uint32 entry, float offsetX, float offsetY, float offsetZ, float angle, uint32 type, uint32 despawntime);

    bool HealthBelowPct(uint32 pct) const { return me->HealthBelowPct(pct); }
    bool HealthAbovePct(uint32 pct) const { return me->HealthAbovePct(pct); }

    //Returns spells that meet the specified criteria from the creatures spell list
    SpellEntry const* SelectSpell(Unit* target, uint32 school, uint32 mechanic, SelectTargetType targets, uint32 powerCostMin, uint32 powerCostMax, float rangeMin, float rangeMax, SelectEffect effect);

    //Checks if you can cast the specified spell
    bool CanCast(Unit* target, SpellEntry const* spell, bool triggered = false);

    void SetEquipmentSlots(bool loadDefault, int32 mainHand = EQUIP_NO_CHANGE, int32 offHand = EQUIP_NO_CHANGE, int32 ranged = EQUIP_NO_CHANGE);

    //Generally used to control if MoveChase() is to be used or not in AttackStart(). Some creatures does not chase victims
    void SetCombatMovement(bool allowMovement);
    bool IsCombatMovementAllowed() { return _isCombatMovementAllowed; }

    bool EnterEvadeIfOutOfCombatArea(uint32 const diff);

    // return true for heroic mode. i.e.
    //   - for dungeon in mode 10-heroic,
    //   - for raid in mode 10-Heroic
    //   - for raid in mode 25-heroic
    // DO NOT USE to check raid in mode 25-normal.
    bool IsHeroic() { return _isHeroic; }

    // return the dungeon or raid difficulty
    Difficulty GetDifficulty() { return _difficulty; }

    // return true for 25 man or 25 man heroic mode
    bool Is25ManRaid() { return _difficulty & 1; }

    template<class T> inline
    const T& DUNGEON_MODE(const T& normal5, const T& heroic10)
    {
        switch (_difficulty)
        {
            case DUNGEON_DIFFICULTY_NORMAL:
                return normal5;
            case DUNGEON_DIFFICULTY_HEROIC:
                return heroic10;
            default:
                break;
        }

        return heroic10;
    }

    template<class T> inline
    const T& RAID_MODE(const T& normal10, const T& normal25)
    {
        switch (_difficulty)
        {
            case RAID_DIFFICULTY_10MAN_NORMAL:
                return normal10;
            case RAID_DIFFICULTY_25MAN_NORMAL:
                return normal25;
            default:
                break;
        }

        return normal25;
    }

    template<class T> inline
    const T& RAID_MODE(const T& normal10, const T& normal25, const T& heroic10, const T& heroic25)
    {
        switch (_difficulty)
        {
            case RAID_DIFFICULTY_10MAN_NORMAL:
                return normal10;
            case RAID_DIFFICULTY_25MAN_NORMAL:
                return normal25;
            case RAID_DIFFICULTY_10MAN_HEROIC:
                return heroic10;
            case RAID_DIFFICULTY_25MAN_HEROIC:
                return heroic25;
            default:
                break;
        }

        return heroic25;
    }

    void SetImmuneToPushPullEffects(bool set)
    {
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, set);
        me->ApplySpellImmune(0, IMMUNITY_ID, 49560, set);
    }

    private:
        Difficulty _difficulty;
        uint32 _evadeCheckCooldown;
        bool _isCombatMovementAllowed;
        bool _isHeroic;
};

struct Scripted_NoMovementAI : public ScriptedAI
{
    Scripted_NoMovementAI(Creature* creature) : ScriptedAI(creature)
    {
        SetImmuneToPushPullEffects(true);
    }
    virtual ~Scripted_NoMovementAI() {}

    //Called at each attack of me by any victim
    void AttackStart(Unit* target);
};

class BossAI : public ScriptedAI
{
    public:
        BossAI(Creature* creature, uint32 bossId);
        virtual ~BossAI() {}

        uint32 inFightAggroCheck_Timer;
        InstanceScript* const instance;
        BossBoundaryMap const* GetBoundary() const { return _boundary; }

        void JustSummoned(Creature* summon);
        void SummonedCreatureDespawn(Creature* summon);

        void UpdateAI(uint32 const diff) = 0;

        void Reset() { _Reset(); }
        void EnterCombat(Unit* /*who*/) { _EnterCombat(); }
        void JustDied(Unit* /*killer*/) { _JustDied(); }
        void JustReachedHome() { _JustReachedHome(); }

    protected:
        void _Reset();
        void _EnterCombat();
        void _JustDied();
        void _JustReachedHome() { me->setActive(false); }
        void _DoAggroPulse(const uint32 diff);

        bool CheckInRoom()
        {
            if (CheckBoundary(me))
                return true;

            EnterEvadeMode();
            return false;
        }

        bool CheckBoundary(Unit* who);
        void TeleportCheaters();

        EventMap events;
        SummonList summons;

    private:
        BossBoundaryMap const* const _boundary;
        const uint32 _bossId;
};

// SD2 grid searchers.
Creature* GetClosestCreatureWithEntry(WorldObject* source, uint32 entry, float maxSearchRange, bool alive = true);
GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange);
void GetCreatureListWithEntryInGrid(std::list<Creature*>& list, WorldObject* source, uint32 entry, float maxSearchRange);
void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& list, WorldObject* source, uint32 entry, float maxSearchRange);

#endif
