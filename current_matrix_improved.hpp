#ifndef current_matrix_hpp
#define current_matrix_hpp


#include "base_class.hpp"
#include "basic_component.hpp"
#include "node.hpp"
#include "source.hpp"
#include "nonlinear_component.hpp"


//FUNCTION TO FIND THE CURRENT MATRIX
vector<double> find_current(vector<basic_component*> inductor_vector, vector<basic_component*> capacitor_vector, vector<source*> voltage_vector, vector<source*> current_vector, vector<nonlinear_component*> diode_vector, double t, double time_step, bool final_loop_checker, int matrix_base_size){
    //Variables to store data that will be acessed many times for efficiency
    int voltage_counter = voltage_vector.size();
    int cap_counter = capacitor_vector.size();
    int ind_counter = inductor_vector.size();
    //Creates empty current matrix to be fileld in
    vector<double> current_matrix;
    for (int i=0; i<matrix_base_size+voltage_counter+cap_counter+ind_counter; i++){
        current_matrix.push_back(0);
    }
    //Add current source values to matrix
    for (int i=0; i<current_vector.size(); i++){
        if (current_vector[i]->return_nodes()[1]->return_ID() != 0){
            current_matrix[ current_vector[i]->return_nodes()[1]->return_ID()-1 ] += current_vector[i]->return_value(t, 0);
        }
        if (current_vector[i]->return_nodes()[0]->return_ID() != 0){
            current_matrix[ current_vector[i]->return_nodes()[0]->return_ID()-1 ] -= current_vector[i]->return_value(t, 0);
        }
    }
    //Add diode values to matrix
    for (int i=0; i<diode_vector.size(); i++){
        if (diode_vector[i]->return_Ieq()->return_nodes()[1]->return_ID() != 0){
            current_matrix[ diode_vector[i]->return_Ieq()->return_nodes()[1]->return_ID()-1 ] += diode_vector[i]->return_Ieq()->return_value(t, 0);
        }
        if (diode_vector[i]->return_Ieq()->return_nodes()[0]->return_ID() != 0){
            current_matrix[ diode_vector[i]->return_Ieq()->return_nodes()[0]->return_ID()-1 ] -= diode_vector[i]->return_Ieq()->return_value(t, 0);
        }
    }
    //Add inductor values to matrix
    for (int i=0; i<inductor_vector.size(); i++){
        if (inductor_vector[i]->return_nodes()[0]->return_ID() != 0){
            current_matrix[ stoi(inductor_vector[i]->return_name().substr(1)) + matrix_base_size+voltage_counter+cap_counter -1] = inductor_vector[i]->return_tot_acc();
        }
    }
    //Add voltage values to the matrix
    for (int i=0; i<voltage_vector.size(); i++){
        if (voltage_vector[i]->return_nodes()[0]->return_ID() != 0){
            current_matrix[ stoi(voltage_vector[i]->return_name().substr(1)) + matrix_base_size -1] = voltage_vector[i]->return_value(t, 0);
        }
    }
    //Add capacitor values to the matrix
    for (int i=0; i<capacitor_vector.size(); i++){
        if (capacitor_vector[i]->return_nodes()[0]->return_ID() != 0){
            current_matrix[ stoi(capacitor_vector[i]->return_name().substr(1)) + matrix_base_size+voltage_counter -1] = capacitor_vector[i]->return_value(time_step, final_loop_checker);
        }
    }
    return current_matrix;
}




//FUNCTION TO FIND THE CURRENT MATRIX FOR OPEN-CIRCUIT CIRCUITS
vector<double> find_currentoc(vector<source*> current_vector, vector<source*> voltage_vector, int matrix_base_size, int inductor_count, double t){
    //Creates empty current matrix to be filled in
    vector<double> current_matrix;
    for (int i=0; i<matrix_base_size+voltage_vector.size()+inductor_count; i++){
        current_matrix.push_back(0);
    }
    //Add values to the current matrix
    for (int i=0; i<current_vector.size(); i++){
        if (current_vector[i]->return_nodes()[1]->return_ID() != 0){
            current_matrix[ current_vector[i]->return_nodes()[1]->return_ID()-1 ] += current_vector[i]->return_value(t, 0);
        }
        if (current_vector[i]->return_nodes()[0]->return_ID() != 0){
            current_matrix[ current_vector[i]->return_nodes()[0]->return_ID()-1 ] -= current_vector[i]->return_value(t, 0);
        }
    }
    //Add voltage values to the matrix
    for (int i=0; i<voltage_vector.size(); i++){
        if (voltage_vector[i]->return_nodes()[0]->return_ID() != 0){
            current_matrix[ stoi(voltage_vector[i]->return_name().substr(1)) + matrix_base_size -1] = voltage_vector[i]->return_value(t, 0);
        }
    }
    return current_matrix;
}


#endif
