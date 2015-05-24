//============================================================================
// Name        : main.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <string>
#include <cstring>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <array>
#include <limits>
#include <atomic>
#include <cstdint>

#include <stdarg.h>
#include <omp.h>

#include <sys/time.h>
using namespace std;


#define PERFORMANCE 0
#define PERFORMANCE_LOOPS (1000*1000*1000)
#define EXAMPLE 9

#if EXAMPLE != 6
#include "Lock.h"
#include "CyclicBuffer.h"
#include "CyclicBufferSimple.h"
#include "Stack.h"
#include "Memory.h"
#include "HardwareC.h"
#include "Hardware.h"
#include "Timers.h"
#include "Log.h"
#include "OpenMP.h"
#include "Pipeline.h"
#endif


/**
 * Dummy lock
 */
class LockDummySimple {
public:

    LockDummySimple() {
        cout << "Locked context" << endl;
    }

    ~LockDummySimple() {
        cout << "Lock is freed" << endl;
    }

protected:

private:

};// class LockDummy

/*
 * Output of this code is going to be
 * Locked context
 * Lock is freed
 */
int testDummyLock1() {
#if (__cplusplus >= 201103) // use "auto" if C++11 or better
    auto myDummyLock = LockDummySimple();
#else
    LockDummySimple myDummyLock = LockDummySimple();
#endif
    cout << "Protected context" << endl;
    return 0;
}


#if EXAMPLE == 2

/**
 * Dummy lock
 */
class LockDummySimple {
public:

    LockDummySimple() {
        cout << "Locked context" << endl;
    }

    ~LockDummySimple() {
        cout << "Lock is freed" << endl;
    }

protected:

private:

};// class LockDummySimple

/*
 * Output of this code is going to be
 * Locked context
 * Lock is freed
 * End of main
 */
int main() {
    {
        auto myDummyLock = LockDummySimple();
    }

    cout << "End of main" << endl;
    return 0;
}

#endif // EXAMPLE == 2

/**
 * Function returns number of elements in the cyclic buffer.
 * Compiler will fail if the value can not be calculated in compilation time.
 */
constexpr size_t calculateCyclicBufferSize() {
    return 10;
}

static CyclicBufferSimple<uint8_t, LockDummy, calculateCyclicBufferSize()> myCyclicBufferSimple;

int mainExample5() {
    for (int i : { 1, 3, 11 }) {
        myCyclicBufferSimple.add(i);
    }

    uint8_t val;
    while (myCyclicBufferSimple.remove(val)) {
        cout << (int) val << endl;
    }
    return 0;
}


#if EXAMPLE == 5
#include "CyclicBufferC.h"

int mainExample6() {
    for (int i = 0;i < 4;i++) {
        CyclicBufferAdd(&myCyclicBuffer, i);
    }

    uint8_t val;
    while (CyclicBufferRemove(&myCyclicBuffer, &val)) {
        cout << (int) val << endl;
    };
    return 0;
}
#endif



static CyclicBuffer<uint_fast8_t, LockDummy, calculateCyclicBufferSize()> myCyclicBuffer;

static uint32_t myDynamicCyclicBufferData[calculateCyclicBufferSize()];
CyclicBufferDynamic<uint32_t, LockDummy> myDynamicCyclicBuffer(calculateCyclicBufferSize(), &myDynamicCyclicBufferData);

int mainExample7() {
    // I want to inspect assembler code generated by the C++ compiler
    myCyclicBuffer.add(0);
    // I demonstrate here a range based loop from C++11
    for (int i : { 1, 2, 3 }) {
        myCyclicBuffer.add(i);
    }

    while (!myCyclicBuffer.isEmpty()) {
        uint8_t val;
        myCyclicBuffer.remove(val);
        cout << (int) val << endl;
    }
    return 0;
}

constexpr size_t calculateStackSize() {
    return 10;
}

typedef uint8_t DataBlock[64];

static Stack<DataBlock, LockDummy, calculateStackSize()> myMemoryPool;

int mainExample8() {
    DataBlock dummyDataBlock[10];

    myMemoryPool.push(&(dummyDataBlock[0]));
    (dummyDataBlock[0])[0] = 0;

    for (int i = 1;i < 10;i++) {
        myMemoryPool.push(&(dummyDataBlock[i]));
        (dummyDataBlock[i])[0] = i;
    }

    while (!myMemoryPool.isEmpty()) {
        DataBlock* dummyDataBlockRes;
        myMemoryPool.pop(&dummyDataBlockRes);
        cout << (int) (*dummyDataBlockRes)[0] << endl;
    }
    return 0;
}


static uint8_t dmaMemoryDummy[512];
static MemoryRegion dmaMemoryRegion("dmaMem", (uintptr_t)dmaMemoryDummy, sizeof(dmaMemoryDummy));

static_assert((sizeof(dmaMemoryDummy) >= MemoryAllocatorRaw::predictMemorySize(63, 3, 2)), "DmaMemoryDummy region is not large enough");
static MemoryAllocatorRaw dmaAllocator(dmaMemoryRegion, 63, 3, 2);

static MemoryPoolRaw<LockDummy, 7> dmaPool("dmaPool", dmaAllocator);

static int mainExample9() {

    uint8_t* block;
    cout << "base=" << reinterpret_cast<uintptr_t>(dmaMemoryDummy) << endl;
    bool res;
    for (int i = 0;i < 5;i++)
    {
        res = dmaPool.allocate(&block);
        if (res) {
            cout << "\t" << i << " block=" << reinterpret_cast<uintptr_t>(block) << endl;
            dmaPool.free(block);
        }
    }

    return 0;
}


#ifdef REAL_HARDWARE
static struct PIO *pios = (PIO*)0xFFFFF200;
#else
static struct PIO pioDummy[5];
static struct PIO *pios = pioDummy;
#endif


static void enableOutput(PIO_NAME name, int pin, int value) {
    struct PIO *pio = &pios[name];
    uint32_t mask = 1 << pin;
    if (value) {
        pio->PIO_SODR = mask;
    }
    else {
        pio->PIO_CODR = mask;
    }
    pio->PIO_PER = mask;
    pio->PIO_OER = mask;
}

static void mainExample10() {
    enableOutput(PIO_A, 2, 1);
}


class HardwarePIO : HardwareModule {
public:
    HardwarePIO(const uintptr_t address) {
        interface = (struct Interface*)address;
    }
    ~HardwarePIO() {}

    struct Interface {
        HardwareRegister32WO PIO_PER  ;
        HardwareRegister32WO PIO_PDR  ;
        HardwareRegister32RO PIO_PSR  ;
        HardwareRegister32NotUsed RESERVED ;
        HardwareRegister32WO PIO_OER  ;
        HardwareRegister32WO PIO_ODR  ;
        HardwareRegister32RO PIO_OSR  ;
        HardwareRegister32NotUsed RESERVED ;
        HardwareRegister32NotUsed RESERVED ;
        HardwareRegister32NotUsed RESERVED ;
        HardwareRegister32NotUsed RESERVED ;
        HardwareRegister32NotUsed RESERVED ;
        HardwareRegister32WO PIO_SODR ;
        HardwareRegister32WO PIO_CODR ;
    };
    //static_assert((sizeof(struct Interface) == (14*sizeof(uint32_t))), "struct interface is of wrong size, broken alignment?");
    enum Name {A, B, C, D, E, F, LAST};

    inline Interface& getInterface(Name name) const {return interface[name];};

    inline void enableOutput(Name name, int pin, int value);
protected:
    struct Interface* interface;
};

inline void HardwarePIO::enableOutput(Name name, int pin, int value)
{
    struct Interface& interface = getInterface(name);
    uint32_t mask = 1 << pin;
    if (value) {
        interface.PIO_SODR = mask;
    }
    else {
        interface.PIO_CODR = mask;
    }
    interface.PIO_PER = mask;
    interface.PIO_OER = mask;
}

static HardwarePIO hardwarePIO(reinterpret_cast<uintptr_t>(pioDummy));

static void mainExample11() {
    hardwarePIO.enableOutput(HardwarePIO::A, 2, 1);
    //uint32_t per = hardwarePIO.getInterface(HardwarePIO::A).PIO_PER;
    cout << pioDummy[0].PIO_SODR << endl;
    cout << pioDummy[0].PIO_CODR << endl;
    cout << pioDummy[0].PIO_PER << endl;
    cout << pioDummy[0].PIO_OER << endl;
}

static const char *helloWorldStr = 0;

#if 0
class HelloWorld {
public:
    HelloWorld() {
        if (helloWorldStr == 0)
            cout << "Hello, world!" << endl;
        else
            cout << helloWorldStr << endl;
    }
};
#endif

class HelloWorld {
public:
    HelloWorld() {
        printHello();
    }
protected:
    void printHello() {
        cout << "Hello, world!" << endl;
    }
};

static HelloWorld helloWorld;

int mainExample12()
{
    cout << "Hello from main()!" << endl;
    return 0;
}

static void mainExpirationHandler(const Timer& timer) {
    TimerID timerId = timer.getId();
    uintptr_t data = timer.getApplicationData();
    cout << "Expired id=" << timerId << ",appdata=" << data << endl;
}


TimerLockDummy timerLock;
static TimerList timerList(3, 3, mainExpirationHandler, timerLock);
int testTimer()
{
    SystemTime currentTime = 0;
    for (int i = 0;i < 3;i++) {
        SystemTime nearestExpirationTime;
        TimerError err = timerList.startTimer(currentTime, nearestExpirationTime, i);
        if (err == TimerError::Ok) {
            cout << "nearestExpirationTime=" << nearestExpirationTime << endl;
        }
        else {
            cout << "timer start failed for timer " << i << endl;
        }
    }

    timerList.processExpiredTimers(3);
    return 0;
}

template<const uint32_t N> struct factorial
{
    static constexpr uint32_t value = N * factorial<N - 1>::value;
};

template<>struct factorial<0>
{
    static constexpr uint32_t value = 1;
};


void testLog(void) {
    LOG_INFO("This is info %d", 1);
    LOG_ERROR("This is error %d", 2);
}

int sum()
{
    return 0;
}

template<typename ... Types>
int sum (int first, Types ... rest)
{
    return first + sum(rest...);
}

const int SUM = sum(1, 2, 3, 4, 5);





const int TIMERS_COUNT = 3;
const int TIMERS_SIZE = 3;
uint32_t dummyTimers[TIMERS_SIZE][TIMERS_COUNT];

HardwareTimer *myHardwareTimers = new (&dummyTimers[0][0]) HardwareTimer[TIMERS_COUNT];

void testHardwareTimers(void) {
    for (int i = 0;i < TIMERS_COUNT;i++) {
        myHardwareTimers[i].start();
    }
}


void testOpenMP() {
    uint8_t packet[1024];
    uint8_t encPacket[1024];
    encryptPacket(packet, encPacket, sizeof(encPacket));
}

void testOpenMpReduction(void) {
    int a = 0;
    #pragma omp parallel reduction (+:a)
    {
      a = 1;
    }
    cout << a << endl;
}

void testLockOmp() {
    {
        LockOmp();
    }
}

volatile uint8_t myArray[(size_t)8*1024*1024];
uint_fast32_t testOpenMPLoop() {
    uint_fast32_t sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (uint64_t i=0; i < 100; i++)
    {
        sum += myArray[i];
    }

    struct timespec t2, t3;
    double dt1;
    clock_gettime(CLOCK_MONOTONIC,  &t2);
    #pragma omp parallel for reduction(+:sum)
    for (uint64_t i=0; i < sizeof(myArray); i++)
    {
        sum += myArray[i];
    }
    clock_gettime(CLOCK_MONOTONIC,  &t3);
    dt1 = (t3.tv_sec - t2.tv_sec) + (double) (t3.tv_nsec - t2.tv_nsec) * 1e-9;
    cout << "time:  " << dt1 << endl;
    return sum;
}

typedef PipelineTask<int, LockDummy, 3> MyPipelineTask;

MyPipelineTask pipelineTask3("3");
MyPipelineTask pipelineTask2("2", &pipelineTask3);
MyPipelineTask pipelineTask1("1", &pipelineTask2);

void testPipeline() {
    int data = 0;
    pipelineTask1.addJob(data);
    pipelineTask1.doJob();
    pipelineTask2.doJob();
    pipelineTask3.doJob();
}

int main()
{
    testPipeline();
//    testDummyLock1();
//    testHardwareTimers();
//    testBinaryLog3();
//    testTimer();
//    testOpenMpReduction();
     //testLockOmp();
    // testOpenMPLoop();
    // testOpenMP();

    return 0;
}
