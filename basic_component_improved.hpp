#ifndef basic_component_hpp
#define basic_component_hpp


#include "base_class_improved.hpp"


//Class for RCL components
class basic_component: public base_class
{
    public:
        //Constructor
        basic_component(char c_type, double c_value, node *c_node1, node *c_node2, int c_name){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
        }


        //Returns value of component depending on the type
        double return_value(double t, bool final_loop_checker){
            if (type == 'C'){		//Capacitor
	            if (final_loop_checker){
        	        tot_acc += prev_cv * t;
               		return tot_acc/value;
            	} else {
                	double temp_acc = tot_acc + prev_cv * t;
                	return temp_acc/value;
               	}
            } else {                //Inductor and Resistor
                return value;
            }
    	}
};


#endif
