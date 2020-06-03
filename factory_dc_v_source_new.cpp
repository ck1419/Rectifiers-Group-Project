#include "component.hpp"
#include "current_matrix.hpp"
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
    bool node_contains_v_source = false;
    bool v_source_grounded = false;
    int node1_ID, node2_ID;
    /////////BOOLEAN DEFINITIONS//////////

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
        bool is_component = (name[0] == 'R');                //support for C and L comes later
        bool is_source = (name[0] == 'I' || name[0] == 'V'); //support for current sources and voltage sources

        //creating dynamic storage for the nodes that have been inputted
        node *truenode1;
        node *truenode2;
        if (node1[0] == 'N'){
            node1_ID = stoi(node1.substr(1));
            truenode1 = new node(stoi(node1.substr(1)));
        }else{
            node1_ID = stoi(node1);
            truenode1 = new node(stoi(node1));
        }

        if (node2[0] == 'N'){
            node2_ID = stoi(node2.substr(1));
            truenode2 = new node(stoi(node2.substr(1)));
        }else{
            node2_ID = stoi(node2);
            truenode2 = new node(stoi(node2));
        }

        cerr << endl << name << " Value " << value << endl;

        //Add components/sources to component vector (WITH SINE SOURCE SUPPORT)
        if (is_component){
            if (node2_ID != node1_ID) {
                components.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
            }     
        }else if (is_source){
            components.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
        }else{
            //exit(1);
        }

        //identify positive and negative terminals of the voltage source
        if (name[0] == 'V')
        {
            if (node1_ID == 0 || node2_ID == 0)
            {
                v_source_grounded = true;
                cerr << "v_source_ground identified" << endl;
            }
            node_contains_v_source = true;
            truenode1->set_v_source_true("pos", v_source_grounded);
            truenode2->set_v_source_true("neg", v_source_grounded);
        }


        //Add nodes to the node vector
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector);
        component_counter += 1;
        input.clear();
    }



    //////////TEST PRINT//////////
    cerr << endl << "Print components" << endl;
    for (int i = 0; i < components.size(); i++){ //print components
        cerr << components[i]->return_type() << " " << components[i]->return_nodes()[0]->return_ID() << " " << components[i]->return_nodes()[1]->return_ID();
        cerr << " " << components[i]->return_value(0) << endl;
    }
    cerr << endl << "Print nodes" << endl;
    for (int i = 0; i < node_vector.size(); i++){ //print nodes
        cerr << node_vector[i]->return_ID() << " ";
    }
    cerr << endl;



    /////////ARRAY STORAGE/////////
    const int h = node_vector.size()-1;
    const int w = node_vector.size()-1;
    MatrixXd g(h, w);


    //////////TEMPORARY VARIABLES FOR ARRAY//////////
    int node_ID_1;
    int node_ID_2;
    int positive_terminal;
    int no_of_components;
    float diag_conductance = 0;
    float other_conductance = 0;

    //CREATES G MTRIX
    for (int i = 0; i < node_vector.size(); i++){
        node_ID_1 = node_vector[i]->return_ID()-1;
        no_of_components = node_vector[i]->return_components().size();
        if(node_ID_1 == -1){
            continue;
        }

        //CERR BOOLEANS BEFORE STARTING
        cerr << endl << "v_source_pos = " << node_vector[i]->return_v_source_pos() << " and v_source neg = " << node_vector[i]->return_v_source_neg() << endl;
        cerr << "v source existing?" << (node_vector[i]->return_v_source_neg() || node_vector[i]->return_v_source_pos()) << endl;

        if (node_vector[i]->return_v_source_neg() && node_vector[i]->return_v_source_grounded()) { //CASE 1:or source with positive terminal towards ground
            diag_conductance = 1;
        } else if (node_vector[i]->return_v_source_pos() && node_vector[i]->return_v_source_grounded()) { //CASE 2: for source with negative terminal towards ground
            diag_conductance = 1;
        }  else if (node_vector[i]->return_v_source_pos() && !node_vector[i]->return_v_source_grounded()) { //CASE 3: at positive terminal for non-grounded source, do v=v3-v2
            diag_conductance = 1;
            for (int j = 0; j < no_of_components; j++){
                if (node_vector[i]->return_components()[j]->return_type() == 'V') { //if same voltage source
                    other_conductance = -1;
                    node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID() - 1; //want node of other end, i.e. the negative terminal (so second entry or [1])
                    g(node_ID_1, node_ID_2) = other_conductance;
                }
            }
        } else if (node_vector[i]->return_v_source_neg() && !node_vector[i]->return_v_source_grounded()) { //CASE 4: at negative terminal or non-grounded source, do KCL equations for supernode.
            //non-grounded voltage source, at the negative terminal, perform relatively normal stuff, 
            //except other conductance positive for supernodes (nodes connected voltage source)
            //e.g. ignore conductances between negative and positive terminal nodes of voltage sources

            //identify positive terminal of voltage source    
            if (node_vector[i]->return_v_source_neg() && !node_vector[i]->return_v_source_grounded()) {
                for (int j = 0; j < no_of_components; j++){
                    if (node_vector[i]->return_components()[j]->return_nodes()[0]->return_v_source_pos() && node_vector[i]->return_v_source_grounded()) {
                        positive_terminal = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID()-1;
                    }
                    if (node_vector[i]->return_components()[j]->return_nodes()[1]->return_v_source_pos() && node_vector[i]->return_v_source_grounded()) {
                        positive_terminal = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID()-1;
                    }
                }
            }
            
            //now actually do the looping
            for (int j = 0; j < no_of_components; j++){
                //if connected component is a resistor
                if (node_vector[i]->return_components()[j]->return_type() == 'R') { 
                    diag_conductance += 1 / (node_vector[i]->return_components()[j]->return_value(0));
                    other_conductance = 1 / (node_vector[i]->return_components()[j]->return_value(0));
                    if (node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1 == node_ID_1) {
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID() - 1;
                    } else {
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1;
                    }
                    if (node_ID_2 == -1) {
                        continue;
                    }
                    if (node_ID_2 == positive_terminal) {
                        continue;
                    }
                    if (node_vector[i]->return_components()[j]->return_nodes()[1]->return_v_source_pos() == true) {
                     //if the other node is the node connected to the positive terminal of the voltage source, then conductance is positive not negative
                        g(node_ID_1, node_ID_2) += other_conductance;
                    } else {
                        g(node_ID_1, node_ID_2) += -other_conductance;
                    }
                }
            }
        } else { //CASE 5: Normal Conductance matrix entries
            for (int j = 0; j < no_of_components; j++){
                if (node_vector[i]->return_components()[j]->return_type() == 'R'){
                    diag_conductance += 1 / (node_vector[i]->return_components()[j]->return_value(0));
                    other_conductance = 1 / (node_vector[i]->return_components()[j]->return_value(0));
                    if (node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID()-1 == node_ID_1){
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID()-1;
                    }else{
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID()-1;
                    }
                    if(node_ID_2 == -1){
                        continue;
                    }
                    g(node_ID_1, node_ID_2) += -other_conductance;
                }              
            }
        }

        g(node_ID_1, node_ID_1) = diag_conductance;
        diag_conductance = 0;
    }


    //////////TEST PRINT CONDUCTANCE MATRIX//////////
    cerr << endl << "Conductance Matrix" << endl << g << endl;


    /////////RUNS TRANSIENT SIM//////////
    for (float t=0; t<=stop_time; t+=time_step){
        /////////CALCULATE CURRENT MATRIX//////////
        MatrixXd current(h,1);
        vector<float> temp;
        temp = find_current(components, node_vector, t);
        for(int f=0; f < temp.size(); f++){
        current(f) = temp[f];
        }

        //////////CALCULATE VOLTAGE MATRIX//////////
        MatrixXd v(h,1);
        v = g.fullPivLu().solve(current);

        /////////TEST PRINT RESULTS/////////
        cerr << endl << "/////////////////////////////////////////////////////////////////////////////////////////" << endl;
        cerr << endl << "TIME " << t << endl;
        cerr << endl << "Current Matrix" << endl << current << endl;
        cerr << endl << "Voltage Matrix" << endl << v << endl;
    }
}
