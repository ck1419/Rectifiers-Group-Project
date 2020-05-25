#include "component.hpp"
using namespace std;

vector<float> find_current(vector<node*> all_nodes, int t){
    vector<float> current_matrix;
    for (int i=0; i<all_nodes.size();i++){
        current_matrix.push_back(0);
    }

    for (int i=0; i<all_nodes.size();i++){
        float current = 0;
        vector<base_class*> connected_components = all_nodes[i]->return_components();
        for (int x=0; x<connected_components.size(); x++){
            if (connected_components[x]->return_type() == 'I'){
                if (connected_components[x]->return_nodes()[0]->return_ID() == all_nodes[i]->return_ID()){
                    current += connected_components[x]->return_value(t);
                }else{
                    current -= connected_components[x]->return_value(t);
                }
                
            }
        }
        current_matrix[all_nodes[i]->return_ID()] = current;
    }

    return current_matrix;
}