

#include "CRPAlgorithms.h"
#include "InternalMessages_m.h"
#include "RFC8609Messages_m.h"
#include "inbaver.h"
#include "RFC8569Forwarder.h"

Define_Module(CRPAlgorithms);

void CRPAlgorithms::initialize(){}
void CRPAlgorithms::handleMessage(){}
void CRPAlgorithms::finish(){}
list <CSEntry *> CRPAlgorithms::getCS(){
    return crpCs;
}
long CRPAlgorithms::getcurrentCSSize(){
    return currentCSSize;
}
void CRPAlgorithms::setValues(list <CSEntry *> cs, long currentCSSize, int Algorithm)
{
    crpCs = cs;
    currentCSSize = currentCSSize;
    selected_algorithm = Algorithm;
}

void CRPAlgorithms::removeCSEntry(int maximumContentStoreSize,ContentObjMsg *contentObjMsg,long *removed)
{

    // before adding content to CS, check if size will exceed the limit
    // when so, remove cache entries until the new content can be added
        if (maximumContentStoreSize > 0) {
            long removedBytes = 0;
            while ((currentCSSize + contentObjMsg->getPayloadSize()) > maximumContentStoreSize) {
                CSEntry *removingCSEntry = crpCs.front();
                crpCs.pop_front();
                currentCSSize -= removingCSEntry->payloadSize;
                removedBytes += removingCSEntry->payloadSize;

                EV_INFO << simTime() << "Cache is full, cannot insert, removing: "
                        << " " << removingCSEntry->prefixName
                        << " " << removingCSEntry->dataName
                        << " " << removingCSEntry->versionName
                        << " " << removingCSEntry->segmentNum
                        << " " << removingCSEntry->payloadSize
                        << " " << currentCSSize
                        << endl;
                delete removingCSEntry;
             }
            *removed = removedBytes;
        }
        //if (removedBytes > 0) {
          //  emit(cacheRemovalsBytesSignal, removedBytes);
           // emit(cacheSizeBytesSignal, currentCSSize);
             //   }
}
void CRPAlgorithms::addCSEntery(int maximumContentStoreSize, ContentObjMsg *contentObjMsg,CSEntry *csentry,long *removed){
    long removedbytes = 0;
    removeCSEntry(maximumContentStoreSize, contentObjMsg,&removedbytes);
    csentry = new CSEntry;
    csentry->prefixName = contentObjMsg->getPrefixName();
    csentry->dataName = contentObjMsg->getDataName();
    csentry->versionName = contentObjMsg->getVersionName();
    csentry->segmentNum = contentObjMsg->getSegmentNum();
    csentry->cachetime = contentObjMsg->getCachetime();
    csentry->expirytime = contentObjMsg->getExpirytime();
    csentry->totalNumSegments = contentObjMsg->getTotalNumSegments();
    csentry->payloadAsString = contentObjMsg->getPayloadAsString();
    csentry->payloadSize = contentObjMsg->getPayloadSize();
    crpCs.push_back(csentry);
    currentCSSize += contentObjMsg->getPayloadSize();
    EV_INFO << simTime() << "Added Cache entry: "
                         << " " << csentry->prefixName
                         << " " << csentry->dataName
                         << " " << csentry->versionName
                         << " " << csentry->segmentNum
                         << " " << csentry->payloadSize
                         << " " << currentCSSize
                         << endl;
    *removed = removedbytes;

}


