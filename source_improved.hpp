#ifndef source_hpp
#define source_hpp


#include "base_class_improved.hpp"


//Class for IV sources
class source: public base_class
{
    protected:
        string output_type;
        double frequency, amplitude;


    public:
        //Constructor
        source(char c_type, string c_output_type, node *c_node1, node *c_node2, int c_name, double c_frequency, double c_value, double c_amplitude){
            type = c_type;
            output_type = c_output_type;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
            frequency = c_frequency;
            value = c_value;
            amplitude = c_amplitude;
        }


        //Returns the output value of the source depending on the type of the source
        double return_value(double t, bool final_loop_checker){
            if (output_type == "sine"){
                return ( amplitude*sin(2*M_PI*frequency*t) ) + value;
            }
            if (output_type == "dc"){
                return value;
            }
        }
};


#endif