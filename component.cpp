#include <vector>
#include <string>
using namespace std;

class component
{
    private:
        string type, node1, node2;        //R, C, L
        float value;

    public:
        //Constructor
        component(string c_type, float c_value, string c_node1, string c_node2){
            type = c_type;
            value = c_value;
            node1 = c_node1;
            node2 = c_node2;
        }

        //Returns nodes the component is connected to
        vector<string>  return_nodes(){
            vector<string> temp;
            temp.push_back(node1);
            temp.push_back(node2);
            return temp;
        }

        //Returns value of component
        float return_value(){
            return value;
        }

        //Returns type of component
        string return_type(){
            return type;
        }
};
