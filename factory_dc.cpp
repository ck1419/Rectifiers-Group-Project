#include "component.hpp"
#include <iostream>
#include <sstream>

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

        //cerr << name << " " << node1 << " " << node2 << " " << value << endl;

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

        /*
        //Add nodes to a vector (version1)
        std::vector<string>::iterator it = find(nodes.begin(), nodes.end(), node1);
        if (it == nodes.end()) {
            nodes.push_back(node1);
        }
        it = find(nodes.begin(), nodes.end(), node2);
        if (it == nodes.end()) {
            nodes.push_back(node2);
        }
        */

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
    for (int i=0; i<nodes.size(); i++) {
        cerr << nodes[i] << " ";
    }
    cerr << endl;

}

