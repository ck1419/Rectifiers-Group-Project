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


int main()
{

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



    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input))
    {

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
            /*
            if (name[0] == 'C') { //push back parallel resistor for capacitor
                components.push_back(new basic_component('R',1000000000, truenode1, truenode2, "par_resis"));
                component_counter += 1;
            }
            */
        }else if (is_source){
            
            components.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
        }

        if (input[0]=='V'){
            voltage_source_counter++;
        } else if (input[0]=='C'){
            capacitor_counter++;
	}
        //Add nodes to the node vector //DO NOT add capacitor parallel resistance to conductance matrix.
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector); 
        component_counter += 1;
        input.clear();
    }



    /////////ARRAY STORAGE/////////
    const int h = node_vector.size()+voltage_source_counter+capacitor_counter-1;
    const int w = node_vector.size()+voltage_source_counter+capacitor_counter-1;
    MatrixXd g(h, w);


    //CREATES G MATRIX
    for (int i=0; i<components.size(); i++){
        int node1_ID = components[i]->return_nodes()[0]->return_ID()-1;
        int node2_ID = components[i]->return_nodes()[1]->return_ID()-1;
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

    /////////RUNS TRANSIENT SIM//////////
    cout << "Time";
    for(int b=0; b<node_vector.size();b++){
        if(node_vector[b]->return_ID()!=0){
            cout << '\t' << "V(" << node_vector[b]->return_ID() << ")";
        }
    }
    cout << endl;

    /////////RUNS TRANSIENT SIM//////////
    for (long double t=0; t<=stop_time; t+=0.00001){
        /////////CALCULATE CURRENT MATRIX//////////
        MatrixXd current(h,1);
        vector<float> temp = find_current(components, node_vector.size()-1, voltage_source_counter, capacitor_counter, t, 0.00001, 1);
        for(int f=0; f < temp.size(); f++){
            current(f) = temp[f];
        }

        //////////CALCULATE VOLTAGE MATRIX//////////
        MatrixXd v(h,1);
        v = g.fullPivLu().solve(current);

       // if ((t/time_step) ==  int(t/time_step)) {
            cout << t;
            cerr << " " << time_step  << " " << t/time_step << " " << int(t/time_step) << " ";
            for(int b=0; b<node_vector.size();b++){
                if(node_vector[b]->return_ID()!=0){
                    cout << '\t' << v((node_vector[b]->return_ID()-1),0);
                }
            }
            cout << endl;           
        //}



	    ////////INPUTTING NEW VALUES INTO PREV VARIABLES////////////////
        for (int i=0; i<components.size(); i++){
            ///////////SETTING CAPACTIOR PREVIOUS VALUES (CURRENT)//////////////
            if (components[i]->return_type()=='C'){
                if (components[i]->return_nodes()[0]->return_ID() != 0){
                    float cap_current = v((stoi(components[i]->return_name().substr(1))+node_vector.size()+voltage_source_counter-2), 0);
                    components[i]->set_prev_cv(cap_current);
                }
            ///////////SETTING INDUCTOR PREVIOUS VALUES (VOLTAGE)///////////////
            }else if (components[i]->return_type() == 'L'){
                if (components[i]->return_nodes()[1]->return_ID() != 0){
                    float ind_voltage = v((components[i]->return_nodes()[1]->return_ID()-1), 0) - v((components[i]->return_nodes()[0]->return_ID()-1), 0);
                    components[i]->set_prev_cv(ind_voltage);
                }
            }
        }
    }
}