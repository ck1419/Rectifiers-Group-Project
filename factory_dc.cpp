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

    string input;

    //////////TEMPORARY STORAGE/////////
    vector<node *> node_vector;      //keep track of nodes and their connected components
    vector<base_class *> components; //stores all the components in the circuit
    string node1, node2, name, temp_value;
    char type;
    float value;               //for RC components
    string output_type;        //triangle, square, sine etc.
    int component_counter = 0; //component counter for figuring out current number of component
    /////////BOOLEAN DEFINITIONS//////////

    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input))
    {

        istringstream ss(input);
        ss >> name;
        ss >> node1;
        ss >> node2;
        ss >> temp_value;
        value = scientific_converter(temp_value);

        //creating conditionals for parsing in the various components from the netlist
        bool is_component = (name[0] == 'R');                //support for C and L comes later
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

        //Add components/ sources to component vector
        if (is_component){
            components.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
        }else if (is_source){
            components.push_back(new source(input[0], "DC", truenode1, truenode2, name, 0, value, 0)); //only DC for now
        }else{
            //exit(1);
        }
        //Add nodes to the node vector
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector);
        component_counter += 1;
        input.clear();
    }

    //Now we should have a vector of components and nodes.

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
    int no_of_components;
    float diag_conductance = 0;
    float other_conductance = 0;

    //////////NOW BEGIN INSERTING INTO ARRAY//////////
    //Insert both diagonal and upper triangular entries
    for (int i = 0; i < node_vector.size(); i++){
        node_ID_1 = node_vector[i]->return_ID()-1;
        no_of_components = node_vector[i]->return_components().size();
        if(node_ID_1 == -1){
            continue;
        }
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
        g(node_ID_1, node_ID_1) = diag_conductance;
        diag_conductance = 0;
    }
    //////////TEST PRINT CONDUCTANCE MATRIX//////////
    cerr << endl << "Conductance Matrix" << endl << g << endl;

    /////////CALCULATE CURRENT MATRIX//////////
    MatrixXd current(h,1);
    vector<float> temp;
    temp = find_current(components, node_vector, 0);
    for(int f=0; f < temp.size(); f++){
	current(f) = temp[f];
    }
    //////////TEST PRINT CURRENT MATRIX//////////
    cerr << endl << "Current Matrix" << endl << current << endl;

    //////////CALCULATE VOLTAGE MATRIX//////////
    MatrixXd v(h,1);
    v = g.fullPivLu().solve(current);

    /////////TEST PRINT VOLTAGE MATRIX/////////
    cerr << endl << "Voltage Matrix" << endl << v << endl;
}
