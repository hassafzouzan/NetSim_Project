

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
void CRPAlgorithms::crphandle(ContentObjMsg *contentObjMsg, CSEntry *csentry, int maximumContentStoreSize,long *removed,std::map<CSEntry*, int> &accessCounts) {
    long removedBytes = 0;

    // Choose the content replacement algorithm based on the selected_algorithm parameter.
    if (selected_algorithm == 0) { // FIFO
        EV_INFO << "FIFO Algorithm\n";
        FIFOAlgorithm(maximumContentStoreSize, contentObjMsg, &removedBytes);
        addCSEntery(maximumContentStoreSize, contentObjMsg, csentry);

    } else if (selected_algorithm == 1) { // LFU
        EV_INFO << "LFU Algorithm\n";
        LFUAlgorithm(maximumContentStoreSize, contentObjMsg, &removedBytes, accessCounts);
        addCSEntery(maximumContentStoreSize, contentObjMsg, csentry);
    }else if (selected_algorithm == 2) { // LRU
        EV_INFO << "LRU Algorithm\n";
        LRUAlgorithm(maximumContentStoreSize, contentObjMsg, &removedBytes);
        addCSEntery(maximumContentStoreSize, contentObjMsg, csentry);
    } else if (selected_algorithm == 3) { // TTL
        EV_INFO << "TTL Algorithm\n";
        TTLAlgorithm(maximumContentStoreSize, contentObjMsg, &removedBytes);
        addCSEntery(maximumContentStoreSize, contentObjMsg, csentry);
    }

    *removed = removedBytes;

    return;
}
void CRPAlgorithms::addCSEntery(int maximumContentStoreSize, ContentObjMsg *contentObjMsg,CSEntry *csentry){

    //csentry = new CSEntry;
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

}

void CRPAlgorithms::FIFOAlgorithm(int maximumContentStoreSize,ContentObjMsg *contentObjMsg,long *removed)
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
void CRPAlgorithms::LFUAlgorithm(int maximumContentStoreSize, ContentObjMsg *contentObjMsg, long *removed, std::map<CSEntry*, int> &accessCounts )
{
    //std::map<CSEntry*, int> accessCounts;

    // Update access counts for existing entries
    //for (CSEntry* entry : crpCs) {
      //  accessCounts[entry]++;
   // }

    // Increment access count for accessed entry
    CSEntry* accessedEntry = nullptr;
    for (CSEntry* entry : crpCs) {
        if (entry->prefixName == contentObjMsg->getPrefixName() &&
            entry->dataName == contentObjMsg->getDataName() &&
            entry->versionName == contentObjMsg->getVersionName() &&
            entry->segmentNum == contentObjMsg->getSegmentNum()) {
            accessedEntry = entry;
            accessCounts[accessedEntry]++;
            break;
        }
    }
    // If the accessed entry exists, increment its access count
    if (accessedEntry != nullptr) {
        accessCounts[accessedEntry]++;
    }
    // Check if the cache is already full
    if (maximumContentStoreSize > 0) {
        while ((currentCSSize + contentObjMsg->getPayloadSize()) > maximumContentStoreSize) {
            CSEntry* leastFrequentEntry = nullptr;
            int minAccessCount = INT_MAX;


    // Find the entry with the lowest access count
            for (auto entry : crpCs) {
                int accessCount = accessCounts[entry];
                if (accessCount < minAccessCount) {
                    leastFrequentEntry = entry;
                    minAccessCount = accessCount;
                }
            }

    // Evict the least frequent entry to make space
    if (leastFrequentEntry != nullptr) {
        crpCs.remove(leastFrequentEntry);
        currentCSSize -= leastFrequentEntry->payloadSize;
        *removed += leastFrequentEntry->payloadSize;
        accessCounts.erase(leastFrequentEntry);

        EV_INFO << simTime() << "Evicting least frequent entry: "
                << leastFrequentEntry->prefixName << " "
                << leastFrequentEntry->dataName << " "
                << leastFrequentEntry->versionName << " "
                << leastFrequentEntry->segmentNum << " "
                << leastFrequentEntry->payloadSize << " "
                << currentCSSize << endl;

        delete leastFrequentEntry;
    }
}
}
}


void CRPAlgorithms::LRUAlgorithm(int maximumContentStoreSize, ContentObjMsg *contentObjMsg, long *removed) {
    // Check if the cache is already full
    if (maximumContentStoreSize > 0) {

        while ((currentCSSize + contentObjMsg->getPayloadSize()) > maximumContentStoreSize) {
            CSEntry *leastRecentlyUsedEntry = crpCs.back();
            crpCs.pop_back();
            currentCSSize -= leastRecentlyUsedEntry->payloadSize;
            *removed += leastRecentlyUsedEntry->payloadSize;

            EV_INFO << simTime() << "Cache is full, cannot insert, removing: "
                    << " " << leastRecentlyUsedEntry->prefixName
                    << " " << leastRecentlyUsedEntry->dataName
                    << " " << leastRecentlyUsedEntry->versionName
                    << " " << leastRecentlyUsedEntry->segmentNum
                    << " " << leastRecentlyUsedEntry->payloadSize
                    << " " << currentCSSize
                    << endl;
            delete leastRecentlyUsedEntry;
         }
    }

}

void CRPAlgorithms::TTLAlgorithm(int maximumContentStoreSize, ContentObjMsg *contentObjMsg, long *removed) {
    simtime_t currentTime = simTime();  // Get the current simulation time

    // Evict expired entries
    for (auto i = crpCs.begin(); i != crpCs.end();) {
        CSEntry* entry = *i;  // Get the current entry
        // If the entry has expired based on its expirytime
        if (entry->expirytime <= currentTime) {
            // Entry has expired, remove it from the cache
            i = crpCs.erase(i);  // Remove the entry from crpCs
            currentCSSize -= entry->payloadSize;  // Update the current size of the cache
            *removed += entry->payloadSize;

            EV_INFO << simTime() << "Evicting expired entry: "
                    << entry->prefixName
                    << " " << entry->dataName
                    << " " << entry->versionName
                    << " " << entry->segmentNum
                    << " " << entry->payloadSize
                    << " " << currentCSSize
                    << endl;

            delete entry;
        } else {
            // Entry has not expired, move to the next entry
            ++i;
        }
    }
}



/*
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

*/
/*
CSEntry* CRPAlgorithms::findAccessedEntry(ContentObjMsg* contentObjMsg) {
// Implement logic to find the accessed entry based on contentObjMsg
// we might compare the contentObjMsg with the entries in crpCs
// Return the corresponding cache entry or nullptr if not found
// Loop through each entry in the list of cache entries (crpCs)
// Check if the properties of the current entry match the properties of contentObjMsg
    for (CSEntry* entry : crpCs) {
        if (entry->prefixName == contentObjMsg->getPrefixName() &&
            entry->dataName == contentObjMsg->getDataName() &&
            entry->versionName == contentObjMsg->getVersionName() &&
            entry->segmentNum == contentObjMsg->getSegmentNum()) {
            // If all the properties match, return the pointer to the current entry
            return entry;
        }
    }
    // If no matching entry is found, return nullptr to indicate that the entry is not in the cache
    return nullptr; // Entry not found
}

void CRPAlgorithms::LFUAlgorithm(int maximumContentStoreSize, ContentObjMsg *contentObjMsg, long *removed)
{
    std::map<CSEntry*, int> accessCounts;
// Map to store access counts of cache entries
//a std::map named accessCounts is used to keep track of the access count for each cache entry.
//The keys of the map are pointers to CSEntry objects,
//and the values are integers representing the access count.

    // Update access counts for existing entries and accessed entry
    for (CSEntry* entry : crpCs) {
        accessCounts[entry]++;
// Increment access count for each entry
//This loop iterates through all cache entries (crpCs)
//and increments their corresponding access counts in the accessCounts map.
//This step reflects the access activity on each cache entry.
    }
//the function findAccessedEntry is used to find the cache entry corresponding to the contentObjMsg.
//If such an entry is found (accessedEntry is not nullptr), its access count in the accessCounts map
// is incremented. This accounts for the access to the specific content in this algorithm.
    CSEntry* accessedEntry = findAccessedEntry(contentObjMsg);
    if (accessedEntry) {
        accessCounts[accessedEntry]++;
    }

    // Find the entry with the lowest access count
    CSEntry* leastFrequentEntry = nullptr;
    int minAccessCount = INT_MAX;
//This loop goes through all cache entries again and compares their access counts in the accessCounts map.
//It finds the cache entry with the lowest access count (minAccessCount) and assigns it to leastFrequentEntry.
//This entry will be evicted later.
    for (auto entry : crpCs) {
        int accessCount = accessCounts[entry];
        if (accessCount < minAccessCount) {
            leastFrequentEntry = entry;
            minAccessCount = accessCount;
        }
    }

    // Evict the least frequent entry to make space
    if (leastFrequentEntry != nullptr) {
        crpCs.remove(leastFrequentEntry);
        currentCSSize -= leastFrequentEntry->payloadSize;
        *removed += leastFrequentEntry->payloadSize;

        EV_INFO << simTime() << "Evicting least frequent entry: "
                << leastFrequentEntry->prefixName << " "
                << leastFrequentEntry->dataName << " "
                << leastFrequentEntry->versionName << " "
                << leastFrequentEntry->segmentNum << " "
                << leastFrequentEntry->payloadSize << " "
                << currentCSSize << endl;

        delete leastFrequentEntry;
    }
}
*/
