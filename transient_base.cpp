#include "component.hpp"
#include "scientific_converter.hpp"
#include <iostream>
#include <sstream>

int main(){


///////////////////////////////////////////////////////////////////////////////////////


    //NEW VARIABLE DECLARATIONS (MERGED WITH OLD ONES)
    vector<node > node_vector;      //keep track of nodes and their connected components
    vector<base_class*> components; //stores all the components in the circuit
    string node1, node2, name, temp_value, output_type, input, source_type;
    char type;
    float value, stop_time, time_step;
    int component_counter = 0; //component counter for figuring out current number of component




    //NEW NETLIST PARSER WITH TRANSIENT SUPPORT
    while (getline(cin, input))
    {
        float frequency = 0;
        float amplitude = 0;

        istringstream ss(input);
        ss >> name;
        if (name[0]=='.'){          //Checks for end of file (.tran 0 <stop time> 0 <timestep>) followed by (.end)
            ss >> temp_value;       //Removes dummy 0 in .trans
            ss >> temp_value;
            stop_time = scientific_converter(temp_value);
            ss >> temp_value;       //Removes dummy 0 after 
            ss >> temp_value;
            time_step = scientific_converter(temp_value);
            break;                  //Stops loop to avoid reading .end creating run time errors
        } else {                    //Parser for RLC and sources
            ss >> node1;
            ss >> node2;
            ss >> temp_value;
            if (tolower(temp_value[0])=='s'){       //Checks for sine sources   SINE(<dc offset> <amplitude> <frequency>)
                source_type = "SINE";
                temp_value.substr(0, 5);            //Removes " SINE( "
                value = scientific_converter(temp_value);
                ss >> temp_value;
                amplitude = scientific_converter(temp_value);
                ss >> temp_value;
                temp_value.pop_back()
                frequency = scientific_converter(temp_value);
            }else{                                  //Not a sine source
                source_type = "DC";
                value = scientific_converter(temp_value);
            }
            
        }


///////////////////////////////////////////////////////////////////////////////////////


        //Add components/sources to component vector (WITH SINE SOURCE SUPPORT)
        if (is_component){
            components.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
        }else if (is_source){
            components.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
        }else{
            //exit(1);
        }



    }


///////////////////////////////////////////////////////////////////////////////////////


    //NEW FOR LOOP FOR TRANSIENT SIMS
    for (float time=0; time<=stop_time; time+=time_step){
        /*
        Put old G matrix code here
        Put prints for completed matrix here
        Change all the values inside return_value(_) to the variable 'time'
        */
    }


}