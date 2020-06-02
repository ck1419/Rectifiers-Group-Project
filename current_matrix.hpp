#include "component.hpp"
#ifndef current_matrix_hpp
#define current_matrix_hpp
using namespace std;

//RECURSIVE FUNCTION FOR SUPERNODES, TBD
//RETURNS TOTAL OF VOLTAGES+CURRENT TO BE IN MAIN SUPERNODE NODE
float supernode(base_class* current_component, float cumulative_value){
    vector <base_class*> connected_components = current_component->return_nodes()[1]->return_components();
    //Scans through  all the components connected at the negative side of the source
    for (int i=0; i<connected_components.size(); i++){
        //Checks for voltage sources, and makes sure that it won't loop back to old components
        if (connected_components[i]->return_type() == 'V' && connected_components[i]->return_nodes()[0]->return_ID() == current_component->return_nodes()[1]->return_ID()){
            //Recursively sends the total of the voltages up
            cumulative_value = supernode(connected_components[i], cumulative_value);
        }
    }
    return cumulative_value;
}

vector<float> find_current(vector<base_class*> all_components, vector<node*> all_nodes, float t){
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
            if (all_components[i]->return_nodes()[1]->return_ID() != 0){
                current_matrix[ all_components[i]->return_nodes()[1]->return_ID()-1 ] += all_components[i]->return_value(t);
            }
        }

        //Keeps track of which nodes has voltage sources connected to it
        if (all_components[i]->return_type() == 'V'){
            //Checks for voltage sources that are connected to ground via negative side
            if (all_components[i]->return_nodes()[0]->return_ID() != 0 && all_components[i]->return_nodes()[1]->return_ID() == 0){
                temp_voltage_holder[ all_components[i]->return_nodes()[0]->return_ID()-1 ] = all_components[i]->return_value(t);
            //via positive side
            } else if (all_components[i]->return_nodes()[1]->return_ID() != 0 && all_components[i]->return_nodes()[0]->return_ID() == 0){
                temp_voltage_holder[ all_components[i]->return_nodes()[1]->return_ID()-1 ] = all_components[i]->return_value(t);
            //checks for top of supernode
            } else if (!all_components[i]->return_nodes()[0]->return_v_source_neg()){
                temp_voltage_holder[ all_components[i]->return_nodes()[0]->return_ID()-1 ] = supernode(all_components[i], 0);
            }
            //all other voltage sources are ignored
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
#endif