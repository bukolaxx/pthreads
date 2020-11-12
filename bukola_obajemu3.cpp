//Bukola Obajemu
//  4/29/20
//PSID:1773165
//A program that uses pthread mutexes and conditions
//to simulates the enforced restrictions of a car tunnel that
//is poorly ventilated
//g++ -fpermissive -std=c++11 bukola_obajemu3.cpp -lpthread

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

//struct that will store # of the car, direction, and the time it take to go through the tunnel
struct carsT
{
    string direction; // if it's N/S
    int drive_time; //how long it takes to drive through
    int id; // car number
};

//global variables, must be static since it's shared
static int maxCars=0;
static int maxNBcars=0;
static int maxSBcars=0;
static int currCars=0;
static int currNBcars=0;
static int currSBcars=0;

static int NBtotal=0;
static int SBtotal=0;
static int waited=0;

static pthread_mutex_t mylock;
pthread_cond_t clear = PTHREAD_COND_INITIALIZER;

//north tunnel child thread
void * northTunnel(void *arg)
{
    carsT* offset = (struct carsT *) arg; //gets the struct
    //assigning variables
    int id =offset->id;
    int dt = offset->drive_time;
    bool waits = false;

    pthread_mutex_lock(&mylock);
    cout<<"Northbound car # " <<id<<" arrives at the tunnel."<<endl;
    //if amount of cars > total amount or northbound cars > total northbound cars, it has to wait
    if(currCars==maxCars || currNBcars==maxNBcars)
    {
        while(currCars==maxCars || currNBcars==maxNBcars)
        {
            waits=true;
            pthread_cond_wait(&clear,&mylock);
        }
    }
    cout<<"Northbound car # " <<id<<" enters the tunnel."<<endl;
    //updating variables
    currCars++;
    currNBcars++;
    NBtotal++;
    pthread_mutex_unlock(&mylock);
    //time it take to cross the tunnel
    sleep(dt);

    pthread_mutex_lock(&mylock);
    cout<<"Northbound car # " <<id<<" exits the tunnel."<<endl;
    //updating variables
    currCars--;
    currNBcars--;
    pthread_cond_broadcast(&clear); //broadcast when the condi is false
    if(waits) //if car waited it increments by 1
    {
        waited++;
    }
    pthread_mutex_unlock(&mylock);

    pthread_exit(nullptr);//exits pthread
}


void * southTunnel(void *arg)
{
    carsT* offset = (struct carsT *) arg;
    int id =offset->id;
    int dt = offset->drive_time;
    bool waits = false;

    //checks if car can enter tunnel
    pthread_mutex_lock(&mylock);
    cout<<"Southbound car # " <<id<<" arrives at the tunnel."<<endl;
    if(currCars==maxCars || currSBcars==maxSBcars)
    {
        while(currCars==maxCars || currSBcars==maxSBcars)
        {
            waits=true;
            pthread_cond_wait(&clear,&mylock);
        }
    }
    cout<<"Southbound car # " <<id<<" enters the tunnel."<<endl;
    currSBcars++;
    SBtotal++;
    currCars++;
    pthread_mutex_unlock(&mylock);

    sleep(dt);

    pthread_mutex_lock(&mylock);
    cout<<"Southbound car # " <<id<<" exits the tunnel."<<endl;
    currCars--;
    currSBcars--;
    pthread_cond_broadcast(&clear);
    if(waits)
    {
        waited++;
    }
    pthread_mutex_unlock(&mylock);

    pthread_exit(nullptr);
}


int main() {
    string word;
    int numCars=0; //counts number of total cars

    //takes the first 3 strings, converts them to int, then assigns to global variables.
    cin>>word;
    maxCars=stoi(word);
    cin>>word;
    maxNBcars=stoi(word);
    cin>>word;
    maxSBcars=stoi(word);

    //first outputs
    cout<<"Maximum number of cars in the tunnel: "<<maxCars<<endl;
    cout<<"Maximum number of northbound cars: "<<maxNBcars<<endl;
    cout<<"Maximum number of southbound cars: "<<maxSBcars<<endl;

    //thread id
    pthread_t tid;
    pthread_t carId[145]; //holds the address of thr cars
    pthread_mutex_init(&mylock, nullptr); //initializes the mutex, helped with making the output not overlap
    //keep track of the NB/SB id number
    int nId=1;
    int sId=1;

    //puts the input in a struct array
    while(cin>>word)
    {
        carsT* temp = new carsT;
        sleep(stoi(word)); // arrival time wait
        cin >> word;
        temp->direction = word;
        cin >> word;
        temp->drive_time = stoi(word);
        numCars++; //updates amunt of cars

        //creates the pthread for NB/SB , separates by direction
        if(temp->direction=="N")
        {
            temp->id=nId; //adds id to struct
            nId++;
            pthread_create(&tid, nullptr,northTunnel,(void*)temp);

        }
        else
        {
            temp->id=sId;
            sId++;
            pthread_create(&tid, nullptr,southTunnel,(void*)temp);

        }
        carId[numCars]=tid; //adds cars in order
    }
    //makes the threads wait their turn to join
    for(int i=1;i<=numCars;i++)
    {
        pthread_join(carId[i], nullptr);
    }
    //output statement
    cout<<NBtotal<<" northbound car(s) crossed the tunnel."<<endl;
    cout<<SBtotal<<" southbound car(s) crossed the tunnel."<<endl;
    cout<<waited<<" car(s) had to wait."<<endl;

    return 0;
}