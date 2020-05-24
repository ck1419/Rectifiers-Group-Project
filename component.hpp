#ifndef component_hpp
#define component_hpp

#include <vector>
#include <string>
#include <cmath>
using namespace std;


class node
{
    protected:
        vector<int> connected_nodesID;        //Linked via int IDs
        int ID;

    public:
        //Constructor
        node(string c_ID, int ID1, int ID2){
            ID = stoi( c_ID.erase('N') );
            connected_nodesID.push_back(ID1);
            connected_nodesID.push_back(ID2);
        }

        //Returns connected nodes
        vector<int> return_nodes();

        //Add connected nodes if it isn't included already
        void add_node(int ID);
};

//Base class just for inherited functionality so everything can use the same methods when iterating through arrays of components
class base_class
{
    protected:
        node* node1;
        node* node2;
        node* node3;    //Optional for BJTs
        string name;       
        float current, voltage;
        char type;
        float value;
        bool node3_exists = false;

    public:
        //Returns the nodes the component is connected to in a vector
        virtual vector<node*> return_nodes();

        //Returns type of component
        virtual char return_type();
};



//Class for RCL components
class basic_component: public base_class
{
    public:
        //Constructor
        basic_component(char c_type, float c_value, node *c_node1, node *c_node2, string c_name){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
        }

        //Returns value of component with respects to time
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

        //Placeholder for variable output depending on source type
        float return_value(float t);
};



class nonlinear_component: public base_class
{
    protected:
        string model;
        //variables for newton-rhapson method
        float old_value;
    
    public:
        //Constructor for diodes
        nonlinear_component(char c_type, float c_value, node *c_node1, node *c_node2, string c_name, string c_model){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
            model = c_model;
        }

        //Constructor for transistors
        nonlinear_component(char c_type, float c_value, node *c_node1, node *c_node2, node *c_node3, string c_name, string c_model){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
            node3 = c_node3;
            name = c_name;
            model = c_model;
            node3_exists = true;
        }

        //Returns value of previous iteration;
        float return_old_value();

        //Returns approximate linear approximation via newton-rhapson method (ADD TRANSISTOR FUNCTIONALITY)
        float return_value(float v_old, float R, float VA);
};



#endif