#include "component.hpp"
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


//Search vector for a node, and adds it to the vector if the node isn't found
vector<string> add_node(string node, vector<string> array){
    bool not_found = true;
    for(int i=0; i<array.size(); i++){
        if (array[i] == node){
            not_found = false;
        }
    }
    if (not_found){
        array.push_back(node);
    }
    return array;
}

int main() {

    string input;

    //////////TEMPORARY STORAGE/////////
    vector<string> nodes; //keep track of number of nodes
    vector<basic_component> components; //only R for now
    vector<source> sources; //only current sources for now
    string node1, node2, name; 
    char type;
    float value; //for RC components
    string output_type; //triangle, square, sine etc.

    /////////BOOLEAN DEFINITIONS//////////

    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input)) {

        istringstream ss(input);
        ss >> name;
        ss >> node1;
        ss >> node2;
        ss >> value;

        bool is_component = (name[0] == 'R'); //support for C and L comes later
        bool is_source = (name[0] == 'I'); //support for voltage sources come later

        //Add components/ sources to vector
        if (is_component) {
            components.push_back(basic_component(input[0], value, node1, node2, name)); 
        } else if (is_source) {
            sources.push_back(source(input[0], "DC", node1, node2, name, 0 , value, 0)); //only DC for now
        } else {
            //exit(1);
        }

        //Add nodes to a vector (version3)
        nodes = add_node(node1, nodes);
        nodes = add_node(node2, nodes);

        input.clear();
    }

    //Now we should have a vector of components, sources and nodes.

    //////////TEST PRINT//////////
    cerr << endl << "Print components" << endl;
    for (int i=0; i<components.size(); i++) { //print components
        cerr << components[i].return_type() << " " << components[i].return_nodes()[0] << " " << components[i].return_nodes()[1];
        cerr << " " << components[i].return_value(0) << endl;
    }
    cerr << endl << "Print sources" << endl;
    for (int i=0; i<sources.size(); i++) { //print sources
        cerr << sources[i].return_type() << " " << sources[i].return_nodes()[0] << " " << sources[i].return_nodes()[1];
        cerr << " " << sources[i].return_value(0) << endl;
    }
    cerr << endl << "Print nodes" << endl;
    for (int i=0; i<nodes.size(); i++) { //print nodes
        cerr << nodes[i] << " ";
    }
    cerr << endl;

    /////////ARRAY STORAGE/////////
    const int h = nodes.size();
    const int w = nodes.size();
    MatrixXd g(h,w);

    //////////TEMPORARY VARIABLES FOR ARRAY//////////
    string node_label;
    float conductance = 0;
    string node_label_one;
    string node_label_two;

    //////////NOW BEGIN INSERTING INTO ARRAY//////////
    //First insert diagonal entries
    for (int i=0; i<nodes.size(); i++) {
        node_label = nodes[i];
        for (int j=0; j<components.size(); j++) {
            if((components[j].return_nodes()[0] == node_label) || (components[j].return_nodes()[1] == node_label)) {
                conductance += 1/(components[j].return_value(0));
            }
        }
        g(i,i) = conductance;
        conductance = 0;
    }

    //Now deal with upper triangular
    for (int i=1; i<nodes.size(); i++) { //going across
        node_label_one = nodes[i];
        for (int j=0; j<i; j++) { //going downwards
            node_label_two = nodes[j];
            for(int k=0; k<components.size(); k++) {
                bool between = ((components[k].return_nodes()[0] ==  node_label_one) && (components[k].return_nodes()[1] ==  node_label_two)) || ((components[k].return_nodes()[1] ==  node_label_one) && (components[k].return_nodes()[0] ==  node_label_two));
                if (between) {
                    conductance += 1/(components[k].return_value(0));
                }
            }
            g(j,i) = -conductance;
            g(i,j) = -conductance;
            conductance = 0;
        }
    }

    //////////TEST PRINT ARRAY//////////
        cerr << g << endl;
}

