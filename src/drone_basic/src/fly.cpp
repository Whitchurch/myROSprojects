#include <iostream>
#include "ros/ros.h"
#include <algorithm>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/Range.h"
#include "sensor_msgs/LaserScan.h"


using namespace std;

//Global Variables:
//For publishers and Subscribers:

ros::Publisher fly_pub; // FOr flying the drone
ros::Subscriber sonar_sub; // For tracking Sonar sensor readings
ros::Subscriber laser_sub;
const double Max_Range = 30.0;
const double Min_Range = 1.5;
const double TakeOffHeight = 0.3;
bool stopFlightFlag = false;
bool takeOffComplete = false;

//Callback for the Sensor Message:
void sonarcallback(sensor_msgs::Range msg) 
{
    if(msg.range < TakeOffHeight)
    {
        geometry_msgs::Twist z_velocity;
        z_velocity.linear.z = 1.0;
        fly_pub.publish(z_velocity);
        //cout <<"Increasing velocity"<<msg.range; 
    }
    else 
    {
        geometry_msgs::Twist z_velocity;
        z_velocity.linear.z = 0.0;
        fly_pub.publish(z_velocity); 
        
        if(msg.range == TakeOffHeight)
        {
            
            sonar_sub.shutdown();
            cout<<"Stopped";
        }
        
    }
}

//Drone movement functions:
void FlyForward()
{
     geometry_msgs::Twist z_velocity;
     z_velocity.linear.x = 1.0;
      z_velocity.angular.z = 0.0;
     fly_pub.publish(z_velocity);
     stopFlightFlag = false; 
}

void StopFlight()
{
     geometry_msgs::Twist z_velocity;
     z_velocity.linear.x = 0.0;
      z_velocity.angular.z = 0.0;
     fly_pub.publish(z_velocity); 
     stopFlightFlag = true;
}




//Callback for Laser message
//Define the subscriber callback here:
void laser_sub_cb(const sensor_msgs::LaserScan msg)
{
    vector<float> readings = msg.ranges;
    int sector_size = readings.size()/4;

    //Split the readings into 6 sectors. Get the closest value from all 6 sectors:
      vector<float> sector_1(readings.begin(),readings.begin()+(sector_size));
      vector<float> sector_2((readings.begin()+sector_size),readings.begin()+(2*sector_size));
      vector<float> sector_3((readings.begin()+(2*sector_size)),readings.begin()+(3*sector_size));
      vector<float> sector_4((readings.begin()+(3*sector_size)),readings.begin()+(4*sector_size));


     double sector1_value =  *min_element(sector_1.begin(), sector_1.end());
     double sector2_value =  *min_element(sector_2.begin(), sector_2.end());
     double sector3_value =  *min_element(sector_3.begin(), sector_3.end());
     double sector4_value =  *min_element(sector_4.begin(), sector_4.end());

     sector1_value = min(sector1_value,Max_Range);
     sector2_value = min(sector2_value,Max_Range);
     sector3_value = min(sector3_value,Max_Range);
     sector4_value = min(sector4_value,Max_Range);


    //Create if else conditions for all states
    if(sector2_value > Min_Range && sector3_value > Min_Range)
    {
        //Keep moving:
        FlyForward();
    }
    else if((sector2_value < Min_Range || sector3_value < Min_Range)&& stopFlightFlag == false)
    {
        //Stop moving:
        StopFlight();
    }
    else if((sector2_value < Min_Range || sector3_value < Min_Range)&& stopFlightFlag == true)
    {
        if(sector1_value > sector2_value && sector1_value > sector3_value)
        {
            geometry_msgs::Twist z_velocity;
            //z_velocity.linear.x = -0.5;
            z_velocity.linear.x = 0.0;
            z_velocity.angular.z = 0.1;
            stopFlightFlag = true;
            fly_pub.publish(z_velocity); 
        }
        else if(sector4_value > sector2_value && sector4_value > sector3_value)
        {
            geometry_msgs::Twist z_velocity;
            //z_velocity.linear.x = -0.5;
            z_velocity.linear.x = 0.0;
            z_velocity.angular.z = -0.1;
            stopFlightFlag = true;
            fly_pub.publish(z_velocity); 
        }
        else if(sector1_value > sector4_value)
        {
            geometry_msgs::Twist z_velocity;
            //z_velocity.linear.x = -0.5;
            z_velocity.linear.x = 0.0;
            z_velocity.angular.z = 0.10;
            stopFlightFlag = true;
            fly_pub.publish(z_velocity); 
        }
        else if(sector1_value < sector4_value)
        {
            geometry_msgs::Twist z_velocity;
            //z_velocity.linear.x = 4.5;
            z_velocity.linear.x = 0.0;
            z_velocity.angular.z = -0.1;
            stopFlightFlag = true;
            fly_pub.publish(z_velocity); 
        }
      /*  else if(sector3_value >
            z_velocity.angular.z = -0.785;
            stopFlightFlag = true;
            fly_pub.publish(z_velocity); 
        }
      /*  else if(sector3_value > sector2_value)
        {
            FlyForward();
        }
        else if(sector2_value > sector3_value)
        {
           FlyForward(); 
        }*/

    }
    

    cout << "\n Sector 1 "<< sector1_value;
    cout << "\n Sector 2 "<< sector2_value;
    cout << "\n Sector 3 "<< sector3_value;
    cout << "\n Sector 4 "<< sector4_value;
    
    /*if(sector_1 > Max_Range && sector_2 > Max_Range && sector_3 > Max_Range && sector_4 > Max_Range)
    {
        cout<<"Move forward no change!!"<<endl;
    }
    else if(sector_1 > Max_Range && sector_2 > Max_Range  && (sector_3 < Max_Range||sector_4 < Max_Range))
    {
        cout << "Turn left !!!" <<endl;
    }*/



     
}

int main(int argc, char**argv)
{

    cout <<"Print content of the file";

    ros::init(argc, argv, "flydronenode");
    ros::NodeHandle nh;

    fly_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 1000);
    sonar_sub = nh.subscribe("/sonar_height",1000,sonarcallback);
    laser_sub = nh.subscribe("/scan",1000,laser_sub_cb);

   

    
   
    ros::Rate loop_rate(10);

    ros::spin();
    return 0;
}