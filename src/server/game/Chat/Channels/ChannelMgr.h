/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __TRINITY_CHANNELMGR_H
#define __TRINITY_CHANNELMGR_H

#include <memory>
#include "Common.h"
#include "Channel.h"
#include "ace/Singleton.h"

#include <map>
#include <string>

#include "World.h"

class ChannelMgr
{
    public:
        uint32 team;
        typedef std::map<std::wstring,Channel*> ChannelMap;

        explicit ChannelMgr(uint32 team)
            : team(std::move(team))
        {
        }

        ~ChannelMgr();

        Channel *GetJoinChannel(std::string name, uint32 channel_id);
        Channel *GetChannel(std::string name, Player *p, bool pkt = true);
        void LeftChannel(std::string name);
    private:
        ChannelMap channels;
        void MakeNotOnPacket(WorldPacket *data, std::string name);
};

class AllianceChannelMgr
    : public ChannelMgr
{
public:
    AllianceChannelMgr();
};

class HordeChannelMgr
    : public ChannelMgr
{
public:
    HordeChannelMgr();
};

ChannelMgr* channelMgr(uint32 team);

#endif

