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
        float return_value(float t){
            if (type == 'R'){               //Resistor
                return value;
            } else if (type == 'C'){        //Capacitor
                tot_acc += prev_cv * t;
                return tot_acc/value;
            } else {                        //Inductor
                tot_acc += prev_cv * t;
                return (prev_cv * t)/value;
            }
        }
};


#endif