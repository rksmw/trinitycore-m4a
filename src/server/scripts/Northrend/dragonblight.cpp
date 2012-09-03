/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Dragonblight
SD%Complete: 100
SDComment:
SDCategory: Dragonblight
EndScriptData */

/* ContentData
npc_alexstrasza_wr_gate
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

enum eEnums
{
    QUEST_RETURN_TO_AG_A    = 12499,
    QUEST_RETURN_TO_AG_H    = 12500,
    MOVIE_ID_GATES          = 14
};

#define GOSSIP_ITEM_WHAT_HAPPENED   "Alexstrasza, can you show me what happened here?"

class npc_alexstrasza_wr_gate : public CreatureScript
{
public:
    npc_alexstrasza_wr_gate() : CreatureScript("npc_alexstrasza_wr_gate") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestRewardStatus(QUEST_RETURN_TO_AG_A) || player->GetQuestRewardStatus(QUEST_RETURN_TO_AG_H))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_WHAT_HAPPENED, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            player->CLOSE_GOSSIP_MENU();
            player->SendMovieStart(MOVIE_ID_GATES);
        }

        return true;
    }
};

/*######
## Quest Strengthen the Ancients (12096|12092)
######*/

enum StrengthenAncientsMisc
{
    SAY_WALKER_FRIENDLY = 0,
    SAY_WALKER_ENEMY = 1,
    SAY_LOTHALOR = 0,

    SPELL_CREATE_ITEM_BARK = 47550,
    SPELL_CONFUSED = 47044,

    NPC_LOTHALOR = 26321,

    FACTION_WALKER_ENEMY = 14,
};

class spell_q12096_q12092_dummy : public SpellScriptLoader // Strengthen the Ancients: On Interact Dummy to Woodlands Walker
{
public:
    spell_q12096_q12092_dummy() : SpellScriptLoader("spell_q12096_q12092_dummy") { }

    class spell_q12096_q12092_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_q12096_q12092_dummy_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            uint32 roll = rand() % 2;

            Creature* tree = GetHitCreature();
            Player* player = GetCaster()->ToPlayer();

            if (!tree || !player)
                return;

            tree->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

            if (roll == 1) // friendly version
            {
                tree->CastSpell(player, SPELL_CREATE_ITEM_BARK);
                tree->AI()->Talk(SAY_WALKER_FRIENDLY, player->GetGUID());
                tree->DespawnOrUnsummon(1000);
            }
            else if (roll == 0) // enemy version
            {
                tree->AI()->Talk(SAY_WALKER_ENEMY, player->GetGUID());
                tree->setFaction(FACTION_WALKER_ENEMY);
                tree->Attack(player, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_q12096_q12092_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_q12096_q12092_dummy_SpellScript();
    }
};

class spell_q12096_q12092_bark : public SpellScriptLoader // Bark of the Walkers
{
public:
    spell_q12096_q12092_bark() : SpellScriptLoader("spell_q12096_q12092_bark") { }

    class spell_q12096_q12092_bark_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_q12096_q12092_bark_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Creature* lothalor = GetHitCreature();
            if (!lothalor || lothalor->GetEntry() != NPC_LOTHALOR)
                return;

            lothalor->AI()->Talk(SAY_LOTHALOR);
            lothalor->RemoveAura(SPELL_CONFUSED);
            lothalor->DespawnOrUnsummon(4000);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_q12096_q12092_bark_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_q12096_q12092_bark_SpellScript();
    }
};

#define SPELL_SEEDS_OF_NATURES_WRATH 49587
#define QUEST_THAT_WHICH_CREATES_CAN_ALSO_DESTROY 12459

#define NPC_WEAKENED_REANIMATED_FROST_WYRM 27821
#define NPC_WEAKENED_TURGID_THE_VILE 27809
#define NPC_WEAKENED_OVERSEER_DEATHGAZE 27807

struct npc_reanimated_frost_wormAI : public ScriptedAI
{
    npc_reanimated_frost_wormAI(Creature *c) : ScriptedAI(c) {}

    void SpellHit(Unit *hitter, const SpellInfo *spell) override
    {
        if ((spell->Id == SPELL_SEEDS_OF_NATURES_WRATH) &&
            (hitter->GetTypeId() == TYPEID_PLAYER) &&
            (CAST_PLR(hitter)->IsActiveQuest(QUEST_THAT_WHICH_CREATES_CAN_ALSO_DESTROY)))
        {
            me->UpdateEntry(NPC_WEAKENED_REANIMATED_FROST_WYRM);
        }
    }
};

struct npc_turgid_the_vileAI : public ScriptedAI
{
    npc_turgid_the_vileAI(Creature *c) : ScriptedAI(c) {}

    void SpellHit(Unit *hitter, const SpellInfo *spell) override
    {
        if ((spell->Id == SPELL_SEEDS_OF_NATURES_WRATH) &&
            (hitter->GetTypeId() == TYPEID_PLAYER) &&
            (CAST_PLR(hitter)->IsActiveQuest(QUEST_THAT_WHICH_CREATES_CAN_ALSO_DESTROY)))
        {
            me->UpdateEntry(NPC_WEAKENED_TURGID_THE_VILE);
        }
    }
};

struct npc_overseer_deathgazeAI : public ScriptedAI
{
    npc_overseer_deathgazeAI(Creature *c) : ScriptedAI(c) {}

    void SpellHit(Unit *hitter, const SpellInfo *spell) override
    {
        if ((spell->Id == SPELL_SEEDS_OF_NATURES_WRATH) &&
            (hitter->GetTypeId() == TYPEID_PLAYER) &&
            (CAST_PLR(hitter)->IsActiveQuest(QUEST_THAT_WHICH_CREATES_CAN_ALSO_DESTROY)))
        {
            me->UpdateEntry(NPC_WEAKENED_OVERSEER_DEATHGAZE);
        }
    }
};

void AddSC_dragonblight()
{
    new npc_alexstrasza_wr_gate;
    new spell_q12096_q12092_dummy;
    new spell_q12096_q12092_bark;
    struct NPC_reanimated_frost_worm
        : CreatureScript
    {
        NPC_reanimated_frost_worm()
            : CreatureScript("npc_reanimated_frost_worm")
        {
        }

        CreatureAI *GetAI(Creature *creature) const override
        {
            return new npc_reanimated_frost_wormAI(std::move(creature));
        }
    };
    new NPC_reanimated_frost_worm;

    struct NPC_turgid_the_vile
        : CreatureScript
    {
        NPC_turgid_the_vile()
            : CreatureScript("npc_turgid_the_vile")
        {
        }

        CreatureAI *GetAI(Creature *creature) const override
        {
            return new npc_turgid_the_vileAI(std::move(creature));
        }
    };
    new NPC_turgid_the_vile;

    struct NPC_overseer_deathgaze
        : CreatureScript
    {
        NPC_overseer_deathgaze()
            : CreatureScript("npc_overseer_deathgaze")
        {
        }

        CreatureAI *GetAI(Creature *creature) const override
        {
            return new npc_overseer_deathgazeAI(std::move(creature));
        }
    };
    new NPC_overseer_deathgaze;
}
