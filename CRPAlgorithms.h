//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INBAVERSIM_CRPALGORITHMS_H_
#define __INBAVERSIM_CRPALGORITHMS_H_

#include <omnetpp.h>
#include "InternalMessages_m.h"
#include "RFC8609Messages_m.h"
#include "inbaver.h"
#include "RFC8569Forwarder.h"
#include <list>
#include <climits>
#include <cstdlib>
#include <string>
#include <queue>
#include "Demiurge.h"
#include "Numen.h"
using namespace omnetpp;
using namespace std;
class Demiurge;
class Numen;
using namespace omnetpp;


/**
 * TODO - Generated class
 */
class CRPAlgorithms : public cSimpleModule
{
protected:
    virtual void initialize();
    virtual void handleMessage();
    virtual void finish();
private:
    list<CSEntry *> crpCs;
    long currentCSSize;
    int selected_algorithm;
public:

    virtual void setValues(list <CSEntry *> cs, long currentCSSize, int Algorithm);
    virtual list <CSEntry *> getCS();
    virtual long getcurrentCSSize();
    virtual void removeCSEntry(int maximumContentStoreSize,ContentObjMsg *contentObjMsg,long *removed);
    virtual void addCSEntery(int maximumContentStoreSize, ContentObjMsg *contentObjMsg,CSEntry *csentry,long *removed);

};

#endif

