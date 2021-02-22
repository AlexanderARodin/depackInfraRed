

struct InterruptBufferItem{
    volatile unsigned long timeStamp;
    volatile bool pinLevel;
};



#define BUFFER_SIZE 64
class IOBuffer{
    InterruptBufferItem items [BUFFER_SIZE];
    volatile unsigned int reader;
    volatile unsigned int writer;

    public:
        IOBuffer(){
            Reset();
        }
        void Reset(){
           reader=0;
            writer=0;
            for(int i=0; i<BUFFER_SIZE; i++){
                items[i].timeStamp = 0;
            }
        }
        
        bool isNotEmpty(){
            return items[reader].timeStamp != 0;
        }
        void copyBufferItemTo(InterruptBufferItem* dest){
            *dest = items[reader];
        }
        void makeReaderStep(){
            if(isNotEmpty()){
                items[reader].timeStamp=0;
                reader++;
                if(reader >= BUFFER_SIZE){
                    reader=0;
                }
            }
        }
        bool tryToAppend(unsigned long srcTime, bool srcPinLevel){
            if(items[writer].timeStamp != 0){
                return false;
            }
            items[writer].timeStamp = srcTime;
            items[writer].pinLevel = srcPinLevel;
            writer++;
            if(writer >= BUFFER_SIZE){
                writer=0;
            }
            return true;
        }
};

struct InterruptBufferPair{
    InterruptBufferItem prevItem;
    InterruptBufferItem currItem;

    bool isPairValid(){
        return prevItem.pinLevel == 0 && currItem.pinLevel==1;
    }
    bool isPairReversed(){
        return prevItem.pinLevel == 1 && currItem.pinLevel==0;
    }
};
