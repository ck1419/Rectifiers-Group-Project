#ifndef component_hpp
#define component_hpp

#include <vector>
#include <string>
#include <cmath>
using namespace std;



//Base class just for inherited functionality so everything can use the same methods when iterating through arrays of components
class base_class
{
    protected:
        string node1, node2, node3, name;
        char type;
        float value;

    public:
        //Returns the nodes the component is connected to in a vector
        virtual vector<string> return_nodes();

        //Returns type of component
        virtual char return_type();

        //Returns the value of the component that depends on time
        virtual float return_value(float t) =0;
        
        //Returns value for non-linear components that requires approximation using voltage across component (v)
        virtual float return_value(float v) =0;
};



//Class for RCL components
class basic_component: public base_class
{
    public:
        //Constructor
        basic_component(char c_type, float c_value, string c_node1, string c_node2, string c_name){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
        }

        //Returns value of component
        float return_value(float t);
};



//Class for IV sources
class source: public base_class
{
    protected:
        string output_type;
        float frequency, amplitude;

    public:
        //Constructor
        source(char c_type, string c_output_type, string c_node1, string c_node2, string c_name, float c_frequency, float c_value, float c_amplitude){
            type = c_type;
            output_type = c_output_type;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
            frequency = c_frequency;
            value = c_value;
            amplitude = c_amplitude;
        }

        //Placeholder for variable output depending on source type
        float return_value(float t);
};



class nonlinear_component: public base_class
{
    protected:
        string model;
        //variables for newton-rhapson method
        int iteration;
        float old_value;
    
    public:
        //Constructor for diodes
        nonlinear_component(char c_type, float c_value, string c_node1, string c_node2, string c_name, string c_model){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
            model = c_model;
        }

        //Constructor for transistors
        nonlinear_component(char c_type, float c_value, string c_node1, string c_node2, string c_node3, string c_name, string c_model){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            node3 = c_node3;
            name = c_name;
            model = c_model;
        }

        //Returns approximate linear approximation via newton-rhapson method (TODO)
        float return_value(float v);
};



#endif