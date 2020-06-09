#ifndef source_hpp
#define source_hpp


#include "base_class.hpp"


//Class for IV sources
class source: public base_class
{
    protected:
        string output_type;
        float frequency, amplitude;


    public:
        //Constructor
        source(char c_type, string c_output_type, node *c_node1, node *c_node2, string c_name, float c_frequency, float c_value, float c_amplitude){
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
        float return_value(float t){
            if (output_type == "sine"){
                return ( amplitude*sin(2*M_PI*frequency*t) ) + value;
            }
            if (output_type == "dc"){
                return value;
            }
        }
};


#endif