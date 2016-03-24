/*==============================================================================
    Johnny Do
    861003761
    Project Part 1
*///============================================================================

//==============================================================================
//                                  CHECKLIST
//==============================================================================
/*
* 
*/ 

//==============================================================================
//                                  INCLUDES AND NAMESPACES
//==============================================================================
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cfloat>
#include <algorithm>
#include <limits>
#include "cpp.h"

using namespace std;

//==============================================================================
//                                  CONSTANTS AND GLOBALS
//==============================================================================

//Defines for Facilitation
#define TRUE true
#define FALSE false

//Facilities
facility_set* road;

//Random Seed
const long SEED = 5;

//Constants for Driver
const double REACTION_TIME = 1.0;

//Constants for Cars
const unsigned NUM_CARS = 55;
const unsigned CAR_LENGTH = 2;
const unsigned MAX_LOOK_AHEAD = 4 * CAR_LENGTH;

//Constants for Road
const int ROAD_CELLS = 120;
const int CROSSWALK_POSITION_1 = 118;
const int CROSSWALK_POSITION_2 = 119;

//Constants for Traffic Light
const int SECONDS_PER_MINUTE = 60;
const int LIGHT_POSITION = 118;
const int GREEN_LIGHT = 1;
const int YELLOW_LIGHT = 2;
const int RED_LIGHT = 3;
const double GREEN_LIGHT_DURATION = 2.0 * SECONDS_PER_MINUTE;
const double YELLOW_LIGHT_DURATION = 10.0;
const double RED_LIGHT_LOWER_DURATION = 30.0;
const double RED_LIGHT_UPPER_DURATION = 90.0;
int lightColor = GREEN_LIGHT;
const bool LIGHTS_ON = false;

//Constants for Simulation Timings
const double REPORT_INTERVAL = 3600;                //One Hour
const double END_TIME = 24 * REPORT_INTERVAL;       //Twenty Four Hours

//Variables for easy use
bool useCommandLine = false;
bool commandLineLights = false;
int commandLineCars = 0;


//==============================================================================
//                                  CAR HOUSE KEEPING
//==============================================================================

//Departure times
double* departures;

//Keeps track of car locations
bool carLocations[ROAD_CELLS];

//Keeps track of speeds of cars
int carSpeeds[ROAD_CELLS];

//==============================================================================
//                                  CAR CLASS
//==============================================================================
class car
{
    private:
        //Identifier
        int carNumber;
        
        //Flags for Motion
        bool movedFromStart;
        bool stopped;
        bool wasMoving;
        bool reservedThree;
        bool objectDetection;
        bool stayStopped;
        
        //Variables for Motion
        int speed;
        int targetSpeed;
        int obstructionSpeed;
        
        //Timing for speed
        double thinkTime;      //Time between speed change
        double nextThinkTime;  //Time to next speed change
        
        //Timing for Reaction
        double reactionTime;
        
        //Variables for Position
        int startingPoint;
        int back;
        int front;
        int ahead;
        
        //Variable for Statistiscs
        bool lapCounted;
        int laps;
        
        //Random Number Generator for Speed
        //`unsigned randomTargetSpeed(){return (2 + (rand() % 4));}
        unsigned randomTargetSpeed(){return (int)uniform(2, 5);}
        
        void initialize()
        {
            //Identifier
            carNumber = -1;

            //Flags for Motion
            movedFromStart = false;
            stopped = true;
            wasMoving = false;
            reservedThree = false;
            objectDetection = false;
            stayStopped = true;
            
            //Variables for Motion
            speed = 0;
            targetSpeed = randomTargetSpeed();
            obstructionSpeed = -1;

            //Timing
            thinkTime = 0;
            nextThinkTime = (int)uniform(1 * SECONDS_PER_MINUTE, 2 * SECONDS_PER_MINUTE);
            
            //Timing for Reaction
            reactionTime = 0;

            //Variables for Position
            startingPoint = -1;
            back = -1;
            front = -1;
            ahead = -1;

            //Variable for Statistiscs
            laps = 0;
            lapCounted = false;
        }
        
        int setLookAhead()
        {
            if(speed == 5){return 4 * CAR_LENGTH;}
            else if(speed == 4){return 3 * CAR_LENGTH;}
            else if(speed == 3 || speed == 2){return 2 * CAR_LENGTH;}
            else if(speed == 1 || speed == 0){return 1 * CAR_LENGTH;}
        }
        
        void setArrayData()
        {
            carLocations[back] = true;
            carLocations[front] = true;
            carLocations[ahead] = true;

            carSpeeds[back] = speed;
            carSpeeds[front] = speed;
            carSpeeds[ahead] = speed;
        }
        
        //Picks random speed after some random time between 1 and 2 minutes
        void pickRandomTargetSpeed()
        {
            if(thinkTime > nextThinkTime)
            {
                targetSpeed = randomTargetSpeed();
                thinkTime = 0;
                nextThinkTime = uniform(1 * SECONDS_PER_MINUTE, 2 * SECONDS_PER_MINUTE);
            }
        }
        
        //Looks ahead 8 spaces to see if anything ahead return found or not
        bool checkFront()
        {
            int i, lookAhead;
            unsigned nextPosition = ahead;
            int firstCarLocation;
            bool firstCarFound = false;
            
            if(stopped == true)
            {
                i = 0;
                lookAhead = setLookAhead();
            }
            else
            {
                i = 1;
                lookAhead = setLookAhead() + 1;
            }
            
            for(i; i < lookAhead; i++)
            {
                nextPosition += 1;
                nextPosition %= ROAD_CELLS;
                if(((*road)[nextPosition].status()) == BUSY)
                {
                    return true;
                }
                else if(nextPosition == LIGHT_POSITION && (lightColor == RED_LIGHT || lightColor == YELLOW_LIGHT))
                {
                    return true;
                }
            }
            return false; 
        }

        //Looks ahead 8 spaces to see if anything ahead return position
        bool getFrontLocation()
        {
            int i, lookAhead;
            unsigned nextPosition = ahead;
            int firstCarLocation;
            bool firstCarFound = false;
            
            if(stopped == true)
            {
                i = 0;
                lookAhead = MAX_LOOK_AHEAD;
            }
            else
            {
                i = 1;
                lookAhead = MAX_LOOK_AHEAD + 1;
            }
            
            for(i; i < lookAhead; i++)
            {
                nextPosition += 1;
                nextPosition %= ROAD_CELLS;
                if(((*road)[nextPosition].status()) == BUSY)
                {
                    if(carLocations[nextPosition] == true && firstCarFound == false)
                    {
                        firstCarFound = true;
                        firstCarLocation = nextPosition;
                    }
                }
                else if(nextPosition == LIGHT_POSITION && (lightColor == RED_LIGHT || lightColor == YELLOW_LIGHT))
                {
                    return nextPosition;
                }
            }
            if(firstCarFound == true){return false;}
            else{return -1;} 
        }

        //Looks ahead 8 spaces to see if anything ahead return position
        bool getFrontDistance()
        {
            int i, lookAhead;
            int j = ahead;
            unsigned nextPosition = ahead;
            int firstCarLocation;
            bool firstCarFound = false;
            
            if(stopped == true)
            {
                i = 0;
                lookAhead = MAX_LOOK_AHEAD;
            }
            else
            {
                i = 1;
                lookAhead = MAX_LOOK_AHEAD + 1;
            }
            
            for(i; i < lookAhead; i++)
            {
                nextPosition += 1;
                nextPosition %= ROAD_CELLS;
                if(((*road)[nextPosition].status()) == BUSY)
                {
                    if(carLocations[nextPosition] == true && firstCarFound == false)
                    {
                        firstCarFound = true;
                        firstCarLocation = nextPosition;
                    }
                }
                else if(nextPosition == LIGHT_POSITION && (lightColor == RED_LIGHT || lightColor == YELLOW_LIGHT))
                {
                    for(j; j % ROAD_CELLS != nextPosition; j++);
                    return j;
                }
            }
            if(firstCarFound == true)
            {
                for(j; j % ROAD_CELLS != nextPosition; j++);
                return j;
            }
            else{return -1;} 
        }
        
        //Finds the speed of the nearest object if any in the 8 spaces ahead
        int getFrontSpeed()
        {
            
            int i, lookAhead;
            unsigned nextPosition = ahead;
            int firstCarLocation;
            bool firstCarFound = false;
            
            //Car is staying still and checking ahead
            if(stopped == true)
            {
                i = 0;
                lookAhead = setLookAhead();
            }
            //Car is moving and checking from front of car in ahead cell
            else
            {
                i = 1;
                lookAhead = setLookAhead() + 1;
            }
            
            for(i; i < lookAhead; i++)
            {
                nextPosition += 1;
                nextPosition %= ROAD_CELLS;

                if((nextPosition == LIGHT_POSITION) && (lightColor == RED_LIGHT || lightColor == YELLOW_LIGHT))
                {
                    return 0;
                }
                else
                {
                    if(carLocations[nextPosition] == true && firstCarFound == false)
                    {
                        firstCarFound = true;
                        firstCarLocation = nextPosition;
                    }
                }
            }
            if(firstCarFound == true){return carSpeeds[firstCarLocation];}
            else{return -1;}
            return -1; 
        }
        
        //One second reaction time
        void waitReaction()
        {
            hold(REACTION_TIME);
            reactionTime += REACTION_TIME;
        }
        
        //Has the driver had the initial reaction yet
        void actualReaction()
        {
            //Obstruction exist but not reacted
            if(obstructionSpeed >= 0 && reactionTime < 1)
            {
                waitReaction();
            }
            //Obstruction exists and reaction already started
            else if(obstructionSpeed >= 0 && reactionTime >= 1)
            {
                //No Wait
            }
            //Relax that there is nothing in front
            else if(obstructionSpeed < 0)
            {
                reactionTime = 0;
            }
        }
        
        //Travel time between cells at different speeds
        void waitTravelTime()
        {
            double waitTime;
            if(speed == 1){waitTime = 3.0 / CAR_LENGTH;}
            else if(speed == 2){waitTime = (11.0 / 6.0) / CAR_LENGTH;}
            else if(speed == 3){waitTime = 1.0 / CAR_LENGTH;}
            else if(speed == 4){waitTime = (2.0 / 3.0) / CAR_LENGTH;}
            else if(speed == 5){waitTime = (1.0 / 2.0) / CAR_LENGTH;}
            else{waitTime = 0.01;}
            hold(waitTime);
            thinkTime += waitTime;
        }
        
        void updateLaps()
        {
            if(movedFromStart == false && back == startingPoint){}
            else{laps++;}
        }
        
//==============================================================================
//                                  WORK TO BE DONE HERE \/
//==============================================================================        
        
        //Adjusts the speed
        void adjustSpeed()
        {
            //No Obstructions
            if(obstructionSpeed < 0)
            {
                stayStopped = false;
                //Acceleration from cold start
                if(stopped == true && wasMoving == false)
                {
                    speed++;
                }
                //Moving
                else if(stopped == false && speed < targetSpeed)
                {
                    speed++;
                }
                //Feel Like Slowing Down
                else if(stopped == false && speed > targetSpeed)
                {
                    if(speed > targetSpeed){speed--;}
                    if(speed == 0){stopped = true;}
                }
            }
            //Object in front is completely still
            else if(obstructionSpeed == 0)
            {
                if(stopped == false)
                {
                    if(speed > obstructionSpeed){speed--;}
                    if(speed == 0){stopped = true;}
                }
            }
            //Object in front is moving
            else
            {
                stayStopped = false;
                if(speed < obstructionSpeed && speed < targetSpeed)
                {
                    speed++;
                }
                else if(speed > obstructionSpeed)
                {
                    if(speed > obstructionSpeed){speed--;}
                    if(speed == 0){stopped = true;}
                }
            }           
            waitTravelTime();            
        }
        
        //Reserve cells depending on car speed
        void moveCell()
        {
            //Cold Start
            if(stopped == true && wasMoving == false)
            {
                //reserve ahead
                ahead = (front + 1) % ROAD_CELLS;
                (*road)[ahead].reserve();
                
                //Update all arrays with information of location and speed
                setArrayData();
                stopped = false;
                wasMoving = true;
            }
            //Moving
            else if(stopped == false)
            {
                movedFromStart = true;
                //Unreserve original back
                carSpeeds[back] = -1;
                carLocations[back] = false;
                (*road)[back].release();
                
                back = front;
                front = ahead;
                ahead = (front + 1) % ROAD_CELLS;
                (*road)[ahead].reserve();
                setArrayData();              
            }
            //Stopping
            else if(stopped == true && wasMoving == true)
            {
                //Unreserve original back
                carSpeeds[back] = -1;
                carLocations[back] = false;
                (*road)[back].release();
                
                back = front;
                front = ahead;
                ahead = (front + 1) % ROAD_CELLS;
                wasMoving = false;
                
                carLocations[back] = true;
                carLocations[front] = true;
                carLocations[ahead] = false;

                carSpeeds[back] = speed;
                carSpeeds[front] = speed;
                carSpeeds[ahead] = -1;                
            }
            //`//Stopping due to Obstruction
            else if(stopped == true && wasMoving == true && obstructionSpeed == 0)
            {
                if(stayStopped == false)
                {
                    //Unreserve original back
                    carSpeeds[back] = -1;
                    carLocations[back] = false;
                    (*road)[back].release();
                    
                    back = front;
                    front = ahead;
                    ahead = (front + 1) % ROAD_CELLS;
                    wasMoving = false;
                    
                    carLocations[back] = true;
                    carLocations[front] = true;
                    carLocations[ahead] = false;

                    carSpeeds[back] = speed;
                    carSpeeds[front] = speed;
                    carSpeeds[ahead] = -1;
                    stayStopped = true;
                }                
            }
        }
        
//==============================================================================
//                                  WORK TO BE DONE HERE /\
//==============================================================================         
        
        void move()
        {
            //Decide if new speed is necessary
            pickRandomTargetSpeed();
            
            //Detection of obstruction
            obstructionSpeed = getFrontSpeed();
            
            //Actual Reaction or Not
            actualReaction();
            
            //Adjust Speed to situation
            adjustSpeed();
            
            //Wait for the travel time
            waitTravelTime();
            
            //Move cells according to speed
            moveCell();
            
            if(back == 0 && movedFromStart == true && lapCounted == false){laps++; lapCounted = true;}
            if(back == ROAD_CELLS / 2){lapCounted = false;}
            //`if(speed == 0){cout << "CAR " << carNumber << " HAS STOPPED AT " << endl;}
            
            //Allow other cars to move if they do
            //Testing purposes only
            //`hold(1);        
        }
        
    public:
        //Base Constructor
        car(){initialize();}
        
        //Constructor with some user defined intialization
        car(const unsigned int& number, const unsigned& start, const unsigned& randomTargetSpeed)
        {
            initialize();
            carNumber = number;
            
            startingPoint = start;
            
            thinkTime = 0;
            back = start % ROAD_CELLS;
            front = (start + 1) % ROAD_CELLS;
            ahead = (front + 1) % ROAD_CELLS;
            
            //Setting Car in proper location
            carLocations[back] = true;
            carLocations[front] = true;
            
            //Setting Speed of car to locations
            carSpeeds[back] = speed;
            carSpeeds[front] = speed;
        }
        
        //Allow Cars to Move
        void simulate()
        {
            char* buffer = new char [32];
            sprintf(buffer, "Car #%d", carNumber);
            
            create(buffer);
            set_priority(1);
            
            //Reserve Locations
            (*road)[back].reserve();
            (*road)[front].reserve();
            
            while(simtime() < END_TIME){move();}
        }
        
        //Returns laps done by car
        int getLaps(){return laps;}
        
        //Prints Car information
        void printReport(){cout << "Car Number: " << carNumber << " Back of Car: " << back << " Front of Car: " << front << " Car Speed: " << speed << " Laps: " << laps << endl;}
        
        //Destructor for car class
        ~car(){}
};

//List of cars
vector<car> carList;

//==============================================================================
//                                  SIMULATION FUNCTIONS
//==============================================================================

//Light simulation funciton
void initializeLights()
{
    if(useCommandLine == false)
    {
        if(LIGHTS_ON == true)
        {
            create("Traffic Light");
            set_priority(2);
            while(simtime() < END_TIME)
            {
                //light changest to green
                lightColor = GREEN_LIGHT;
                //`cout << "AT TIME " << simtime() << " STREET LIGHT IS GREEN" << endl;
                
                //Wait for green light time
                hold(GREEN_LIGHT_DURATION);            
                
                //Light changes to yellow
                lightColor = YELLOW_LIGHT;
                //`cout << "AT TIME " << simtime() << " STREET LIGHT IS YELLOW" << endl;
                
                //Wait for yellow light time
                hold(YELLOW_LIGHT_DURATION);            
                
                //Light changes to red
                lightColor = RED_LIGHT;
                //`cout << "AT TIME " << simtime() << " STREET LIGHT IS RED" << endl;
                
                //Hold the crosswalk
                (*road)[CROSSWALK_POSITION_1].reserve();
                (*road)[CROSSWALK_POSITION_2].reserve();
                
                //Wait for red light mean time
                hold(uniform(RED_LIGHT_LOWER_DURATION, RED_LIGHT_UPPER_DURATION));
                
                //Releases the crosswalk
                (*road)[CROSSWALK_POSITION_1].release();
                (*road)[CROSSWALK_POSITION_2].release();
            }
        }
    }
    else
    {
        if(commandLineLights == true)
        {
            create("Traffic Light");
            set_priority(2);
            while(simtime() < END_TIME)
            {
                //light changest to green
                lightColor = GREEN_LIGHT;
                //`cout << "AT TIME " << simtime() << " STREET LIGHT IS GREEN" << endl;
                
                //Wait for green light time
                hold(GREEN_LIGHT_DURATION);            
                
                //Light changes to yellow
                lightColor = YELLOW_LIGHT;
                //`cout << "AT TIME " << simtime() << " STREET LIGHT IS YELLOW" << endl;
                
                //Wait for yellow light time
                hold(YELLOW_LIGHT_DURATION);            
                
                //Light changes to red
                lightColor = RED_LIGHT;
                //`cout << "AT TIME " << simtime() << " STREET LIGHT IS RED" << endl;
                
                //Hold the crosswalk
                (*road)[CROSSWALK_POSITION_1].reserve();
                (*road)[CROSSWALK_POSITION_2].reserve();
                
                //Wait for red light mean time
                hold(uniform(RED_LIGHT_LOWER_DURATION, RED_LIGHT_UPPER_DURATION));
                
                //Releases the crosswalk
                (*road)[CROSSWALK_POSITION_1].release();
                (*road)[CROSSWALK_POSITION_2].release();
            }            
        }
    }
}

//Set up simulation
void initializeSimulation()
{
    if(useCommandLine == false)
    {   
        //Set Up Road
        road = new facility_set("Road", ROAD_CELLS);
        
        //Set Up Departures
        departures = new double[ROAD_CELLS];
        
        //Set Up Car Position
        unsigned newCarBack = (LIGHT_POSITION - 1) - CAR_LENGTH * 2;
        unsigned newCarFront = newCarBack + 1;
        
        //Initialize Speed Track
        for(unsigned i = 0; i < ROAD_CELLS; i++){carSpeeds[i] = -1;}
        
        //Locations Of Cars On Track
        for(unsigned i = 0; i < ROAD_CELLS; i++){carLocations[i] = false;}
        
        //Initialize Leave Track
        for(unsigned i = 0; i < NUM_CARS; i++){departures[i] = 0;}

        //Initialize Cars on Track
        for(unsigned i = 0; i < NUM_CARS; i++)
        {
            //Setting Car Initial Conditions
            carList.push_back(car(i, newCarBack, 5));
            //cout << "Car " << i << " Back is at " << newCarBack << endl; 
            
            //Setting up car location
            carLocations[newCarBack] = true;
            carLocations[newCarFront] = true;
            
            //Setting Next Car's Location
            newCarBack -= CAR_LENGTH;
            newCarFront = newCarBack + 1;
        }
        
        //Initialize Light On Track
        initializeLights();
        
        //Allow cars to simulate
        for(unsigned i = 0; i < NUM_CARS; i++){carList[i].simulate();}
    }
    else
    {
        //Set Up Road
        road = new facility_set("Road", ROAD_CELLS);
        
        //Set Up Departures
        departures = new double[ROAD_CELLS];
        
        //Set Up Car Position
        unsigned newCarBack = (LIGHT_POSITION) - CAR_LENGTH;
        unsigned newCarFront = newCarBack + 1;
        
        //Initialize Speed Track
        for(unsigned i = 0; i < ROAD_CELLS; i++){carSpeeds[i] = -1;}
        
        //Locations Of Cars On Track
        for(unsigned i = 0; i < ROAD_CELLS; i++){carLocations[i] = false;}
        
        //Initialize Leave Track
        for(unsigned i = 0; i < commandLineCars; i++){departures[i] = 0;}

        //Initialize Cars on Track
        for(unsigned i = 0; i < commandLineCars; i++)
        {
            //Setting Car Initial Conditions
            carList.push_back(car(i, newCarBack, 5));
            //cout << "Car " << i << " Back is at " << newCarBack << endl; 
            
            //Setting up car location
            carLocations[newCarBack] = true;
            carLocations[newCarFront] = true;
            
            //Setting Next Car's Location
            newCarBack -= CAR_LENGTH;
            newCarFront = newCarBack + 1;
        }
        
        //Initialize Light On Track
        initializeLights();
        
        //Allow cars to simulate
        for(unsigned i = 0; i < commandLineCars; i++){carList[i].simulate();}
    }
}

//Counts throughput
int getThroughput()
{
    if(useCommandLine == false)
    {
        int totalLaps = 0;
        for(int i = 0; i < NUM_CARS; i++){totalLaps += carList[i].getLaps();} 
        return totalLaps;
    }
    else
    {
        int totalLaps = 0;
        for(int i = 0; i < commandLineCars; i++){totalLaps += carList[i].getLaps();} 
        return totalLaps;
    }
}

//==============================================================================
//                                  MAIN FUNCTION
//==============================================================================
extern "C" void sim(int argc, char** argv)
{
    if(argc > 2)
    {
        useCommandLine = true;
        commandLineLights = atoi(argv[1]) == 0 ? false : true;
        commandLineCars = atoi(argv[2]) < 60 ? atoi(argv[2]) : 59;
    }
    
    //`trace_on();
	reseed(NIL, SEED);
    srand(SEED);
    if(useCommandLine == false)
    {
        create("Car Sim");
        set_priority(LONG_MAX);
        initializeSimulation();
        while(simtime() < END_TIME)
        {
            hold(REPORT_INTERVAL);
            cout << "Simulation Time: " << simtime() << endl;
            for(int i = 0; i < NUM_CARS; i++)
            {
                carList[i].printReport();
            }
            cout << "Total Laps Completed: " << getThroughput() << endl;
            cout << "Average Laps Completed: " << getThroughput() / NUM_CARS << endl << endl;
        }
    }
    else
    {       
        create("Car Sim");
        set_priority(LONG_MAX);
        initializeSimulation();
        while(simtime() < END_TIME)
        {
            hold(END_TIME);
            //`cout << "Simulation Time: " << simtime() << endl;
            for(int i = 0; i < commandLineCars; i++)
            {
                carList[i].printReport();
            }
            cout << "Total Laps Completed: " << getThroughput() << endl;
            cout << "Average Laps Completed: " << (getThroughput() + 1) / commandLineCars << endl << endl;
        }
	}
}
