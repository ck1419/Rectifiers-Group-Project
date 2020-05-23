#include <vector>
#include <string>
using namespace std;


//Base class just for inherited functionality so everything can use the same methods when iterating through arrays of components
class base_class
{
    protected:
        string node1, node2, name; 
        char type;
        float value;

    public:
        virtual vector<string>  return_nodes(){
            vector<string> temp;
            temp.push_back(node1);
            temp.push_back(node2);
            return temp;
        }

        //Returns type of component
        virtual char return_type(){
            return type;
        }

        virtual float return_value(float t) =0;
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

        //Returns nodes the component is connected to
        vector<string>  return_nodes(){
            vector<string> temp;
            temp.push_back(node1);
            temp.push_back(node2);
            return temp;
        }

        //Returns value of component
        float return_value(float t){
            return value;
        }


};



//Class for IV sources
class source: public base_class
{
    protected:
        string output_type;
        float frequency;

    public:
        //Constructor
        source(char c_type, string c_output_type, string c_node1, string c_node2, string c_name){
            type = c_type;
            output_type = c_output_type;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
        }

        //Placeholder for variable output depending on source type
        float return_value(float t){
            if (output_type == "triangle"){
                return 1;
            }
            if (output_type == "sine"){
                return 1;
            }
            if (output_type == "square"){
                return 1;
            }
            if (output_type == "DC"){
                return value;
            }
        }
};