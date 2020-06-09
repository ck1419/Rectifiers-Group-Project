#ifndef component_hpp
#define component_hpp

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
using namespace std;



//Declaration so base_class is usable in node
class base_class;

//Class for storing info about nodes
class node
{
    protected:
        vector<node*> connected_nodes;  //Pointer to connected nodes
        vector<base_class*> connected_components;   //Pointer to connected components
        int ID;
        bool v_source_neg = false;
        bool v_source_pos  = false;
        bool v_source_grounded = false;

    public:
        //Constructor
        node(int c_ID){
            ID = c_ID;      //ID = 0 for ground/reference node
        }

        //Returns connected nodes
        vector<node*> return_nodes();

        //Returns connected components
        vector<base_class*> return_components();

        //Returns ID of current node
        int return_ID();

        //Add connected nodes if it isn't included already
        void add_node(node *c_node);

        //Add base class connected to this component (For more efficient current calculations)
        void add_component(base_class *c_component);

        void set_v_source_true(string pos, bool ground_status);

        bool return_v_source_neg() {
            return v_source_neg;
        }

        bool return_v_source_pos() {
            return v_source_pos;
        } 

        bool return_v_source_grounded() {
            return v_source_grounded;
        }
};



//Base class just for inherited functionality so everything can use the same methods when iterating through arrays of components
class base_class
{
    protected:
        node* node1;
        node* node2;
        node* node3;    //Optional for BJTs
        string name;       
        float current, voltage, value, prev_cv, tot_acc;
        char type;
        bool node3_exists = false;
        bool current_found = false;

    public:
        //Returns the nodes the component is connected to in a vector
        virtual vector<node*> return_nodes();

        //Returns type of component
        virtual char return_type();

        //Returns name of the current component
        virtual string return_name();

        //Returns current if inductor, voltage if capacitor
        virtual float return_value(float t) =0;

        //Sets the current going through the component
        virtual void set_current(float i);

        //Returns current accross component
        virtual float return_current();

        //Returns 1 if current is set, 0 if current is still not found;
        virtual bool current_calculated();

	//Sets the previous value of current/voltage
	virtual void set_prev_cv(float cv) =0;

	//Adds onto the accumulator for caps and inductors
	virtual void set_tot_acc(float add_on);
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
	    prev_cv = 0;
	    tot_acc = 0;
        }

	//Sets the previous current/voltage depending on whether component is capacitor/inductor
	void set_prev_cv(float cv);

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

	void set_prev_cv(float cv);

        //Placeholder for variable output depending on source type
        float return_value(float t);
};



class nonlinear_component: public base_class
{
    protected:
        string model;
        //variables for newton-rhapson method
	base_class* Ieq;
	base_class* Req;
	float Isat;
	float Vtemp;
    public:
        //Constructor for diodes
        nonlinear_component(char c_type, node *c_node1, node *c_node2, string c_name, string c_model){
            type = c_type;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
            model = c_model;
	    Isat = 0;
	    Vtemp = 0;
	    prev_cv = 0;
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
	void set_prev_cv(float cv);

        //Returns approximate linear approximation via newton-rhapson method (ADD TRANSISTOR FUNCTIONALITY)
};



#endif
