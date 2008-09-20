// Copyright (C) 2008 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include "config.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Campaign.h"
#include "xmlhelper.h"
#include "Configuration.h"

std::string Campaign::d_tag = "campaign";

Campaign* Campaign::d_instance = 0;

Campaign* Campaign::getInstance()
{
    if (!d_instance)
        d_instance = new Campaign();

    return d_instance;
}

Campaign* Campaign::getInstance(XML_Helper *helper)
{
    if (!d_instance)
        d_instance = new Campaign();

    d_instance = new Campaign(helper);
    return d_instance;
}

void Campaign::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}


Campaign::Campaign(XML_Helper* helper)
{
  helper->getData(d_next_scenario, "next_scenario");
  helper->getData(d_name, "name");
  helper->getData(d_comment, "comment");
  helper->getData(d_num_heroes, "num_heroes");
}

Campaign::Campaign()
{
  d_next_scenario = "";
  d_name = "";
  d_comment = "";
  d_num_heroes = 0;
}

Campaign::~Campaign()
{
}

bool Campaign::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Campaign::d_tag);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("comment", d_comment);
    retval &= helper->saveData("next_scenario", d_next_scenario);
    retval &= helper->saveData("num_heroes", d_num_heroes);
    retval &= helper->closeTag();

    return retval;
}

std::string Campaign::get_campaign_from_scenario_file(std::string campaign)
{
  CampaignNextScenarioNameLoader loader;
  
  XML_Helper helper(campaign, std::ios::in, Configuration::s_zipfiles);
  helper.registerTag(Campaign::d_tag, 
		     sigc::mem_fun(loader, &CampaignNextScenarioNameLoader::loadNextScenarioName));
  helper.parse();

  return loader.d_next_campaign;
}

