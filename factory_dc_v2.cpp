#include "base_class.hpp"
#include "source.hpp"
#include "basic_component.hpp"
#include "nonlinear_component.hpp"
#include "node.hpp"
#include "current_matrix_v2.hpp"
#include "scientific_converter.hpp"
#include "add_nodes_to_vector.hpp"
#include <iostream>
#include <sstream>
#include <Eigen/Dense>
using Eigen::MatrixXd;


/*
//////////ALGORITHM for circuits with only resistors and current sources//////////

done 1) Take in components and sources from netlist (current sources and voltage sources work in a totally different way tho)
2) Determine the number of nodes (v_num)

4) Construct G Matrix
    4.1) The diagonal is always positive: sum of all conductances connected to node
    4.2) Top triangular always negative: sum of all conductances in between 2 nodes.
    4.3) Lower triangular is identical (reflect values across diagonal)
5) V matrix implicit - it's the result
6) I matrix - value is net current into node.

*/


int main(int argc, char** argv)
{
    string calc_node = argv[1];


    //NEW VARIABLE DECLARATIONS (MERGED WITH OLD ONES)
    vector<node*> node_vector;      //keep track of nodes and their connected components
    vector<base_class*> components; //stores all the components in the circuit
    string node1, node2, name, temp_value, output_type, input, source_type;
    char type;
    float value;
    float stop_time = 0;
    float time_step = 1;
    int component_counter = 0; //component counter for figuring out current number of component
    int voltage_source_counter = 0;
    int capacitor_counter = 0;
    int diode_checker = 0;
    float prev_diode_vd = 0;
    bool looper = 1;
    bool final_loop = 0;
    bool cond = 1;


    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input)){

        float frequency = 0;
        float amplitude = 0;

        istringstream ss(input);
        ss >> name;
        if (name[0]=='.'){          //Checks for end of file (.tran 0 <stop time> 0 <timestep>) followed by (.end)
            ss >> temp_value;       //Removes dummy 0 in .trans
            ss >> temp_value;
            stop_time = scientific_converter(temp_value);
            ss >> temp_value;       //Removes dummy 0 after 
            ss >> temp_value;
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
                value = scientific_converter(temp_value);
            }
        }


        //creating conditionals for parsing in the various components from the netlist
        bool is_component = (name[0] == 'R' || name[0] == 'C' || name[0] == 'L');                //support for C and L comes later
        bool is_source = (name[0] == 'I' || name[0] == 'V'); //support for current sources and voltage sources
	    bool is_nonlinear_component = (name[0] == 'D'); //support for diodes


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
        if (is_component){
            components.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
        }else if (is_source){
            components.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
        }else if (is_nonlinear_component){
	    components.push_back(new nonlinear_component(input[0], truenode1, truenode2, name, "ideal_diode"));
        }


        if (input[0]=='V'){
            voltage_source_counter++;
        } else if (input[0]=='C'){
            capacitor_counter++;
        } else if (input[0]=='D'){
            diode_checker=1;
        }


        //Add nodes to the node vector
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector);
        component_counter += 1;
        input.clear();
    }


    /////////ARRAY STORAGE/////////
    const int h = node_vector.size()+voltage_source_counter+capacitor_counter-1;
    const int w = node_vector.size()+voltage_source_counter+capacitor_counter-1;
    MatrixXd g(h, w);


    /////////RUNS TRANSIENT SIM//////////
    cout << "Time " << "V(" << calc_node << ")" << endl;
    int calc_node_ID = stoi(calc_node.substr(1))-1;
    for (float t=0; t<=stop_time; t+=time_step){
    diode_checker = 1;
    while (looper){
    if(cond){
    final_loop = 0;


    //SET G MATRIX TO 0
    for (int i=0; i<h; i++){
        for(int j=0; j<w; j++){
            g(i,j) = 0;
        }
    }


    //CREATES G MATRIX
    for (int i=0; i<components.size(); i++){
        int node1_ID = components[i]->return_nodes()[0]->return_ID()-1;
        int node2_ID = components[i]->return_nodes()[1]->return_ID()-1;
        //cout << components[i]->return_name() << endl;
        if (components[i]->return_type()=='R'){
            if (node1_ID!=-1){
                g(node1_ID, node1_ID) += 1/components[i]->return_value(0, 0);
            }
            if (node2_ID!=-1){
                g(node2_ID, node2_ID) += 1/components[i]->return_value(0, 0);
            }
            if (node1_ID!=-1 && node2_ID!=-1){
                g(node1_ID, node2_ID) -= 1/components[i]->return_value(0, 0);
                g(node2_ID, node1_ID) -= 1/components[i]->return_value(0, 0);
            }
	} else if (components[i]->return_type()=='D'){
            if(diode_checker == 1){
		components[i]->set_prev_cv(0);
            }

	    if (node1_ID!=-1){
                g(node1_ID, node1_ID) += 1/components[i]->return_Req()->return_value(0, 0);
            }
            if (node2_ID!=-1){
                g(node2_ID, node2_ID) += 1/components[i]->return_Req()->return_value(0, 0);
            }
            if (node1_ID!=-1 && node2_ID!=-1){
                g(node1_ID, node2_ID) -= 1/components[i]->return_Req()->return_value(0, 0);
                g(node2_ID, node1_ID) -= 1/components[i]->return_Req()->return_value(0, 0);
	    }
        } else if (components[i]->return_type()=='V' || components[i]->return_type()=='C'){
            float position = stoi(components[i]->return_name().substr(1))+node_vector.size()-2;
            if (components[i]->return_type()=='C'){
                position += voltage_source_counter;
            }
            if (node1_ID!=-1){
                g(position, node1_ID) = 1;
                g(node1_ID, position) = 1;
            }
            if (node2_ID!=-1){
                g(position, node2_ID) = -1;
                g(node2_ID, position) = -1;
            }
        }
    }

if(diode_checker == 0){
    cond = 0;
    final_loop = 1;
}else{
    diode_checker++;
}
if(diode_checker == 5){
    final_loop = 1;
}
}


    //////////TEST PRINT CONDUCTANCE MATRIX//////////
    //cerr << endl << "Conductance Matrix" << endl << g << endl;
        /////////CALCULATE CURRENT MATRIX//////////
        MatrixXd current(h,1);
        vector<float> temp = find_current(components, node_vector.size()-1, voltage_source_counter, capacitor_counter, t, time_step, final_loop);
        for(int f=0; f < temp.size(); f++){
            current(f) = temp[f];
        }
        //////////CALCULATE VOLTAGE MATRIX//////////
        MatrixXd v(h,1);
        v = g.fullPivLu().solve(current);


	if(final_loop == 1){
	    cout << t << '\t' << v(calc_node_ID, 0) << endl;
	}
	////////INPUTTING NEW VALUES INTO PREV VARIABLES////////////////
	for (int i=0; i<components.size(); i++){
        ///////////SETTING CAPACTIOR PREVIOUS VALUES (CURRENT)//////////////
        if (components[i]->return_type()=='C' && final_loop == 1){
            if (components[i]->return_nodes()[0]->return_ID() != 0){
                float cap_current = v((stoi(components[i]->return_name().substr(1))+node_vector.size()+voltage_source_counter-2), 0);
                components[i]->set_prev_cv(cap_current);
            }
        ///////////SETTING INDUCTOR PREVIOUS VALUES (VOLTAGE)///////////////
        }else if (components[i]->return_type() == 'L' && final_loop == 1){
            if (components[i]->return_nodes()[1]->return_ID() != 0){
                float ind_voltage = v((components[i]->return_nodes()[1]->return_ID()-1), 0) - v((components[i]->return_nodes()[0]->return_ID()-1), 0);
                components[i]->set_prev_cv(ind_voltage);
            }
	    //////////SETTING DIODE PREVIOUS VALUES
        }else if (components[i]->return_type() == 'D' && final_loop == 0){
            if (components[i]->return_nodes()[1]->return_ID() != 0){
                prev_diode_vd = v((components[i]->return_nodes()[1]->return_ID()-1) , 0);
                components[i]->set_prev_cv(prev_diode_vd);
            }
	    }
    }
}
if(final_loop == 1){
	break;
}
}
}
