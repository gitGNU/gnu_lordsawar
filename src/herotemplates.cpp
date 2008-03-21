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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "herotemplates.h"

#include "File.h"
#include "defs.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "hero.h"

HeroTemplates* HeroTemplates::d_instance = 0;

HeroTemplates* HeroTemplates::getInstance()
{
    if (!d_instance)
        d_instance = new HeroTemplates();

    return d_instance;
}

void HeroTemplates::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}


HeroTemplates::HeroTemplates()
{
  loadHeroTemplates();
}

HeroTemplates::~HeroTemplates()
{
  for (unsigned int i = 0; i < MAX_PLAYERS; ++i)
    for (std::vector<Hero *>::iterator j = d_herotemplates[i].begin();
           j != d_herotemplates[i].end(); ++j)
      delete *j;
}

Hero *HeroTemplates::getRandomHero(int player_id)
{
  int num = rand() % d_herotemplates[player_id].size();
  return d_herotemplates[player_id][num];
}

int HeroTemplates::loadHeroTemplates()
{
  FILE *fileptr = fopen (File::getMiscFile("heronames").c_str(), "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int retval;
  int gender;
  int side;
  size_t bytesread = 0;
  char *tmp;
  const Armysetlist* al = Armysetlist::getInstance();
  const Army* herotype;

  // list all the army types that are heroes.
  std::vector<const Army*> heroes;
  Player *p = Playerlist::getInstance()->getNeutral();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const Army *a = al->getArmy (p->getArmyset(), j);
      if (a->isHero())
	heroes.push_back(a);
    }

  if (fileptr == NULL)
    return -1;
  while ((read = getline (&line, &len, fileptr)) != -1)
    {
      bytesread = 0;
      retval = sscanf (line, "%d%d%n", &side, &gender, &bytesread);
      if (retval != 2)
	{
	  free (line);
	  return -2;
	}
      while (isspace(line[bytesread]) && line[bytesread] != '\0')
	bytesread++;
      tmp = strchr (&line[bytesread], '\n');
      if (tmp)
	tmp[0] = '\0';
      if (strlen (&line[bytesread]) == 0)
	{
	  free (line);
	  return -3;
	}
      if (side < 0 || side > (int) MAX_PLAYERS)
	{
	  free (line);
	  return -4;
	}

      herotype = heroes[rand() % heroes.size()];
      Hero *newhero = new Hero (*herotype, "", NULL, true);
      if (gender)
	newhero->setGender(Hero::MALE);
      else
	newhero->setGender(Hero::FEMALE);
      newhero->setName (&line[bytesread]);
      d_herotemplates[side].push_back (newhero);
    }
  if (line)
    free (line);
  fclose (fileptr);
  return 0;
}
        
