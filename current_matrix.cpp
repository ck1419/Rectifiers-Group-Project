#include "component.hpp"
using namespace std;


vector<float> find_current(vector<base_class*> all_components, vector<node*> all_nodes, int t){
    //Creates current matrix filled with 0s
    vector<float> current_matrix;
    for (int i=0; i<all_nodes.size();i++){
        current_matrix.push_back(0);
    }
    //Replace 0s with current values if node is connected to a source
    for (int i=0; i<all_components.size(); i++){
        if (all_components[i].type == "I"){
            current_matrix[ all_components[i]->return_nodes()[0]->return_ID() ] += all_components[i]->return_value(t);
        }
    }
    return current_matrix;
}