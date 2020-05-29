#include "component.hpp"
using namespace std;


vector<float> find_current(vector<base_class*> all_components, vector<node*> all_nodes, int t){
    //Creates current matrix filled with 0s
    vector<float> current_matrix, temp_voltage_holder;
    for (int i=0; i<all_nodes.size()-1;i++){
        current_matrix.push_back(0);
        temp_voltage_holder.push_back(0);
    }
    
    //Scans through every component looking for sources
    for (int i=0; i<all_components.size(); i++){
        //Add values to the current matrix
        if (all_components[i]->return_type() == 'I'){
            current_matrix[ all_components[i]->return_nodes()[0]->return_ID()-1 ] += all_components[i]->return_value(t);
        }
        //Keeps track of which nodes has voltage sources connected to it
        if (all_components[i]->return_type() == 'V'){
            temp_voltage_holder[ all_components[i]->return_nodes()[0]->return_ID()-1 ] += all_components[i]->return_value(t);
        }
    }
    
    //Replace the old current values with voltage values if connected to a voltage value
    for (int i=0; i<all_nodes.size(); i++){
        if (temp_voltage_holder[i] != 0){
            current_matrix[i] = temp_voltage_holder[i];
        }
    }
    return current_matrix;
}