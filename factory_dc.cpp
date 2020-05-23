#include "component.cpp"
#include <iostream>
#include <sstream>

/* 

//////////ALGORITHM for circuits with only resistors and current sources//////////

1) Take in components and sources from netlist (current sources and voltage sources work in a totally different way tho)
2) Determine the number of nodes (v_num)

4) Construct G Matrix
    4.1) The diagonal is always positive: sum of all conductances connected to node
    4.2) Top triangular always negative: sum of all conductances in between 2 nodes.
    4.3) Lower triangular is identical (reflect values across diagonal)
5) V matrix implicit - it's the result
6) I matrix - value is net current into node.

*/

int main() {

    string input;

    //////////TEMPORARY STORAGE/////////
    vector<basic_component> components; //only R for now
    vector<source> sources; //only current sources for now
    string node1, node2, name; 
    char type;
    float value; //for RC components
    string output_type; //triangle, square, sine etc.

    /////////BOOLEAN DEFINITIONS//////////
    bool is_component = (input[0] == 'R'); //support for C and L comes later
    bool is_source = (input[0] == 'I'); //support for voltage sources come later

    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input)) {

        istringstream ss(input);
        ss >> name;
        ss >> node1;
        ss >> node2;
        ss >> value; 

        if (is_component) {
            components.push_back(basic_component(input[0], value, node1, node2, name));     
        } else if (is_source) {
            sources.push_back(source(input[0], "DC", node1, node2, name, 0 , value, 0)); //only DC for now
        }

    input.clear();
    }

    //Now we should have a vector of components and sources.

    //////////TEST PRINT//////////
    for (int i=0; i<components.size(); i++) {

    }

}

