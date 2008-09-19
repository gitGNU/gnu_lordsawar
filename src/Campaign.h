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

#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include <string>
#include <sigc++/trackable.h>

#include "xmlhelper.h"

//! Campign information associated with a scenario.
class Campaign : public sigc::trackable
{
    public:

        //! Returns the singleton instance.
	static Campaign* getInstance();

	//! Reads in the itemlist from a file
        static Campaign* getInstance(XML_Helper *helper);

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();
        
	//! Save the item data.  See XML_Helper for details.
	bool save(XML_Helper* helper) const;

	std::string getNextScenario() const {return d_next_scenario;};
	std::string getName() const {return d_name;};
	std::string getComment() const {return d_comment;};
	int getNumberOfHeroesToCarryOver() const {return d_num_heroes;};

	static std::string get_campaign_from_scenario_file(std::string campaign);
    protected:
	//! Default constructor.
	Campaign();
	//! Loading constructor.
        Campaign(XML_Helper* helper);
	//! Destructor.
        ~Campaign();

  class CampaignNextScenarioNameLoader 
    {
  public:
      bool loadNextScenarioName(std::string tag, XML_Helper* helper)
	{
	  if (tag == "campaign")
	    {
	      helper->getData(d_next_campaign, "next_campaign");
	      return true;
	    }
	  return false;
	}

      std::string d_next_campaign;
    };
    private:

        static Campaign* d_instance;
	std::string d_next_scenario;
	std::string d_name;
	std::string d_comment;
	int d_num_heroes;
};

#endif //CAMPAIGN_H
