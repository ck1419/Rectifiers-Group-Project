#include "component.hpp"
using namespace std;



vector<string>  base_class::return_nodes(){
    vector<string> temp;
    temp.push_back(node1);
    temp.push_back(node2);
    if (!node3.empty()){
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



float nonlinear_component::return_value(float v){
    if (model == "D"){
        return 1;
    }
    if (model == "NPN"){
        return 1;
    }
    if (model == "PNP"){
        return 1;
    }
}