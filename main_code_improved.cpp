#include "base_class.hpp"
#include "source.hpp"
#include "basic_component.hpp"
#include "nonlinear_component.hpp"
#include "node.hpp"
#include "current_matrix_improved.hpp"
#include "scientific_converter.hpp"
#include "add_nodes_to_vector.hpp"
#include <iostream>
#include <sstream>
#include <Eigen/Dense>
using Eigen::MatrixXd;


int main()
{
    //VARIABLE DECLARATIONS
    vector<node*> node_vector;      //keep track of nodes and their connected components
    vector<base_class*> components; //stores all the components in the circuit, used for easy outputs
    vector<basic_component*> resistor_vector, capacitor_vector, inductor_vector;
    vector<source*> current_vector, voltage_vector;
    vector<nonlinear_component*> diode_vector;
    string node1, node2, name, temp_value, output_type, input, source_type;
    char type;
    double value;
    double stop_time = 0;
    double time_step = 1;
    int component_counter = 0; //component counter for figuring out current number of component
    bool diode_checker = 0;
    bool looper = 1;
    bool final_loop = 0;
    bool cond = 1;
    bool oc_looper = 1;


    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input)){
        istringstream ss(input);
        double frequency = 0;
        double amplitude = 0;
        ss >> name;
        if (name[0]=='*'){
            continue;
        }
        if (name[0]=='.'){          //Checks for end of file (.tran 0 <stop time> 0 <timestep>) followed by (.end)
            ss >> temp_value >> temp_value;       //Removes dummy 0 in .trans
            stop_time = scientific_converter(temp_value);
            ss >> temp_value >> temp_value;       //Removes dummy 0 after 
            time_step = scientific_converter(temp_value);
            break;                  //Stops loop to avoid reading .end creating run time errors
        } else {                    //Parser for RLC and sources
            ss >> node1;
            ss >> node2;
            ss >> temp_value;
            if (tolower(temp_value[0])=='s'){       //Checks for sine sources   SINE(<dc offset> <amplitude> <frequency>)
                source_type = "sine";
                value = scientific_converter(temp_value.substr(5));     //Removes ' SINE( '
                ss >> temp_value;
                amplitude = scientific_converter(temp_value);
                ss >> temp_value;
                temp_value.pop_back();              //Removes )
                frequency = scientific_converter(temp_value);
            }else{                                  //Not a sine source
                source_type = "dc";
                if (temp_value != "D"){
                    value = scientific_converter(temp_value);
                }      
            }
        }


        //creating dynamic storage for the nodes that have been inputted
        node *truenode1;
        node *truenode2;
        if (node1[0] == 'N'){
            truenode1 = new node(stoi(node1.substr(1)));
        }else{
            truenode1 = new node(stoi(node1));
        }

        if (node2[0] == 'N'){
            truenode2 = new node(stoi(node2.substr(1)));
        }else{
            truenode2 = new node(stoi(node2));
        }


        //Add components/sources to component vector (WITH SINE SOURCE SUPPORT)
        if (name[0] == 'R'){
            resistor_vector.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
            components.push_back(resistor_vector[resistor_vector.size()-1]);
        } else if (name[0] == 'C'){
            capacitor_vector.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
            components.push_back(capacitor_vector[capacitor_vector.size()-1]);
        } else if (name[0] == 'L'){
            inductor_vector.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
            components.push_back(inductor_vector[inductor_vector.size()-1]);
        } else if (name[0] == 'I'){
            current_vector.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
            components.push_back(current_vector[current_vector.size()-1]);
        } else if (name[0] == 'V'){
            voltage_vector.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
            components.push_back(voltage_vector[voltage_vector.size()-1]);
        } else if (name[0] == 'D'){
	        diode_vector.push_back(new nonlinear_component(input[0], truenode2, truenode1, name, "ideal_diode"));
            components.push_back(diode_vector[diode_vector.size()-1]);
            diode_checker=1;
	        oc_looper=1;
        }


        //Add nodes to the node vector
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector);
        component_counter += 1;
        input.clear();
    }


    //CREATES NORMAL CONDUCTANCE MATRIX, AND OPEN CIRCUIT CONDUCTANCE MATRIX
    const int h = node_vector.size()+voltage_vector.size()+capacitor_vector.size()+inductor_vector.size()-1;
    const int w = node_vector.size()+voltage_vector.size()+inductor_vector.size()-1;
    MatrixXd g(h, h);
    MatrixXd goc(w, w);


    int voltage_source_counter = voltage_vector.size();
    int capacitor_counter = capacitor_vector.size();
    int inductor_counter = inductor_vector.size();
    int diode_counter = diode_vector.size();


    //TRANSIENT SIM OUTPUT HEADER
    cout << "time";
    for(int b=0; b<node_vector.size();b++){
	if(node_vector[b]->return_ID()!=0){
	    cout << '\t' << "V(" << node_vector[b]->return_ID() << ")";
    	}
    }
    for(int c=0; c<components.size();c++){
        cout << '\t' << "I(" << components[c]->return_name() << ")";
    }
    cout << endl;


    //RUN TRANSIENT SIM
    for (long double t=0; t<=stop_time; t+=time_step){
        while (looper){
            if(oc_looper){
                oc_looper = 0;

                //SET Goc MATRIX TO 0
                for (int i=0; i<w; i++){
                    for(int j=0; j<w; j++){
                        goc(i,j) = 0;
                    }
                }


                //FILLS IN Goc MATRIX WITH RESISTORS
                for (int i=0; i<resistor_vector.size(); i++){
                    int node1_ID = resistor_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = resistor_vector[i]->return_nodes()[1]->return_ID()-1;
                    if (node1_ID!=-1){
                        goc(node1_ID, node1_ID) += 1/resistor_vector[i]->return_value(0, 0);
                    }
                    if (node2_ID!=-1){
                        goc(node2_ID, node2_ID) += 1/resistor_vector[i]->return_value(0, 0);
                    }
                    if (node1_ID!=-1 && node2_ID!=-1){
                        goc(node1_ID, node2_ID) -= 1/resistor_vector[i]->return_value(0, 0);
                        goc(node2_ID, node1_ID) -= 1/resistor_vector[i]->return_value(0, 0);
                    }
                }
                //FILLS IN Goc MATRIX WITH VOLTAGE SOURCES
                for (int i=0; i<voltage_vector.size(); i++){
                    int node1_ID = voltage_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = voltage_vector[i]->return_nodes()[1]->return_ID()-1;
                    int position = stoi(voltage_vector[i]->return_name().substr(1))+node_vector.size()-2;
                    if (node1_ID!=-1){
                        goc(position, node1_ID) = 1;
                        goc(node1_ID, position) = 1;
                    }
                    if (node2_ID!=-1){
                        goc(position, node2_ID) = -1;
                        goc(node2_ID, position) = -1;
                    }
                }
                //FILLS IN Goc MATRIX WITH INDUCTORS
                for (int i=0; i<inductor_vector.size(); i++){
                    int node1_ID = inductor_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = inductor_vector[i]->return_nodes()[1]->return_ID()-1;
                    int position = stoi(inductor_vector[i]->return_name().substr(1))+node_vector.size()+voltage_vector.size()-2;
                    if (node1_ID!=-1){
                        goc(position, node1_ID) = 1;
                        goc(node1_ID, position) = 1;
                    }
                    if (node2_ID!=-1){
                        goc(position, node2_ID) = -1;
                        goc(node2_ID, position) = -1;
                    }
                }


                //CALCULATE CURRENT MATRIX FOR OPEN CIRCUIT
                MatrixXd currentoc(w,1);
                vector<double> temp = find_currentoc(current_vector, voltage_vector, node_vector.size()-1, inductor_vector.size(), t);
                for(int f=0; f < temp.size(); f++){
                    currentoc(f, 0) = temp[f];
                }


                //CALCULATE VOLTAGE MATRIX FOR OC
                MatrixXd voc(w,1);
                voc = goc.fullPivLu().solve(currentoc);


                //SETTING DIODE INITIAL VD
                for (int i=0; i<components.size(); i++){
                    if (components[i]->return_type() == 'D'){
                        double diode_v1 = 0;
                        double diode_v2 = 0;
                        if (components[i]->return_nodes()[0]->return_ID() != 0){
                            diode_v1 = voc((components[i]->return_nodes()[0]->return_ID()-1) , 0);
                        }
                        if (components[i]->return_nodes()[1]->return_ID() != 0){
                            diode_v2 = voc((components[i]->return_nodes()[1]->return_ID()-1) , 0);
                        }
                        if(diode_v2-diode_v1 < 0.7){
                            components[i]->set_prev_cv(diode_v2-diode_v1);
                        }else{
                            components[i]->set_prev_cv(0.7);
                        }
                    }
                }
		    }


            if(cond){
                final_loop = 0;

                //SET G MATRIX TO 0
                for (int i=0; i<h; i++){
                    for(int j=0; j<h; j++){
                        g(i,j) = 0;
                    }
                }


                //////////////////////////////////////////////////////////////////////////

                //CREATES G MATRIX BY GOING THROUGH EACH COMPONENT AND ADDING IT TO THE MATRIX
                for (int i=0; i<resistor_vector.size(); i++){
                    int node1_ID = resistor_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = resistor_vector[i]->return_nodes()[1]->return_ID()-1;
                    if (node1_ID!=-1){
                        g(node1_ID, node1_ID) += 1/resistor_vector[i]->return_value(0, 0);
                    }
                    if (node2_ID!=-1){
                        g(node2_ID, node2_ID) += 1/resistor_vector[i]->return_value(0, 0);
                    }
                    if (node1_ID!=-1 && node2_ID!=-1){
                        g(node1_ID, node2_ID) -= 1/resistor_vector[i]->return_value(0, 0);
                        g(node2_ID, node1_ID) -= 1/resistor_vector[i]->return_value(0, 0);
                    }
                }
                for (int i=0; i<diode_vector.size(); i++){
                    int node1_ID = diode_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = diode_vector[i]->return_nodes()[1]->return_ID()-1;
                    if (node1_ID!=-1){
                        g(node1_ID, node1_ID) += 1/diode_vector[i]->return_Req()->return_value(0, 0);
                    }
                    if (node2_ID!=-1){
                        g(node2_ID, node2_ID) += 1/diode_vector[i]->return_Req()->return_value(0, 0);
                    }
                    if (node1_ID!=-1 && node2_ID!=-1){
                        g(node1_ID, node2_ID) -= 1/diode_vector[i]->return_Req()->return_value(0, 0);
                        g(node2_ID, node1_ID) -= 1/diode_vector[i]->return_Req()->return_value(0, 0);
                    }
                }    
                for (int i=0; i<voltage_vector.size(); i++){
                    int node1_ID = voltage_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = voltage_vector[i]->return_nodes()[1]->return_ID()-1;
                    double position = stoi(voltage_vector[i]->return_name().substr(1))+node_vector.size()-2;
                    if (node1_ID!=-1){
                        g(position, node1_ID) = 1;
                        g(node1_ID, position) = 1;
                    }
                    if (node2_ID!=-1){
                        g(position, node2_ID) = -1;
                        g(node2_ID, position) = -1;
                    }                   
                }
                for (int i=0; i<capacitor_vector.size(); i++){
                    int node1_ID = capacitor_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = capacitor_vector[i]->return_nodes()[1]->return_ID()-1;
                    double position = stoi(capacitor_vector[i]->return_name().substr(1))+node_vector.size()+voltage_vector.size()-2;
                    if (node1_ID!=-1){
                        g(position, node1_ID) = 1;
                        g(node1_ID, position) = 1;
                    }
                    if (node2_ID!=-1){
                        g(position, node2_ID) = -1;
                        g(node2_ID, position) = -1;
                    }
                }
                for (int i=0; i<inductor_vector.size(); i++){
                    int node1_ID = inductor_vector[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = inductor_vector[i]->return_nodes()[1]->return_ID()-1;
                    double position = stoi(inductor_vector[i]->return_name().substr(1))+node_vector.size()-2+voltage_source_counter+capacitor_counter;
                    g(position, position) = 1;
                    if(node1_ID!=-1){
                        g(position, node1_ID) = -time_step/(inductor_vector[i]->return_value(0, 0));
                        g(node1_ID, position) = -1;
                    }
                    if(node2_ID!=-1){
                        g(position, node2_ID) = time_step/(inductor_vector[i]->return_value(0, 0));
                        g(node2_ID, position) = 1;
                    }
                }


                //////////////////////////////////////////////////////////////////////////


                if(!diode_checker){
                    cond = 0;
                    final_loop = 1;
                }else{
                    diode_counter++;
                }
                if(diode_counter == 10){
                    final_loop = 1;
		            oc_looper = 1;
                }
            }


            //CALCULATE CURRENT MATRIX
            MatrixXd current(h,1);
            vector<double> temp = find_current(inductor_vector, capacitor_vector, voltage_vector, current_vector, diode_vector, t, time_step, final_loop, node_vector.size()-1);
            for(int f=0; f < temp.size(); f++){
                current(f, 0) = temp[f];
            }


            //CALCULATE VOLTAGE MATRIX
            MatrixXd v(h,1);
            v = g.fullPivLu().solve(current);


            //TO MAKE SURE THAT VALUES DONT UPDATE/OUTPUT DURING NEWTON-RAPHSON ITERATION UNLESS IT'S THE FINAL LOOP
            if (final_loop){
                for (int i=0; i<capacitor_vector.size(); i++){
                    if (capacitor_vector[i]->return_nodes()[0]->return_ID() != 0){
                        double cap_current = v((stoi(capacitor_vector[i]->return_name().substr(1))+node_vector.size()+voltage_source_counter-2), 0);
                        capacitor_vector[i]->set_prev_cv(cap_current);
                    }
                }
                for (int i=0; i<inductor_vector.size(); i++){
                    if (inductor_vector[i]->return_nodes()[0]->return_ID() != 0){
                        double ind_current = v((stoi(inductor_vector[i]->return_name().substr(1))+node_vector.size()+voltage_source_counter+capacitor_counter-2), 0);
                        inductor_vector[i]->set_tot_acc(ind_current);
		            }
                }
                for (int i=0; i<diode_vector.size(); i++){
                    double diode_v1 = 0;
                    double diode_v2 = 0;
                    if (diode_vector[i]->return_nodes()[0]->return_ID() != 0){
                        diode_v1 = v((diode_vector[i]->return_nodes()[0]->return_ID()-1) , 0);
                    }
                    if (diode_vector[i]->return_nodes()[1]->return_ID() != 0){
                        diode_v2 = v((diode_vector[i]->return_nodes()[1]->return_ID()-1) , 0);
                    }
                    diode_vector[i]->set_prev_cv(diode_v2-diode_v1);
                }

                //OUTPUTS NODAL VOLTAGE EACH TIME STEP
                cout << t;
                for(int b=0; b<node_vector.size();b++){
                    if(node_vector[b]->return_ID()!=0){
                        cout << '\t' << v((node_vector[b]->return_ID()-1),0);
                    }
                }
	        }

            for (int s=0; s<components.size(); s++){
                ///////////OUTPUTTING CAPACITOR CURRENT//////////////
                if (components[s]->return_type()=='C' && final_loop){
                    cout << '\t' << components[s]->return_prev_cv();

                ///////////OUTPUTTING INDUCTOR CURRENT///////////////
                }else if (components[s]->return_type() == 'L' && final_loop){
                    cout << '\t' << components[s]->return_tot_acc();

                ///////////OUTPUTTING RESISTOR CURRENT///////////////
                }else if (components[s]->return_type() == 'R' && final_loop){
                    double resistor_voltage1 = 0;
                    double resistor_voltage2 = 0;
                    if (components[s]->return_nodes()[1]->return_ID()-1 != -1){
                        resistor_voltage1 = v((components[s]->return_nodes()[1]->return_ID()-1), 0);
                    }
                    if (components[s]->return_nodes()[0]->return_ID()-1 != -1){
                        resistor_voltage2 = v((components[s]->return_nodes()[0]->return_ID()-1), 0);
       		        }
                    double resistor_voltage = resistor_voltage1-resistor_voltage2;
                    cout << '\t' << resistor_voltage * 1/(components[s]->return_value(t, final_loop));

                ///////////OUTPUTTING CURRENT SOURCE CURRENT///////////////
                }else if (components[s]->return_type() == 'I' && final_loop){
                    cout << '\t' << components[s]->return_value(t, final_loop);

                ///////////OUTPUTTING VOLTAGE SOURCE CURRENT///////////////
                }else if (components[s]->return_type() == 'V' && final_loop){
                    cout << '\t' << v(stoi(components[s]->return_name().substr(1)) + node_vector.size()-2, 0);

		        ///////////OUTPUTTING DIODE CURRENT///////////////////
		        }else if (components[s]->return_type() == 'D' && final_loop){
                    double resistor_voltage1 = 0;
                    double resistor_voltage2 = 0;
                    if (components[s]->return_nodes()[1]->return_ID()-1 != -1){
                        resistor_voltage1 = v((components[s]->return_nodes()[1]->return_ID()-1), 0);
                    }
                    if (components[s]->return_nodes()[0]->return_ID()-1 != -1){
                        resistor_voltage2 = v((components[s]->return_nodes()[0]->return_ID()-1), 0);
                    }
                    double resistor_voltage = resistor_voltage1-resistor_voltage2;
                    cout << '\t' << resistor_voltage * 1/(components[s]->return_Req()->return_value(t, final_loop)) + components[s]->return_Ieq()->return_value(t, final_loop);

		        }
            }
            

            if(final_loop){
                cout << endl;
                diode_counter = 0;
                break;
            }
	    }
    }
}
