#include "counter.h"

FL_Counter* fl_counter;

FL_Counter::FL_Counter(Uint32 start)
    :d_curID(start)
{
}

FL_Counter::FL_Counter(XML_Helper* helper)
{
    helper->getData(d_curID, "curID");
}

FL_Counter::~FL_Counter()
{
}

Uint32 FL_Counter::getNextId()
{
    d_curID++;
    return d_curID;
}

bool FL_Counter::save(XML_Helper* helper)
{
    bool retval =true;

    retval &= helper->openTag("counter");
    retval &= helper->saveData("curID", d_curID);
    retval &= helper->closeTag();

    return retval;
}

