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

#include "ScriptPCH.h"

#define EMOTE_FRENZY            -1469031

#define SPELL_SHADOWFLAME        22539
#define SPELL_WINGBUFFET         23339
#define SPELL_FRENZY             23342

class boss_flamegor : public CreatureScript
{
public:
    boss_flamegor() : CreatureScript("boss_flamegor") {}

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_flamegorAI (pCreature);
    }

    struct boss_flamegorAI : public ScriptedAI
    {
        boss_flamegorAI(Creature *c) : ScriptedAI(c) {}

        uint32 ShadowFlame_Timer;
        uint32 WingBuffet_Timer;
        uint32 Frenzy_Timer;

        void Reset()
        {
            ShadowFlame_Timer = 21000;
            WingBuffet_Timer = 35000;
            Frenzy_Timer = 10000;
        }

        void EnterCombat(Unit * /*who*/)
        {
            DoZoneInCombat();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (ShadowFlame_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_SHADOWFLAME);
                ShadowFlame_Timer = 15000 + rand()%7000;
            } else ShadowFlame_Timer -= diff;

            if (WingBuffet_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_WINGBUFFET);
                if (DoGetThreat(me->getVictim()))
                    DoModifyThreatPercent(me->getVictim(),-75);

                WingBuffet_Timer = 25000;
            } else WingBuffet_Timer -= diff;

            if (Frenzy_Timer <= diff)
            {
                DoScriptText(EMOTE_FRENZY, me);
                DoCast(me, SPELL_FRENZY);
                Frenzy_Timer = urand(8000, 10000);
            } else Frenzy_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_flamegor()
{
    new boss_flamegor();
}
