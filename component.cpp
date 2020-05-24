#include "component.hpp"
using namespace std;



vector<int> node::return_nodes(){
    return connected_nodesID;
}



void node::add_node(int ID){
    bool not_found = true;
    for(int i=0; i<connected_nodesID.size(); i++){
        if (connected_nodesID[i]==ID)
        {
            not_found = false;
        }
        if (not_found){
            connected_nodesID.push_back(ID);
        }
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