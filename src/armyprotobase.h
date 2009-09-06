// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#ifndef ARMY_PROTO_BASE_H
#define ARMY_PROTO_BASE_H

#include <gtkmm.h>
#include <string>

#include "defs.h"

class XML_Helper;

#include "armybase.h"

class ArmyProtoBase : public ArmyBase
{
    public:

	//! Copy constructor.
        ArmyProtoBase(const ArmyProtoBase& armyprotobase);

	//! Loading constructor.
        ArmyProtoBase(XML_Helper* helper);
        
	//! Create an empty army prototype base.
	ArmyProtoBase();

	//! Destructor.
        ~ArmyProtoBase();

        // Set functions:
        
        void setTypeId(guint32 type_id) {d_type_id = type_id;};

        //! Sets the descriptive text for this Army.
        void setDescription(std::string text) {d_description = text;};
        
        //! Set the gold pieces needed to add this Army to a city's production.
        void setProductionCost(guint32 production_cost)
	  {d_production_cost = production_cost;}

	//! Sets the armyset id for this army.
	void setArmyset(guint32 id) {d_armyset = id;};

        // Get functions
        
	//! Get the Id of the Armyset to which the Army's type belongs.
        guint32 getTypeId() const {return d_type_id;}

        //! Returns the descriptive text of this Army.
        std::string getDescription() const {return _(d_description.c_str());}

        //! Returns how much gold setting up the production costs
	/**
	 * @return The amount of gold pieces required to add this Army
	 *         into the City's suite of 4 production slots.
	 */
        guint32 getProductionCost() const {return d_production_cost;}

	//! Returns the armyset id for this army.
	guint32 getArmyset() const {return d_armyset;};

        //! Set the army bonus of the army prototype.
        void setArmyBonus(guint32 bonus) {d_army_bonus = bonus;};

        //! Set the move bonus.
        void setMoveBonus(guint32 bonus) {d_move_bonus = bonus;};

        //! Set the movement points of the army.
        void setMaxMoves(guint32 moves) {d_max_moves = moves;};

        //! Set the sight of the army.
        void setSight(guint32 sight) {d_sight = sight;};

        //! Set the name of the Army.
        void setName(std::string name){d_name = name;}

        //! Returns the name of the Army.
        std::string getName() const {return _(d_name.c_str());};

        //! Returns how many turns this Army needs to be produced.
        guint32 getProduction() const {return d_production;};

        //! Set how many turns this unit type needs to be produced.
        void setProduction(guint32 production){d_production = production;};

    protected:
	bool saveData(XML_Helper* helper) const;

	//! The name of the Army unit.  e.g. Scouts.
        std::string d_name;

	//! The index of the Army prototype's index in it's Armyset.
        guint32 d_type_id;

	//! The description of the Army unit.
        std::string d_description;

        //! How many gold pieces needed to add this Army to a city's production.
	/**
	 * If d_production_cost is over zero, then the Army can be purchased.
	 * If not, then the Army unit cannot be incorporated into a 
	 * City's production at any price.
	 *
	 * This value does not change during gameplay.
	 */
        guint32 d_production_cost;

	//! How many turns the Army unit takes to be produced in a City.
	/**
	 * This value must be above 0.  Normal values for d_production are
	 * 1 through 4.
	 * This value does not change during gameplay.
	 */
        guint32 d_production;

	//! The armyset to which this army prototype belongs.
	guint32 d_armyset;
	
};

#endif // ARMY_PROTO_BASE_H
