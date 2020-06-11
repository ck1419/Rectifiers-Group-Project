#ifndef basic_component_hpp
#define basic_component_hpp


#include "base_class.hpp"


//Class for RCL components
class basic_component: public base_class
{
    protected:
        float tot_acc = 0;          //Stores total current/voltage in component


    public:
        //Constructor
        basic_component(char c_type, float c_value, node *c_node1, node *c_node2, string c_name){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
        }


        //Returns value of component depending on the type
        float return_value(float t, bool final_loop_checker){
            if (type == 'R'){               //Resistor
                return value;
            } else if (type == 'C'){		//Capacitor
            if (final_loop_checker){
                tot_acc += prev_cv * t;
                return tot_acc/value;
            } else {
                float temp_acc = tot_acc + prev_cv * t;
                return temp_acc/value;
                }
            } else {                        //Inductor
                if(final_loop_checker){
                    tot_acc += prev_cv * t;
                    return tot_acc/value;
                } else {
                    float temp_acc = tot_acc + prev_cv * t;
                    return temp_acc/value;
                }
            }
        }
};


#endif
