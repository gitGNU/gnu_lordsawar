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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <assert.h>
#include <pgmessagebox.h>
#include <pgwidget.h>
#include <pglabel.h>
#include "w_edit.h"
#include "sound.h"
#include "GraphicsCache.h"
#include "Popup.h"
#include "RuinSearchDialog.h"
#include "CityOccupationDialog.h"
#include "FightDialog.h"
#include "bigmap.h"
#include "smallmap.h"
#include "army.h"
#include "hero.h"
#include "stacklist.h"
#include "cityinfo.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "ruin.h"
#include "temple.h"
#include "GameMap.h"
#include "playerlist.h"
#include "stackinfo.h"
#include "hero_offer.h"
#include "path.h"
#include "Configuration.h"
#include "File.h"
#include "QuestsManager.h"
#include "ArmyLevelDialog.h"
#include "ArmyMedalDialog.h"
#include "events/ERound.h"
#include "events/ENextTurn.h"
#include "events/RUpdate.h"
#include "events/RCenter.h"
#include "events/RCenterObj.h"
#include "events/RRaiseEvent.h"
#include "events/RActEvent.h"
#include "events/RWinGame.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

SigC::Signal1<bool,bool> W_Edit::sigChangeResolution;

W_Edit::W_Edit(GameScenario* gameScenario, PG_Widget* parent, PG_Rect rect)
        :PG_Widget(parent, rect), d_gameScenario(gameScenario), d_lock(false)
{
    myrect=rect;

    // this defines how many squares the bigmap shows! this is important
    // for the smallmap, because the viewrect depends on it
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    int x_squares = (rect.my_width - 219) / ts;
    int y_squares = (rect.my_height - 190) / ts;

    // load button picture
    loadImages();

    //some graphical finetuning: place the smallmap in the middle of the piece
    //above the buttons (in the middle of x = (width - 109) -- (width -7) and
    //y = 20 -- 122
    PG_Rect smallrect;

    int displacement=(rect.my_width-(100 + x_squares*ts + GameMap::getWidth()))/2;
    smallrect.x = 100 + x_squares*ts + displacement;
    smallrect.y = 76 - (GameMap::getHeight()/2);
    smallrect.w = GameMap::getWidth() + 2;  
    smallrect.h = smallrect.w;
    d_smallmap = new SmallMap(this, smallrect, PG_Rect(0,0,x_squares, y_squares));

    // Create the bigmap...
    d_bigmap = new BigMap(this,PG_Rect(40,40, x_squares*ts, y_squares*ts));
    d_bigmap->setViewrect(d_smallmap->getViewrect());
    
    // ...and the borders around. The borders consist of 8 scrolling buttons and
    // 8 connecting pieces. We start with the top left and go clockwise.
    // Regard the placing and the sizes as semi-magical and ignore them unless you want
    // to actually change something here.

    d_b_scroll[0] = new Scroller(this, PG_Rect(0, 0, 40, 40),10,d_smallmap,-1,-1);
    d_b_scroll[1] = new Scroller(this, PG_Rect(20 + x_squares*ts/2, 0, 40, 40),11,d_smallmap,0,-1);
    d_b_scroll[2] = new Scroller(this, PG_Rect(40 + x_squares*ts, 0, 40, 40),12,d_smallmap,1,-1);
    d_b_scroll[3] = new Scroller(this, PG_Rect(40 + x_squares*ts, 20 + y_squares*ts/2, 40, 40),13,d_smallmap,1,0);
    d_b_scroll[4] = new Scroller(this, PG_Rect(40 + x_squares*ts, 40 + y_squares*ts, 40, 40),14,d_smallmap,1,1);
    d_b_scroll[5] = new Scroller(this, PG_Rect(20 + x_squares*ts/2, 40 + y_squares*ts, 40, 40),15,d_smallmap,0,1);
    d_b_scroll[6] = new Scroller(this, PG_Rect(0, 40 + y_squares*ts, 40, 40),16,d_smallmap,-1,1);
    d_b_scroll[7] = new Scroller(this, PG_Rect(0, 20 + y_squares*ts/2, 40, 40),17,d_smallmap,-1,0);

    d_border[0] = new PG_ThemeWidget(this, PG_Rect(40, 5, x_squares*ts/2 - 20, 35), true);
    d_border[1] = new PG_ThemeWidget(this, PG_Rect(60 + x_squares*ts/2, 5, x_squares*ts/2 - 20, 35), true);
    d_border[2] = new PG_ThemeWidget(this, PG_Rect(40 + x_squares*ts, 40, 35, y_squares*ts/2 - 20), true);
    d_border[3] = new PG_ThemeWidget(this, PG_Rect(40 + x_squares*ts, 60 + y_squares*ts/2, 35, y_squares*ts/2 - 20), true);
    d_border[4] = new PG_ThemeWidget(this, PG_Rect(60 + x_squares*ts/2, 40 + y_squares*ts, x_squares*ts/2 - 20, 35), true);
    d_border[5] = new PG_ThemeWidget(this, PG_Rect(40, 40 + y_squares*ts, x_squares*ts/2 - 20, 35), true);
    d_border[6] = new PG_ThemeWidget(this, PG_Rect(5, 60 + y_squares*ts/2, 35, y_squares*ts/2 - 20), true);
    d_border[7] = new PG_ThemeWidget(this, PG_Rect(5, 40, 35, y_squares*ts/2 - 20), true);

    // Load the images
    for (int i = 0; i < 8; i++)
    {
        char buffer[80];
        buffer[79] = '\0';

        snprintf(buffer, 79, "scroll%i", i);
        d_scrollsurf[i] = File::getBorderPic(std::string(buffer));

        snprintf(buffer, 79, "scroll%i_on", i);
        d_scrollsurfon[i] = File::getBorderPic(std::string(buffer));

        if (i % 2 == 0)
        {
            snprintf(buffer, 79, "border%i", i/2);
            d_bordersurf[i/2] = File::getBorderPic(std::string(buffer));
        }
    }

    // And finish the borders
    for (int i = 0; i < 8; i++)
    {
        d_b_scroll[i]->SetIcon(d_scrollsurf[i], 0, d_scrollsurfon[i]);
        d_b_scroll[i]->SetBorderSize(0, 0, 0);
        d_b_scroll[i]->SetShift(0);
        d_b_scroll[i]->SetTransparency(255, 255, 255);
	//d_b_scroll[i]->sigClick.connect(slot(*this, &W_Edit::b_scrollClicked));

        d_border[i]->SetBackground(d_bordersurf[i/2]);
        d_border[i]->SetBackgroundBlend(0);
    }
    
    // the tilepos label
    PG_Rect buttonrect(95 + x_squares*ts, 340, 50, 50);
    d_l_tilepos = new PG_Label(this, buttonrect, "");
    d_l_tilepos->SetAlignment(PG_Label::CENTER);

    // And do the rest of the setup
    d_stackinfo = new Stackinfo(this,PG_Rect(15, rect.my_height - 80, 360, 72));

    d_fllogo = new PG_Label(this, PG_Rect(rect.my_width-220,rect.my_height - 55, 220, 55),""); 
    d_pic_logo = File::getMiscPicture("lordsawar_logo.png"); 
    d_fllogo->SetIcon(d_pic_logo);

    l_turns = new PG_Label(this,PG_Rect(100 + x_squares*ts, 140, 100, 20), "");
    l_gold  = new PG_Label(this,PG_Rect(100 + x_squares*ts, 160, 100, 20), "");

    d_tp = new ToolTip(this, PG_Rect(0,0,0,0));
    d_tp->Hide();

    d_b_prev = new PG_Button(this,PG_Rect(100 + x_squares*ts, 180, 40, 40),"",1);
    d_b_next = new PG_Button(this,PG_Rect(140 + x_squares*ts, 180, 40, 40),"",2);
    d_b_nextwithmove = new PG_Button(this,PG_Rect(180 + x_squares*ts, 180, 40, 40),"",3);
    d_b_move = new PG_Button(this,PG_Rect(100 + x_squares*ts, 220, 40, 40),"",4);
    d_b_moveall = new PG_Button(this,PG_Rect(140 + x_squares*ts, 220, 40, 40),"",5);
    d_b_centerCurrent = new PG_Button(this,PG_Rect(180 + x_squares*ts, 220, 40, 40),"",6);
    d_b_defend = new PG_Button(this,PG_Rect(100 + x_squares*ts, 260, 40, 40),"",7);
    d_b_defendAndNext = new PG_Button(this,PG_Rect(140 + x_squares*ts, 260, 40, 40),"",8);
    d_b_defendAndNextwithmove = new PG_Button(this,PG_Rect(180 + x_squares*ts, 260, 40, 40),"",9);
    d_b_search = new PG_Button(this,PG_Rect(100 + x_squares*ts, 300, 40, 40),"",10);
    d_b_nextTurn = new PG_Button(this,PG_Rect(140 + x_squares*ts, 300, 40, 40),"",11);


    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
    {
        (*it)->sdyingStack.connect(SigC::slot((*this), &W_Edit::stackDied));

        if ((*it)->getType() == Player::HUMAN)
        {
            (*it)->snewLevelArmy.connect(SigC::slot((*this), &W_Edit::newLevelArmy));
            (*it)->snewMedalArmy.connect(SigC::slot((*this), &W_Edit::newMedalArmy));
        }
    }

    // this has to be done for each player!!!
    for (Playerlist::iterator it = Playerlist::getInstance()->begin();
            it != Playerlist::getInstance()->end(); ++it)
    {
        (*it)->schangingStatus.connect(SigC::slot((*this), &W_Edit::updateStatus));
        (*it)->supdatingStack.connect(SigC::slot((*this), &W_Edit::stackUpdate));

        (*it)->sinvadingCity.connect(SigC::slot((*this), &W_Edit::cityOccupied));
        (*it)->sinterruptTimers.connect(SigC::slot((*this), &W_Edit::stopTimers));
        (*it)->scontinueTimers.connect(SigC::slot((*this), &W_Edit::startTimers));

        //We can connect to all players, since only the human players will raise
        //the signal.
        (*it)->srecruitingHero.connect(SigC::slot((*this), &W_Edit::heroJoins));
    }

    d_smallmap->schangingViewrect.connect(SigC::slot(*d_bigmap, &BigMap::Redraw));
    d_bigmap->schangingViewrect.connect(SigC::slot(*d_smallmap, &SmallMap::Redraw));
    
    d_bigmap->sselectingStack.connect(SigC::slot((*this), &W_Edit::bigmapStackSelected));
    d_bigmap->sdeselectingStack.connect(SigC::slot((*this), &W_Edit::unselectStack));
    d_bigmap->smovingMouse.connect(SigC::slot(*this, &W_Edit::movingMouse));

    d_b_move->sigClick.connect(slot(*this, &W_Edit::b_moveClicked));
    d_b_moveall->sigClick.connect(slot(*this, &W_Edit::b_moveAllClicked));
    d_b_prev->sigClick.connect(slot(*this, &W_Edit::b_prevClicked));
    d_b_nextwithmove->sigClick.connect(slot(*this, &W_Edit::b_nextwithmoveClicked));
    d_b_next->sigClick.connect(slot(*this, &W_Edit::b_nextClicked));
    d_b_defend->sigClick.connect(slot(*this, &W_Edit::b_defendClicked));
    d_b_defendAndNext->sigClick.connect(slot(*this, &W_Edit::b_defendAndNextClicked));
    d_b_defendAndNextwithmove->sigClick.connect(slot(*this, &W_Edit::b_defendAndNextWithMoveClicked));
    d_b_search->sigClick.connect(slot(*this, &W_Edit::b_searchClicked));
    d_b_nextTurn->sigClick.connect(slot(*this, &W_Edit::b_nextTurnClicked));
    d_b_centerCurrent->sigClick.connect(slot(*this, &W_Edit::b_centerCurrentClicked));

    //set up a NextTurn object
    d_nextTurn = new NextTurn(d_gameScenario->getTurnmode());
    d_nextTurn->splayerStart.connect(SigC::slot((*this), &W_Edit::checkPlayer));
    d_nextTurn->snextRound.connect(SigC::slot((*d_gameScenario), &GameScenario::nextRound));
    d_nextTurn->supdating.connect(SigC::slot(*d_bigmap, &BigMap::Redraw));
            
    connectEvents();
    
    d_allDefending = false;
    
    placeWidgets(rect);
    checkButtons();
}

W_Edit::~W_Edit()
{
    //smallmap as well as bigmap have timers running, so first stop the timers
    //and then wait some microseconds to make sure we don't get problems with
    //the timers
    stopTimers();
    SDL_Delay(1);

    delete d_smallmap;
    delete d_bigmap;

    delete d_gameScenario;
    delete d_nextTurn;
    
    delete d_stackinfo;
    delete l_turns;
    delete l_gold;
    
    for (int i = 0; i < 2; i++)
    {
        SDL_FreeSurface(d_pic_prev[i]);
        SDL_FreeSurface(d_pic_next[i]);
        SDL_FreeSurface(d_pic_defend[i]);
        SDL_FreeSurface(d_pic_move[i]);
        SDL_FreeSurface(d_pic_moveall[i]);
        SDL_FreeSurface(d_pic_defendAndNext[i]);
        SDL_FreeSurface(d_pic_defendAndNextwithmove[i]);
        SDL_FreeSurface(d_pic_search[i]);
        SDL_FreeSurface(d_pic_nextTurn[i]);
        SDL_FreeSurface(d_pic_centerCurrent[i]);
        SDL_FreeSurface(d_pic_nextwithmove[i]);
    }
    SDL_FreeSurface(d_pic_turn_start);
    SDL_FreeSurface(d_pic_winGame);
    SDL_FreeSurface(d_pic_logo);

    delete d_b_prev;
    delete d_b_next;
    delete d_b_nextwithmove;
    delete d_b_move;
    delete d_b_moveall;
    delete d_b_defend;
    delete d_b_defendAndNext;
    delete d_b_defendAndNextwithmove;
    delete d_b_search;
    delete d_b_nextTurn;
    delete d_b_centerCurrent;
    delete d_l_tilepos;
    delete d_fllogo;
    delete d_tp;

    for (int i = 0; i < 8; i++)
    {
        delete d_b_scroll[i];
        delete d_border[i];
    }

    for (int i = 0; i < 8; i++)
    {
        SDL_FreeSurface(d_scrollsurf[i]);
        SDL_FreeSurface(d_scrollsurfon[i]);
    }
    for (int i = 0; i < 4; i++)
        SDL_FreeSurface(d_bordersurf[i]);
}

void W_Edit::placeWidgets(PG_Rect rect)
{
    // one fourth of the screen is reserved for the smallmap; subtract something
    // for spare space
    int right_part = rect.w/4 - 30;

    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    int x_squares = (rect.my_width - 100 - right_part)/ts;
    int y_squares = (rect.my_height - 160) / ts;
    
    // Now place all the widgets
    // first smallmap (note: center it a bit)
    PG_Rect smallrect;
    smallrect.x = 100 + x_squares*ts;
    smallrect.y = 30;
    smallrect.w = smallrect.h = right_part;
    d_smallmap->MoveWidget(smallrect);
    d_smallmap->changeResolution(PG_Rect(0,0,x_squares,y_squares));

    // then the bigmap
    d_bigmap->MoveWidget(PG_Rect(40, 40, x_squares*ts, y_squares*ts));
    d_bigmap->setViewrect(d_smallmap->getViewrect());
    displayFirstCity();
    
    // ...and the borders around. 

    d_b_scroll[0]->MoveWidget(PG_Rect(0, 0, 40, 40));
    d_b_scroll[1]->MoveWidget(PG_Rect(20 + x_squares*ts/2, 0, 40, 40));
    d_b_scroll[2]->MoveWidget(PG_Rect(40 + x_squares*ts, 0, 40, 40));
    d_b_scroll[3]->MoveWidget(PG_Rect(40 + x_squares*ts, 20 + y_squares*ts/2, 40, 40));
    d_b_scroll[4]->MoveWidget(PG_Rect(40 + x_squares*ts, 40 + y_squares*ts, 40, 40));
    d_b_scroll[5]->MoveWidget(PG_Rect(20 + x_squares*ts/2, 40 + y_squares*ts, 40, 40));
    d_b_scroll[6]->MoveWidget(PG_Rect(0, 40 + y_squares*ts, 40, 40));
    d_b_scroll[7]->MoveWidget(PG_Rect(0, 20 + y_squares*ts/2, 40, 40));

    d_border[0]->MoveWidget(PG_Rect(40, 5, x_squares*ts/2 - 20, 35));
    d_border[1]->MoveWidget(PG_Rect(60 + x_squares*ts/2, 5, x_squares*ts/2 - 20, 35));
    d_border[2]->MoveWidget(PG_Rect(40 + x_squares*ts, 40, 35, y_squares*ts/2 - 20));
    d_border[3]->MoveWidget(PG_Rect(40 + x_squares*ts, 60 + y_squares*ts/2, 35, y_squares*ts/2 - 20));
    d_border[4]->MoveWidget(PG_Rect(60 + x_squares*ts/2, 40 + y_squares*ts, x_squares*ts/2 - 20, 35));
    d_border[5]->MoveWidget(PG_Rect(40, 40 + y_squares*ts, x_squares*ts/2 - 20, 35));
    d_border[6]->MoveWidget(PG_Rect(5, 60 + y_squares*ts/2, 35, y_squares*ts/2 - 20));
    d_border[7]->MoveWidget(PG_Rect(5, 40, 35, y_squares*ts/2 - 20));

    // the tilepos label etc.
    d_l_tilepos->MoveWidget(PG_Rect(95 + x_squares*ts, 250+smallrect.h, 50, 50));
    d_tp->MoveWidget(PG_Rect(0,0,0,0));

    // And do the rest of the setup
    d_stackinfo->MoveWidget(PG_Rect(15,rect.my_height - 90, 480, 86));

    d_fllogo->MoveWidget(PG_Rect(rect.my_width-220,rect.my_height - 55,220,55));

    l_turns->MoveWidget(PG_Rect(100 + x_squares*ts, 50+smallrect.h, 100, 20));
    l_gold->MoveWidget(PG_Rect(100 + x_squares*ts, 70+smallrect.h, 100, 20));

    d_b_prev->MoveWidget(PG_Rect(100 + x_squares*ts, 90+smallrect.h, 40, 40));
    d_b_next->MoveWidget(PG_Rect(140 + x_squares*ts, 90+smallrect.h, 40, 40));
    d_b_nextwithmove->MoveWidget(PG_Rect(180 + x_squares*ts, 90+smallrect.h, 40, 40));
    d_b_move->MoveWidget(PG_Rect(100 + x_squares*ts, 130+smallrect.h, 40, 40));
    d_b_moveall->MoveWidget(PG_Rect(140 + x_squares*ts, 130+smallrect.h, 40, 40));
    d_b_centerCurrent->MoveWidget(PG_Rect(180 + x_squares*ts, 130+smallrect.h, 40, 40));
    d_b_defend->MoveWidget(PG_Rect(100 + x_squares*ts, 170+smallrect.h, 40, 40));
    d_b_defendAndNext->MoveWidget(PG_Rect(140 + x_squares*ts, 170+smallrect.h, 40, 40));
    d_b_defendAndNextwithmove->MoveWidget(PG_Rect(180 + x_squares*ts, 170+smallrect.h, 40, 40));
    d_b_search->MoveWidget(PG_Rect(100 + x_squares*ts, 210+smallrect.h, 40, 40));
    d_b_nextTurn->MoveWidget(PG_Rect(140 + x_squares*ts, 210+smallrect.h, 40, 40));
}

void W_Edit::changeResolution(PG_Rect rect, bool smaller)
{
    // Here we resize the w_edit only if we go to a greater resolution
    myrect=rect;
    if (!smaller) 
    {
       sigChangeResolution.emit(false);
       SizeWidget(rect.my_width,rect.my_height);
    }
    
    placeWidgets(rect);

    // Here we resize the w_edit only if we go to a lesser resolution
    if (smaller) 
    {
       sigChangeResolution.emit(false);

       // fix: paragui leaves an X error (something you encounter really seldom)
       // in this special case. To circumvent this, we must pretend our widget
       // has been increased in size. If I succeed in patching paragui, this will
       // become unneccessary with paragui-1.1.9
       my_width = rect.my_width - 1;
       my_height = rect.my_height - 1;
       
       SizeWidget(rect.my_width,rect.my_height);
    }

    Redraw();			       
}

void W_Edit::connectEvents()
{
    std::list<Event*> elist = d_gameScenario->getEventlist();
    ERound* eround;
    ENextTurn* eturn;

    // first, connect the static signals appropriately
    RUpdate::supdating.connect(SigC::slot(*d_bigmap, &BigMap::Redraw));
    RCenter::scentering.connect(SigC::slot(*d_bigmap, &BigMap::centerView));
    RCenterObj::scentering.connect(SigC::slot(*d_bigmap, &BigMap::centerView));
    RRaiseEvent::sgettingEvents.connect(SigC::slot(*d_gameScenario,
                                        &GameScenario::getEventlist));
    RActEvent::sgettingEvents.connect(SigC::slot(*d_gameScenario,
                                        &GameScenario::getEventlist));
    RWinGame::swinDialog.connect(SigC::slot(*this, &W_Edit::gameFinished));


    // and connect some of the events
    for (std::list<Event*>::iterator it = elist.begin(); it != elist.end(); it++)
    {
        switch((*it)->getType())
        {
            //the round event needs rather much help with its signals
            case Event::ROUND:
                eround = dynamic_cast<ERound*>(*it);
                eround->sgettingRound.connect(SigC::slot(*d_gameScenario,
                                                        &GameScenario::getRound));
                d_nextTurn->snextTurn.connect(SigC::slot(*eround, &ERound::trigger));
                break;

            case Event::NEXTTURN:
                eturn = dynamic_cast<ENextTurn*>(*it);
                d_nextTurn->snextTurn.connect(SigC::slot(*eturn, &ENextTurn::trigger));
                break;
                
            default:
                continue;
        }
    }
}

bool W_Edit::b_nextTurnClicked(PG_Button* btn)
{
    //clean up and hand over control to NextTurn class
    d_bigmap->stackDeselected();
    d_stackinfo->Hide();
    lockScreen();

    d_nextTurn->endTurn();
    checkButtons();
    return true;
}

void W_Edit::nextTurn()
{
    debug("nextTurn()");

    //set up everything for the next human player's turn

    if (!Playerlist::getActiveplayer())
    {
        return;
    }

    unlockScreen();
    
    d_allDefending = false;
    updateStatus();
    
    // human player
    if (Configuration::s_showNextPlayer)
    {
        pictureNextPlayer();
    }
}

// Print new status
void W_Edit::updateStatus()
{
    char buffer[80];
    sprintf(buffer, _("Turns: %i"), d_gameScenario->getRound());
    l_turns->SetText(buffer);

    sprintf(buffer, _("Gold: %i"), Playerlist::getActiveplayer()->getGold());
    l_gold->SetText(buffer);
}

// find first city of activeplayer and view it if outside screen
void W_Edit::displayFirstCity()
{
    debug("displayFirstCity()");

    const Player* p = Playerlist::getInstance()->getActiveplayer();
    // preferred city is a capital city that belongs to the player 
    for (Citylist::iterator it = Citylist::getInstance()->begin();
            it != Citylist::getInstance()->end(); it++)
    {
        if ((*it).getPlayer() == p && (*it).isCapital())
        {
            d_bigmap->centerView((*it).getPos());
            return;
        }
    }

    // okay, then find _any_ city that belongs to the player and center on it.
    for (Citylist::iterator it = Citylist::getInstance()->begin();
        it != Citylist::getInstance()->end(); it++)
    {
        if ((*it).getPlayer() == p)
        {
            d_bigmap->centerView((*it).getPos());
            break;
        }
    }
}

// pop up message window for next player
void W_Edit::pictureNextPlayer()
{
    debug("pictureNextPlayer()");

    // create and run the next player popup
    char buf[101]; buf[100] = '\0';
    snprintf(buf, 100, _("Next Player: %s"),
             Playerlist::getInstance()->getActiveplayer()->getName().c_str());
    
    Popup* nextPlayer = new Popup(this, PG_Rect((my_width-300)/2,(my_height-200)/2, 300, 200));
    new PG_Label(nextPlayer, PG_Rect(10, 10, 300, 20), buf);
    
    nextPlayer->SetIcon(d_pic_turn_start);
    nextPlayer->Show();
    nextPlayer->RunModal();
    
    delete nextPlayer;
}

bool W_Edit::heroJoins(Hero* hero, int gold)
{
    debug("heroJoins()");

    char buffer[101];
    buffer[100]='\0';
    snprintf(buffer, 100, _("Hero offer for %s"),
            Playerlist::getActiveplayer()->getName().c_str());

    Sound::getInstance()->playMusic("hero", 1);
    Hero_offer hero_offer(this, hero, gold);
    hero_offer.SetTitle(buffer);
    hero_offer.Show();
    hero_offer.RunModal();
    hero_offer.Hide();
    Sound::getInstance()->haltMusic();

    return hero_offer.getRetval();
}

bool W_Edit::b_prevClicked(PG_Button* btn)
{
    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setPrev();
    debug("Setting active stack = " << stack)
    if (stack)
        d_bigmap->stackSelected();
    else
        d_allDefending = true;

    checkButtons();
    return true;
}

bool W_Edit::b_nextClicked(PG_Button* btn)
{
    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setNext();
    debug("Setting active stack = " << stack)
    if (stack)
        d_bigmap->stackSelected();
    else
        d_allDefending = true;

    checkButtons();
    return true;
}

bool W_Edit::b_nextwithmoveClicked(PG_Button* btn)
{
    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setNextWithMove();
    debug("Setting active stack = " << stack)
    if (stack) d_bigmap->stackSelected();
    else d_allDefending = true;

    checkButtons();
    return true;
}

bool W_Edit::b_moveClicked(PG_Button* btn)
{
    Playerlist::getActiveplayer()->stackMove(Playerlist::getActiveplayer()->getActivestack());
    checkButtons();
    return true;
}

bool W_Edit::b_moveAllClicked(PG_Button* btn)
{
    vector<unsigned int> movedid;
    vector<unsigned int>::iterator it;
    Stacklist* sl = Playerlist::getActiveplayer()->getStacklist();
    Stack* orig = sl->getActivestack();
    Stack* stack = sl->setNextWithMove();

    while ((stack) && (find(movedid.begin(),movedid.end(),stack->getId())==movedid.end())) 
    {
        debug(" newid=-->"<< stack->getId())

        if(stack->getPath()->size() >= 0 ) 
        {
            debug("Path>=0 and movedid-->"<< stack->getId())
            movedid.push_back(stack->getId());
            d_bigmap->stackSelected();
            Playerlist::getActiveplayer()->stackMove(Playerlist::getActiveplayer()->getActivestack());
        }	

        stack = Playerlist::getActiveplayer()->getStacklist()->setNextWithMove();
    }

    // if the stack still exists, set the active stack to the old one, else to 0
    if (find(sl->begin(), sl->end(), orig) != sl->end())
    {
        sl->setActivestack(orig);
        stackUpdate(orig);
    }
    else
    {
        sl->setActivestack(0);
        Redraw();
    }

    checkButtons();
    return true;
}

bool W_Edit::b_defendClicked(PG_Button* btn)
{
    Playerlist::getActiveplayer()->getActivestack()->setDefending(true);
    d_bigmap->stackDeselected();
    d_stackinfo->Hide();

    // Now check if there are any non-defending armies of the active player.
    d_allDefending = true;
    Stacklist* sl = Playerlist::getActiveplayer()->getStacklist();
    for (Stacklist::iterator it = sl->begin(); it != sl->end(); it++)
        if (!(*it)->getDefending())
        {
            d_allDefending = false;
            break;
        }
    
    checkButtons();

    return true;
}

bool W_Edit::b_defendAndNextClicked(PG_Button* btn)
{
    Playerlist::getActiveplayer()->getActivestack()->setDefending(true);
    d_bigmap->stackDeselected();
    d_stackinfo->Hide();

    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setNext();
    debug("Setting active stack = " << stack)
    if (stack)
        d_bigmap->stackSelected();
    else
        d_allDefending = true;

    checkButtons();
    return true;
}

bool W_Edit::b_defendAndNextWithMoveClicked(PG_Button* btn)
{
    Playerlist::getActiveplayer()->getActivestack()->setDefending(true);
    d_bigmap->stackDeselected();
    d_stackinfo->Hide();

    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setNextWithMove();
    debug("Setting active stack = " << stack)
    if (stack)
        d_bigmap->stackSelected();
    else
        d_allDefending = true;

    checkButtons();
    return true;

}

bool W_Edit::b_centerCurrentClicked(PG_Button* btn)
{
    Stack *stack = Playerlist::getActiveplayer()->getActivestack();
    if (stack) 
        d_bigmap->stackSelected();
    return true;
}

bool W_Edit::b_searchClicked(PG_Button* btn)
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    Player* p = Playerlist::getInstance()->getActiveplayer();
    Ruin* ruin = Ruinlist::getInstance()->getObjectAt(stack->getPos());
    Temple* temple = Templelist::getInstance()->getObjectAt(stack->getPos());

    debug("search: ruin = " << ruin << ", temple = " << temple)

    if (ruin && ruin->isSearched() == false)
    {
        char buf[101]; buf[100]='\0';
        int cur_gold = p->getGold();
        unsigned int w1 = 240;
        unsigned int h1 = 110;

        snprintf(buf,sizeof(buf), _("%s encounters a %s..."),
             stack->getFirstHero()->getName().c_str(), 
             ruin->getOccupant()->getStrongestArmy()->getName().c_str());
        PG_MessageBox mb(this, 
                         PG_Rect((my_width-w1)/2, (my_height-h1)/2, w1, h1), 
                         _("Searching..."), buf,
                         PG_Rect(80, 70, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        if (!(p->stackSearchRuin(stack, ruin)))
        {
            stackRedraw();
            return true;
        }

        stackRedraw();
        //this also includes the gold-only hack
        int gold_added = p->getGold() - cur_gold;

        RuinSearchDialog dialog(this, gold_added);
        dialog.SetTitle(ruin->getName().c_str());
        dialog.Show();
        dialog.RunModal();
        dialog.Hide();

        updateStatus();
    }
    
    else if ((temple) && (temple->search()))
    {
        unsigned int w1 = 240;
        unsigned int h1 = 110;
        PG_MessageBox mb(this, PG_Rect((my_width-w1)/2, 
                                       (my_height-h1)/2, w1, h1),
                temple->getName().c_str(), _("Your armies have been blessed."),
                PG_Rect(80, 70, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        p->stackVisitTemple(stack, temple);

        PG_MessageBox mb2(this, PG_Rect((my_width-w1)/2, 
                                       (my_height-h1)/2, w1, h1),
                temple->getName().c_str(), _("Do you seek a quest?"),
                PG_Rect(10, h1-40, 100, 30), _("Yes"),
                PG_Rect(120, h1-40, 100, 30), _("No"),
                PG_Label::CENTER
                );
        mb2.Show();
        int decision = mb2.RunModal();
        debug("decision = " << decision);
        mb2.Hide();

        if (decision == 1)
        {
            Quest *q = p->stackGetQuest(stack, temple);
            Hero* hero = dynamic_cast<Hero*>(stack->getFirstHero());

            if (q)
                for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
                    if ((*it)->getId() == q->getHeroId())
                        hero = dynamic_cast<Hero*>(*it);
            
            std::string title = _("Quest - ") + hero->getName();
            std::string txt = q ? q->getDescription()
                                : _("This hero already has a quest");
            PG_MessageBox mb(this, PG_Rect((my_width-400)/2, (my_height-160)/2, 
                             400, 160), title.c_str(), txt.c_str(),
            PG_Rect((400 - 80)/2, 160 - 30 - 10, 80, 30), _("OK"));
            mb.Show();
            mb.RunModal();
            mb.Hide();
        }
    }

    return true;
}

// the parameter is currently not used (=0), but may be used for more detailed
// descriptions later on
void W_Edit::stackUpdate(Stack* s)
{
    debug("W_Edit: stackUpdate()");

    d_bigmap->stackMoved(s);

    d_stackinfo->readData();

    checkButtons();
}

// s is currently unused, but can later be filled with reasonable data
void W_Edit::stackDied(Stack* s)
{
    debug("stackDied()");
    d_bigmap->stackDeselected();
    d_smallmap->Redraw();
    d_stackinfo->Hide();
    checkButtons();
}


Army::Stat W_Edit::newLevelArmy(Army* a)
{
    debug("NEWLEVEL Dialog SHOW")

    // Don't show this dialog if computer or enemy's armies advance
    if ((a->getPlayer()->getType() != Player::HUMAN) ||
        (a->getPlayer() != Playerlist::getInstance()->getActiveplayer()))
        return Army::STRENGTH;

    ArmyLevelDialog dialog(a, 0, PG_Rect(200, 100, 430, 260));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

    // Here Which functions should i add to refresh?? (Andrea)
    // Do we need them (as checkButtons() or others)?
    // UL: only need to redraw the stackinfo
    // // and the bigmap
    d_stackinfo->Redraw();
    d_bigmap->Redraw();

    return dialog.getResult();
}

void W_Edit::newMedalArmy(Army* a)
{
    // We don't want to have medal awards of computer players displayed
    if (!a->getPlayer() || (a->getPlayer()->getType() != Player::HUMAN) ||
        (a->getPlayer() != Playerlist::getInstance()->getActiveplayer()))
        return;
    
    debug("NEWMedal Dialog SHOW")
    std::cerr << "NEWMedal Dialog SHOW" << std::endl;
    ArmyMedalDialog dialog(a, 0, PG_Rect(200, 100, 230, 230));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();
 
    // Here Which functions should i add to refresh?? (Andrea)
    // Do we need them (as checkButtons() or others)?
    // UL: only need to redraw the stackinfo
    d_stackinfo->Redraw();
}

void W_Edit::gameFinished()
{
    // timers would interfere with the dialog, so stop them
    stopTimers();
    
    // show a nice dialog in the center of the screen
    PG_Rect r;
    r.w = d_pic_winGame->w;
    r.h = d_pic_winGame->h;
    r.x = my_width/2 - r.w/2;
    r.y = my_height/2 - r.h/2;
    
    Popup* win = new Popup(this, r);
    SDL_Surface* winpic = GraphicsCache::getInstance()->applyMask(
                                        d_pic_winGame, d_pic_winGameMask,
                                        Playerlist::getActiveplayer());
    win->SetIcon(winpic);
    
    win->Show();
    win->RunModal();

    delete win;
    SDL_FreeSurface(winpic);

    // We don't restart the timers, the game should be over now.
}

bool W_Edit::stackRedraw()
{
    d_bigmap->Redraw();
    d_stackinfo->readData();
    checkButtons();
    return true;
}

void W_Edit::bigmapStackSelected(Stack* s)
{
    d_stackinfo->Show();
    d_stackinfo->readData();
    checkButtons();
}

void W_Edit::cityOccupied(City* city)
{
    debug("cityOccupied()");

    // if a computer makes it's turn and occupied a city, we shouldn't
    // show a modal dialog :)

    if (!d_lock)
    {
        d_bigmap->Redraw();
        CityOccupationDialog dialog(city);
        dialog.Show();
        dialog.RunModal();
        dialog.Hide();
	if (city->isBurnt() == false)
	// if occupied, pillaged or sacked, show cityinfo
	{
            CityInfo cityinfo(city);
            cityinfo.Show();
            cityinfo.RunModal();
            cityinfo.Hide();

            //some visible city properties (defense level) may have changed
            Redraw();
	}
    }
   
    Playerlist::getInstance()->checkPlayers();
    d_bigmap->Redraw();
    d_stackinfo->readData();
    checkButtons();
}

void W_Edit::lockScreen()
{
    debug("lockScreen()");

    //don't accept modifying user input from now on
    d_bigmap->setEnable(false);
    d_b_nextTurn->EnableReceiver(false);
    d_b_prev->EnableReceiver(false);
    d_b_next->EnableReceiver(false);
    d_b_nextwithmove->EnableReceiver(false);
    d_b_defend->EnableReceiver(false);
    d_b_move->EnableReceiver(false);
    d_b_moveall->EnableReceiver(false);
    d_b_search->EnableReceiver(false);
    d_b_defendAndNext->EnableReceiver(false);
    d_b_centerCurrent->EnableReceiver(false);
    d_lock = true;
    stopTimers();   //on computer turns, strange things may happen if the timers
                    //are still active
}

//the inverse of lock_screen()

void W_Edit::unlockScreen()
{
    debug("unlockScreen()");
    d_b_nextTurn->EnableReceiver(true);
    d_bigmap->setEnable(true);
    d_lock = false;
    checkButtons();
    startTimers();
}

void W_Edit::stopTimers()
{
    d_bigmap->interruptTimer();
    d_smallmap->interruptTimer();
}

void W_Edit::startTimers()
{
    //never start timers with the computer player's turn
    if (d_lock)
        return;

    d_bigmap->restartTimer();
    d_smallmap->restartTimer();
}

void W_Edit::checkButtons()
{
    if (d_lock)
        return;
    
    debug("checkButtons()");

    // first assume that all buttons are disabled
    bool prev = false;
    bool next = false;
    bool nextwmove = false;
    bool move = false;
    bool moveall = false;
    bool center = false;
    bool defend = false;
    bool defendAndNext = false;
    bool defendAndNextwithmove = false;
    bool search = false;

    // now check which buttons should be enabled
    if (!d_allDefending) 
    {
        next = true;
        prev = true;
        nextwmove = true;
    }


    // if any stack can move, enable the moveall button
    if (Playerlist::getActiveplayer()->getStacklist()->enoughMoves())
        moveall = true;

    // We search if there is an active stack
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();
    if (stack)
    {
        defend = true;
        center = true;

        if (stack->getPath()->size() > 0 && stack->enoughMoves())
        {
            move = true;
        }
        //TBD what if there is only 1 army and it isn't defending?
        // should be something like (!all defend) && (further_stacks)
        // Answer: Then the army defends and no stack is selected. This
        // behaviour sounds OK to me...
        if (!d_allDefending)
        {
            defendAndNext = true;
            defendAndNextwithmove = true;
        }

        if (stack->hasHero())
        {
            Ruin* ruin = Ruinlist::getInstance()->getObjectAt(stack->getPos());
            Temple* temple = Templelist::getInstance()->getObjectAt(stack->getPos());
            if ((ruin && !ruin->isSearched()) || temple)
            {
                search = true;
            }
        }
    }
    else 
    {
        // We search if there is a moveable stack with a path even if no active stack is selected
        stack=0;
        if (Playerlist::getActiveplayer()->getType()==0) // we check if and only if the player is a human
        {  
            stack = Playerlist::getActiveplayer()->getStacklist()->setNextWithMove(false);
        }
    }
    

    // now set the button states according to the calculations
    d_b_move->EnableReceiver(move);
    d_b_moveall->EnableReceiver(moveall);
    d_b_defend->EnableReceiver(defend);
    d_b_defendAndNext->EnableReceiver(defendAndNext);
    d_b_defendAndNextwithmove->EnableReceiver(defendAndNextwithmove);
    d_b_search->EnableReceiver(search);
    d_b_prev->EnableReceiver(prev);
    d_b_next->EnableReceiver(next);
    d_b_nextwithmove->EnableReceiver(nextwmove);
    d_b_centerCurrent->EnableReceiver(center);

    debug("Prev=" << prev)
    debug("next=" << next)
    debug("nextwmove=" << nextwmove)
    debug("move=" << move)
    debug("moveall=" << moveall)
    debug("defend=" << defend)
    debug("defendandnext=" << defendAndNext)
    debug("search=" << search)
    debug("center=" << center)

    int i;
    
    prev ? i = 0 : i = 1;
    d_b_prev->SetIcon(d_pic_prev[i], 0, 0);
    next? i = 0: i = 1;
    d_b_next->SetIcon(d_pic_next[i], 0, 0);
    nextwmove? i = 0 : i = 1;
    d_b_nextwithmove->SetIcon(d_pic_nextwithmove[i], 0, 0);
    move? i = 0: i = 1;
    d_b_move->SetIcon(d_pic_move[i], 0, 0);
    moveall? i = 0: i = 1;
    d_b_moveall->SetIcon(d_pic_moveall[i], 0, 0);
    center? i = 0: i = 1;
    d_b_centerCurrent->SetIcon(d_pic_centerCurrent[i], 0, 0);
    defend? i = 0: i = 1;
    d_b_defend->SetIcon(d_pic_defend[i], 0, 0);
    defendAndNext? i = 0: i = 1;
    d_b_defendAndNext->SetIcon(d_pic_defendAndNext[i], 0, 0);
    defendAndNextwithmove? i = 0: i = 1;
    d_b_defendAndNextwithmove->SetIcon(d_pic_defendAndNextwithmove[i], 0, 0);
    search? i = 0: i = 1;
    d_b_search->SetIcon(d_pic_search[i], 0, 0);
    d_b_nextTurn->SetIcon(d_pic_nextTurn[0], 0, 0);
    
    // redraw the buttons (SetIcon doesn't do it)
    // don't redraw everything, or bigmap is drawn again
    d_b_prev->Redraw();
    d_b_next->Redraw();
    d_b_nextwithmove->Redraw();
    d_b_move->Redraw();
    d_b_moveall->Redraw();
    d_b_centerCurrent->Redraw();
    d_b_defend->Redraw();
    d_b_defendAndNext->Redraw();
    d_b_defendAndNextwithmove->Redraw();
    d_b_search->Redraw();
    d_b_nextTurn->Redraw();
}

void W_Edit::startGame()
{
    debug ("start_game()");
    lockScreen();

    d_nextTurn->start();
    checkButtons();
}

void W_Edit::loadGame()
{
    if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    {
	char buf[101];
	buf[100] = '\0';
        //human players want access to the controls and an info box
        unlockScreen();
        d_bigmap->stackDeselected();
        d_stackinfo->Hide();
        displayFirstCity();
        updateStatus();

	snprintf(buf, 100, _("%s, your turn continues."), 
		Playerlist::getInstance()->getActiveplayer()->getName().c_str());
        PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-87, 200, 150), _("Load Game"),
                buf, PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }       
    else
    {
        lockScreen();
        updateStatus();
        Playerlist::getActiveplayer()->startTurn();
        d_nextTurn->endTurn();
        checkButtons();
    }
}

void W_Edit::stopGame()
{
    stopTimers();
    d_nextTurn->stop();
    d_gameScenario->deactivateEvents();
    Playerlist::finish();
}

bool W_Edit::checkPlayer(Player* p)
{
    //This function checks at the beginning of a new player's turn if
    //we want to interrupt the NextTurn cycle and give control back to
    //the graphics interface.
    //TBD: Currently this function only checks for a human player. You
    //can also have it check for e.g. escape key pressed to interrupt
    //an AI-only game to save/quit.

    //in any case, center on the active player's city and wait a moment
    displayFirstCity();
    SDL_Delay(250);
    
    if (p->getType() == Player::HUMAN)
    {
        //we have a human player, set up the screen and the player and return
        //false
        nextTurn();
        p->startTurn();

        // since d_nextTurn cannot do this, we must raise this signal manually
        d_nextTurn->snextTurn.emit(Playerlist::getActiveplayer());
        
        return false;
    }

    return true;
}

// This function is called by MainWindow when the user presses a cursor key.
void W_Edit::helpSmallmap(int arrowx ,int arrowy)
{
	d_smallmap->inputFunction(arrowx, arrowy);
}

// This function makes all armies part of the stack.
void W_Edit::selectAllStack()
{
    Stack *s = Playerlist::getActiveplayer()->getStacklist()->getActivestack();
    if (s)
    {
    	s->selectAll();
    	d_stackinfo->readData();
    }

    return;
}


void W_Edit::unselectStack()
{
    Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
    d_stackinfo->Hide();
    d_bigmap->Redraw();
    checkButtons();

    return;
}

bool W_Edit::eventMouseMotion(const SDL_MouseMotionEvent *event)
{
    if (event->state == SDL_PRESSED)
        return true;

#ifndef FL_NO_TIMERS
    // some shortcuts
    PG_Point mousepos;
    mousepos.x = event->x;
    mousepos.y = event->y;

    PG_Rect toolrect;
    toolrect.x = event->x;
    toolrect.y = event->y;
    toolrect.w = myrect.my_width;
    toolrect.h = myrect.my_height;

    if (d_b_prev->IsInside(mousepos))
        d_tp->setData(_("Select the previous stack"), toolrect);
    else if (d_b_next->IsInside(mousepos))
        d_tp->setData(_("Select the next stack"), toolrect);
    else if (d_b_nextwithmove->IsInside(mousepos))
        d_tp->setData(_("Select the next stack that can move"), toolrect);
    else if (d_b_move->IsInside(mousepos))
        d_tp->setData(_("Let the selected stack move to destination"), toolrect);
    else if (d_b_moveall->IsInside(mousepos))
        d_tp->setData(_("Let all stacks move to their destinations"), toolrect);
    else if (d_b_centerCurrent->IsInside(mousepos))
        d_tp->setData(_("Center window on selected stack"), toolrect);
    else if (d_b_defend->IsInside(mousepos))
        d_tp->setData(_("Let the stack defend"), toolrect);
    else if (d_b_defendAndNext->IsInside(mousepos))
        d_tp->setData(_("Let the stack defend and select the next stack"), toolrect);
    else if (d_b_defendAndNextwithmove->IsInside(mousepos))
        d_tp->setData(_("Let the stack defend and select the next stack that can move"), toolrect);
    else if (d_b_search->IsInside(mousepos))
        d_tp->setData(_("Search a ruin or a temple"), toolrect);
    else if (d_b_nextTurn->IsInside(mousepos))
        d_tp->setData(_("Next turn"), toolrect);
#endif
    return true;
}

void W_Edit::movingMouse(PG_Point pos)
{
    if (pos.x < 0 || pos.y < 0)
    {
        d_l_tilepos->SetText("");
        return;
    }
    
    char buffer[31]; buffer[30]='\0';
    PG_Rect *smallmapr = d_smallmap->getViewrect();
    snprintf(buffer, 30, "(%i,%i)", pos.x+smallmapr->my_xpos,pos.y+smallmapr->my_ypos);
    d_l_tilepos->SetText(buffer);

}

void W_Edit::loadImages()
{
    SDL_Surface* buttons = File::getMiscPicture("buttons.png");
    SDL_Surface* buttons2 = File::getMiscPicture("buttons_inactive.png");
    SDL_SetAlpha(buttons, 0, 0);
    SDL_SetAlpha(buttons2, 0, 0);

    SDL_PixelFormat* fmt = buttons->format;

    for (int i = 0; i < 2; i++)
    {
        d_pic_prev[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_next[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_nextwithmove[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_move[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_moveall[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_centerCurrent[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_defend[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_defendAndNext[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_defendAndNextwithmove[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_search[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
        d_pic_nextTurn[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 30, 30, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    }

    SDL_Rect r;
    r.x = r.y = 0;
    r.w = r.h = 30;
    SDL_BlitSurface(buttons, &r, d_pic_prev[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_prev[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_next[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_next[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_nextwithmove[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_nextwithmove[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_move[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_move[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_moveall[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_moveall[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_centerCurrent[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_centerCurrent[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_defend[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_defend[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_defendAndNext[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_defendAndNext[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_defendAndNextwithmove[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_defendAndNextwithmove[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_search[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_search[1], 0);

    r.x += r.w;
    SDL_BlitSurface(buttons, &r, d_pic_nextTurn[0], 0);
    SDL_BlitSurface(buttons2, &r, d_pic_nextTurn[1], 0);
    
    // load other pictures
    d_pic_turn_start = File::getMiscPicture("ship.jpg", false);
    d_pic_winGame = File::getMiscPicture("win.jpg", false);

    // mask pics need a special format
    SDL_Surface* tmp = File::getMiscPicture("win_mask.png");
    d_pic_winGameMask = SDL_CreateRGBSurface(SDL_SWSURFACE, tmp->w, tmp->h,
                                        32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
    SDL_SetAlpha(tmp, 0, 0);
    SDL_BlitSurface(tmp, 0, d_pic_winGameMask, 0);
    SDL_FreeSurface(tmp);
}

// End of file
