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
        cerr << "v_source_neg = " << v_source_neg << endl;
        cerr << "v_source_grounded = " << v_source_grounded << endl;
    } else if (pos == "pos") {
        v_source_pos = true;
        v_source_grounded = ground_status;
        cerr << "v_source_pos = " << v_source_pos << endl;
        cerr << "v_source_grounded = " << v_source_grounded << endl;
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

void base_class::set_tot_acc(float add_on){
    tot_acc += add_on;
}

void basic_component::set_prev_cv(float cv){
    prev_cv = cv;
}

float basic_component::return_value(float t){
    if(type == 'R'){
    return value;
    } else if(type == 'C'){
        float charge = prev_cv * t;
        set_tot_acc(charge);
        return tot_acc/value;
    } else if(type == 'L'){
	    float dt = prev_cv * t;
        set_tot_acc(dt);
	    return dt/value;
    }
}



float source::return_value(float t){
    if (output_type == "sine"){
        return ( amplitude*sin(2*M_PI*frequency*t) ) + value;
    }
    if (output_type == "dc"){
        return value;
    }
}

void source::set_prev_cv(float cv){
    prev_cv = cv;
}

void nonlinear_component::set_prev_cv(float cv){
    if (model == "D"){
    prev_cv = cv;
    Isat = 3e-9;         //Placeholder value
    Vtemp = 0.025;        //Placeholder value
    float I_component = Isat * ( pow(M_E, prev_cv/Vtemp)-1 );
    Req = new basic_component('R', 1/(I_component/Vtemp), node1, node2, "Req");
    Ieq = new source('I', "dc" , node1, node2, "Ieq", 0, (I_component - (I_component/Vtemp)*prev_cv), 0);
    }
    if (model == "NPN"){
	//not implemented
    }
    if (model == "PNP"){
	//not implemented
    }

}
