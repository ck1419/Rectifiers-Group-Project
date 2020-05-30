#include "component.hpp"
using namespace std;



vector<node*> node::return_nodes(){
    return connected_nodes;
}



vector<base_class*> node::return_components(){
    return connected_components;
}



int node::return_ID(){
    return ID;
}

void node::set_v_source_true(string pos, bool ground_status) {
    if (pos == "neg") {
        v_source_neg = true;
        v_source_grounded = ground_status;
    } else if (pos == "pos") {
        v_source_pos = true;
        v_source_grounded = ground_status;
    } else {
        //
    }
}



void node::add_node(node *c_node){
    bool not_found = true;
    for(int i=0; i<connected_nodes.size(); i++){
        if (connected_nodes[i]->return_ID() == c_node->return_ID())
        {
            not_found = false;
        }
    }
    if (not_found){
        connected_nodes.push_back(c_node);
    }
}



void node::add_component(base_class *c_component){
    bool not_found = true;
    for(int i=0; i<connected_components.size(); i++){
        if (connected_components[i]->return_name() == c_component->return_name())
        {
            not_found = false;
        }
    }
    if (not_found){
        connected_components.push_back(c_component);
    }
}



vector<node*> base_class::return_nodes(){
    vector<node*> temp;
    temp.push_back(node1);
    temp.push_back(node2);
    if (node3_exists){
        temp.push_back(node3);
    }
    return temp;
}



char base_class::return_type(){
    return type;
}



string base_class::return_name(){
    return name;
}



void base_class::set_current(float i){
    current = i;
    current_found = true;
}



float base_class::return_current(){
    return current;
}



bool base_class::current_calculated(){
    return current_found;
}



float basic_component::return_value(float t){
    return value;
}



float source::return_value(float t){
    if (output_type == "sine"){
        return ( amplitude*sin(2*M_PI*frequency*t) ) + value;
    }
    if (output_type == "triangle"){
        return 1;
    }
    if (output_type == "square"){
        return 1;
    }
    if (output_type == "DC"){
        return value;
    }
}



float nonlinear_component::return_value(float v_old, float R, float VA){
    /*
    SOURCE: http://www.kennethkuhn.com/students/ee351/diode_solution.pdf
    Shockley's ideal diode equation for newton-rahpson iterative method      
            IS * (exp(VD/nVT) - 1) - VA/R + VD/R
    V_new = -------------------------------------
                (IS/nVT) * exp(VD/nVT) + 1/R
    Is = Reverse saturation current of diode
    VD (v_old) = Voltage across diode
    nVT = Thermal voltage
    VA = Applied voltage
    R = Resistance
    V_new = New voltage across diode
    */
    if (model == "D"){
        float old_value = v_old;

        float Is = 3e-9;         //Placeholder value
        float nVt = 0.05;        //Placeholder value
        float top = Is * ( pow(M_E, v_old/nVt)-1 ) - (VA/R) + (v_old/R);
        float bottom = (Is/nVt) * pow(M_E, v_old/nVt) + (1/R);
        float v_new = top/bottom;

        return v_new;
    }
    if (model == "NPN"){
        return 1;
    }
    if (model == "PNP"){
        return 1;
    }
}



float nonlinear_component::return_old_value(){
    return old_value;
}