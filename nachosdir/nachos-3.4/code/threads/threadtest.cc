// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;
int threadsDone;
//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

#if defined(CHANGED) && defined(HW1_SEMAPHORES)
int SharedVariable; 
Semaphore *s;

void SimpleThread(int which)
{
    if(s == NULL)
    {
	DEBUG('t', "USING SEMAPHORES\n");
	s = new Semaphore("shared semaphore", 1);
    }
    int num;
    int val;
   
    for (num = 0; num < 5; num++) 
    { 
        s->P();

        val = SharedVariable; 
	printf("*** Thread %d sees value %d\n", which, val);
        currentThread->Yield();
        SharedVariable = val+1;

        s->V();

        currentThread->Yield();
    }

    threadsDone--;
    while(threadsDone > 0)
	currentThread->Yield();
    val = SharedVariable; 
    
    printf("Thread %d sees final value %d\n", which, val);
}


#elif defined(CHANGED) && defined(HW1_ELEVATOR)
void
SimpleThread(int which) {
    int num;

    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}
/**
 Question 4 Elevator code starts here.
**/

#define MAX_CAPACITY 5

struct PersonThread {
    int id;
    int atFloor;
    int toFloor;
}

struct ElevatorThread {
    int numFloors;
    int currentFloor;
    int numPeopleIn;
    int gettingOn;
    int gettingOff;
}

enum Direction {
    UP,
    DOWN,
    NILL
}

struct ElevatorThread *elevator;
Direction direction = UP;
int currentFloor;
int occupied;
Condition* eleCond;
Lock* eleLock;

void Elevator(int numFloor) {
    Thread* elevator = new Thread("Elevator Thread");
    elevatorInit(numFloor); 
    elevator->Fork(elevatorStart, numFloor);
}



void elevatorInit(int numFloor) {
    floors = new struct elevatorThread[numFloor];
    for(int i = 0; i < numFloor; i++) {
	    floors[i].gettingOn = 0;
	    floors[i].gettingOff = 0;
    }

    eleCond = new Condition("elevator condition");
    eleLock = new Lock("elevator lock");
    currentFloor = -1;
    occupied = 0;
    direction = UP;
}


void elevatorStart(int numFloor) {
    do {
        eleLock->Acquire();

        // Counting elevator move
        for(int i = 0; i < 50000000; i++); 

        if(direction == UP) {
            currentFloor++;
        }
        else if(direction == DOWN) {
            currentFloor--;
        }
        else {
            break;
        }
        printf("Elevator has arrived at floor: %d.\n", currentFloor + 1);

        eleLock->Release();
        currentThread->Yield(); 

        if(currentFloor == numFloor - 1) {
            direction = DOWN;
        }
        else if(currentFloor == 0) {
            direction = UP;
        }

        eleLock->Acquire();
        eleCond->Broadcast(eleLock);
        eleLock->Release();
        eleLock->Acquire();

        while(floors[currentFloor].gettingOff > 0) {
            eleCond->Wait(eleLock);
        }
        eleLock->Release();

        eleLock->Acquire();
        eleCond->Broadcast(eleLock);

        while(floors[currentFloor].gettingOn > 0 && occupied < MAX_CAPACITY) {
            eleCond->Wait(eleLock);    
        }
        eleLock->Release();
    }
    while(1);
}



void personStart(int people) {

    PersonThread *person = (PersonThread*)people;
    //Arriving at floor.
    eleLock->Acquire();

    floors[person->atFloor-1].gettingOn++;
    printf("Person number %d would like to go to floor %d from floor %d.\n", person->id, person->toFloor, person->atFloor);

    eleLock->Release();


    eleLock->Acquire();

    while(currentFloor != person->atFloor-1 || occupied == MAX_CAPACITY)
	eleCond->Wait(eleLock);

    eleLock->Release();
    

    eleLock->Acquire();

    floors[person->toFloor-1].gettingOff++;
    floors[person->atFloor-1].gettingOn--;
    occupied++;
    printf("Person number  %d has gotten into the elevator at floor %d.\n", person->id, currentFloor + 1);
    eleCond->Broadcast(eleLock);

    eleLock->Release();

    eleLock->Acquire();

    while(currentFloor != person->toFloor-1)
	eleCond->Wait(eleLock);
   
    //Get off on desired Floor
    occupied--;
    printf("Person %d got out of the elevator at floor %d.\n", person->id, currFloor+1); 
    floors[person->toFloor-1].gettingOff--;
    eleCond->Broadcast(eleLock);

    eleLock->Release();
}

int ID;
void ArrivingGoingFromTo(int atFloor, int toFloor)
{
    Thread* person = new Thread("Person Thread");
    PersonThread *p = new PersonThread;
    p->atFloor = atFloor;
    p->toFloor = toFloor;
    p->id = ID++;
    person->Fork(personStart,(int)p);
}

#else
void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("***  HI thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}
#endif

#if defined(CHANGED) && defined(THREADS)
void ThreadTest(int n)
{
    int i;
    threadsDone = n+1;
    DEBUG('t', "threadsDone = %d\n", threadsDone);
    
    for(i = 0; i < n; i++)
    {
	Thread *t = new Thread("forked thread");
	t->Fork(SimpleThread, i + 1);
    }
    SimpleThread(0);
}
#endif

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

