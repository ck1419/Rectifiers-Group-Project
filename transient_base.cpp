#include "component.hpp"
#include "scientific_converter.hpp"
#include <iostream>
#include <sstream>

int main(){


    //NEW VARIABLE DECLARATIONS (MERGED WITH OLD ONES)
    vector<node > node_vector;      //keep track of nodes and their connected components
    vector<base_class*> components; //stores all the components in the circuit
    string node1, node2, name, temp_value, output_type, input;
    char type;
    float value, stop_time, time_step;
    int component_counter = 0; //component counter for figuring out current number of component




    //NEW NETLIST PARSER WITH TRANSIENT SUPPORT
    while (getline(cin, input))
    {
        istringstream ss(input);
        ss >> name;
        //Checks for end of file (.tran 0 <stop time> 0 <timestep>) followed by (.end)
        if (name[0]=='.'){
            ss >> temp_value;       //Removes dummy 0 in .trans
            ss >> temp_value;
            stop_time = scientific_converter(temp_value);
            ss >> temp_value;       //Removes dummy 0 after 
            ss >> temp_value;
            time_step = scientific_converter(temp_value);
            break;                  //Stops loop to avoid reading .end creating run time errors
        } else {            //Parser for RLC and sources
            ss >> node1;
            ss >> node2;
            ss >> temp_value;
            value = scientific_converter(temp_value);
        }
    }




    //NEW FOR LOOP FOR TRANSIENT SIMS
    for (float time=0; time<=stop_time; time+=time_step){
        /*
        Put old G matrix code here
        Put prints for completed matrix here
        Change all the values inside return_value(_) to the variable 'time'
        */
    }


}