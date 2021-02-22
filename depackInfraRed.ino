#include "IOBuffer.h"


enum { THELED=13, IK_IN = 2, US_IN = 3, US_STARTER = 4 };
#define TIME_CUTOFF 200000


IOBuffer ioBuffer;
volatile unsigned long usDelay;

void setup(){
    Serial.begin(9600);
    msg(    "\n\n#################\n"
                "# >> Started << #\n"
                "#################\n\n");

    
    pinMode(THELED, OUTPUT);
    
    pinMode(IK_IN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IK_IN), ioChange, CHANGE);

    pinMode(US_STARTER, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(US_IN), usChange, CHANGE);
 }

void loop(){
    static InterruptBufferPair pair = {0,1,0,0};
    static bool notEmpty;
        do{
                noInterrupts();
                notEmpty = ioBuffer.isNotEmpty();
                if(notEmpty){
                    ioBuffer.copyBufferItemTo(&(pair.currItem));
                    ioBuffer.makeReaderStep();
                }
                interrupts();
                
            if(notEmpty){
                processPair(pair);
                pair.prevItem = pair.currItem;
            }else{
                restartUS();
            }
        }while(notEmpty);
}

void restartUS(){
    digitalWrite(US_STARTER, HIGH);
    delay(10);
    digitalWrite(US_STARTER, LOW);
}

void processPair(const InterruptBufferPair &pair){
    static unsigned long prevDeltaTime;
    static unsigned long deltaTime;
    static unsigned long sum;
    static byte bitCounter;
    
    deltaTime = pair.currItem.timeStamp - pair.prevItem.timeStamp;
        if(pair.isPairValid()){
            if(prevDeltaTime > TIME_CUTOFF){
                msg("\n\nXXXXXX");
            }else{
                //dbg( prevDeltaTime );
            }
            //msg( "\t" );
            //msg( deltaTime );
            //msg( "\t" );
            char ch = Interpret(deltaTime+prevDeltaTime);
            msg( String(ch) );
                {{{
                    if(ch=='S'){
                        bitCounter=0;
                    }
                    if(ch=='r'){
                        bitCounter=32;
                    }
                    if(ch=='*'){
                        sum=0;
                        bitCounter=0;
                    }
                    if(ch=='0'){
                        sum = sum << 1;
                        bitCounter++;
                    }
                    if(ch=='1'){
                        sum = sum << 1;
                        sum |= 1;
                        bitCounter++;
                    }
                msg( "\t" );
                if(bitCounter<32){
                    msg( "." );
                }else{
                    msg( String(sum>>16, HEX) );
                    msg( "\t" );
                    msg( String(sum&0xFFFF, HEX) );
                }
                }}}
            //msg( "\t" );
            //msg( deltaTime+prevDeltaTime );
            msg("\t\tUS:"+String(usDelay*0.034)+"");
            msg("\n");
        }else{
            if(!pair.isPairReversed()){
                msg( "\n\nERROR SEQUENSE:\t" );
                msg( String(deltaTime) );
                msg( "-" );
                msg( String(pair.currItem.pinLevel) );
                msg("\n\n");
            }
        }
        
    prevDeltaTime = deltaTime;
}

char Interpret( unsigned long impulsPeriod){
    if(impulsPeriod < 500 )return ' ';
    if(impulsPeriod < 1500 )return '0';
    if(impulsPeriod < 2500 )return '1';
    if(impulsPeriod < 3500 )return 'r';
    if(impulsPeriod < 5500 )return '*';

    
    return 'S';
}


void ioChange(){
    static unsigned long timeStamp;
    timeStamp = micros();
    
    static bool ikIn;
    ikIn = digitalRead(IK_IN);
    digitalWrite( THELED, !ikIn );

    static bool resultStatus;
    resultStatus = ioBuffer.tryToAppend(timeStamp,ikIn);
        if(!resultStatus){
            ioBuffer.Reset();
            dbg(String("ACHTUNGGGG!!!!!"));
        }
}


void usChange(){
    static bool usIn;
    usIn = digitalRead(US_IN);

    static unsigned long prevTimeStamp;
    if(usIn){
        prevTimeStamp = micros();    
    }else{
        usDelay = micros() - prevTimeStamp;
        prevTimeStamp = 0;
    }
}


void dbg(const String& str){
    msg("\nDBG:<");
    msg(str);
    msg(">\n");
}
void msg(const String& str){
    Serial.print(str);
}
