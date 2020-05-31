#include "component.hpp"
#include "current_matrix.hpp"
#include "scientific_converter.hpp"
#include "add_nodes_to_vector.hpp"
#include <iostream>
#include <sstream>
#include <Eigen/Dense>
using Eigen::MatrixXd;
/*
//////////ALGORITHM for circuits with only resistors and current sources AND VOLTAGE SOURCES//////////

1) Take in components and sources from netlist (current sources and voltage sources work in a totally different way tho)
2) Determine the number of nodes (v_num)

4) Construct G Matrix
    4.1) The diagonal is always positive: sum of all conductances connected to node
        If voltage node row, insert 1, -1 or 0s only
        If supernode row, sum of all conductances connected to both nodes, except conductances in between 2 ndoes.
    4.2) Other conductances: sum of all conductances in between 2 nodes.
        If not voltage/ supernode row, must be negative.
        If voltage node row, insert 1, -1 or 0s only. 
        If supernode row, sum of all conductances node, except conductances in between 2 ndoes. If one node is supernode, that entry must be positive.
5) V matrix implicit - it's the result
6) I matrix - value is net current into node. one entry is for voltage source value (dealt with in current matrix cpp)

*/

/*
//Search vector for a node, and adds it to the vector if the node isn't found
vector<node *> add_nodes_to_vector(node *input_node1, node *input_node2, base_class *current_component, vector<node *> array)
{
    bool not_found1 = true;
    bool not_found2 = true;
    node *vector_node1 = input_node1;
    node *vector_node2 = input_node2;
    for (int i = 0; i < array.size(); i++)
    {
        if (array[i]->return_ID() == input_node1->return_ID())
        {
            not_found1 = false;
            vector_node1 = array[i];
        }
        if (array[i]->return_ID() == input_node2->return_ID())
        {
            not_found2 = false;
            vector_node2 = array[i];
        }
    }
    if (not_found1)
    {
        array.push_back(input_node1);
    }
    if (not_found2)
    {
        array.push_back(input_node2);
    }
    vector_node1->add_node(vector_node2);
    vector_node2->add_node(vector_node1);
    vector_node1->add_component(current_component);
    vector_node2->add_component(current_component);
    return array;
}
*/

int main()
{

    string input;

    //////////TEMPORARY STORAGE/////////
    vector<node *> node_vector;      //keep track of nodes and their connected components
    vector<base_class *> components; //stores all the components in the circuit
    string node1, node2, name;
    char type;
    float value;               //for RC components
    string output_type;        //triangle, square, sine etc.
    int component_counter = 0; //component counter for figuring out current number of component
    bool node_contains_v_source = false;
    bool v_source_grounded = false;
    /////////BOOLEAN DEFINITIONS//////////

    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input))
    {
        int node1_ID, node2_ID;
        istringstream ss(input);
        ss >> name;
        ss >> node1;
        ss >> node2;
        ss >> value;

        //creating conditionals for parsing in the various components from the netlist
        bool is_component = (name[0] == 'R');                //support for C and L comes later
        bool is_source = (name[0] == 'I' || name[0] == 'V'); //support for current sources and voltage sources

        //creating dynamic storage for the nodes that have been inputted
        node *truenode1;
        node *truenode2;
        if (node1[0] == 'N')
        {
            node1_ID = stoi(node1.substr(1));
            truenode1 = new node(node1_ID);
        }
        else
        {
            node1_ID = stoi(node1);
            truenode1 = new node(node1_ID);
        }

        if (node2[0] == 'N')
        {
            node2_ID = stoi(node2.substr(1));
            truenode2 = new node(node2_ID);
        }
        else
        {
            node2_ID = stoi(node2);
            truenode2 = new node(node2_ID);
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

        cerr << value;

        //Add components/ sources to component vector
        if (is_component)
        {
            if (node2_ID != node1_ID)
            { // do not push back components if they have been short circuited
                components.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
            }
        }
        else if (is_source)
        {
            components.push_back(new source(input[0], "DC", truenode1, truenode2, name, 0, value, 0)); //only DC for now
        }
        else
        {
            //exit(1);
        }

        //Add nodes to the node vector
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector);
        component_counter += 1;

        //reset temporary variables
        input.clear();
        node_contains_v_source = false;
        v_source_grounded = false;
    }

    //Now we should have a vector of components and nodes.

    //////////TEST PRINT//////////
    cerr << endl
         << "Print components" << endl;
    for (int i = 0; i < components.size(); i++)
    { //print components
        cerr << components[i]->return_type() << " " << components[i]->return_nodes()[0]->return_ID() << " " << components[i]->return_nodes()[1]->return_ID();
        cerr << " " << components[i]->return_value(0) << endl;
    }
    cerr << endl
         << "Print nodes" << endl;
    for (int i = 0; i < node_vector.size(); i++)
    { //print nodes
        cerr << node_vector[i]->return_ID() << " ";
    }
    cerr << endl;

    /////////ARRAY STORAGE/////////
    const int h = node_vector.size() - 1;
    const int w = node_vector.size() - 1;
    MatrixXd g(h, w);

    //////////TEMPORARY VARIABLES FOR ARRAY//////////
    int node_ID_1;
    int node_ID_2;
    int no_of_components;
    float diag_conductance = 0;
    float other_conductance = 0;
    //bool locked_entry = false;

    //////////NOW BEGIN INSERTING INTO ARRAY//////////
    //Insert conductance matrix entries
    for (int i = 0; i < node_vector.size(); i++)
    { // iterating through nodes (except reference node)
        node_ID_1 = node_vector[i]->return_ID() - 1;
        no_of_components = node_vector[i]->return_components().size();
        if (node_ID_1 == -1)
        { //not doing anything with ground
            continue;
        }
        //just filling in regular entries
        for (int j = 0; j < no_of_components; j++)
        { //iterating through number of components connected to one node
            if (node_vector[i]->return_v_source_neg() || node_vector[i]->return_v_source_pos())
            {
                //THINGS TO WORK ON IN CURRENT MATRIX
                //ASSUMING VOLTAGE SOURCE VALUES ARE ALWAYS GIVEN WITH RESPECT TO GROUND (e.g. negative if positive terminal towards ground)
                //ASSUME: if multiple parallel voltage sources from ground, both same direction, voltage in current matrix at this node is maximum of voltages;
                //ASSUME: if parallel voltage sources opposite directions, voltage in current matrix is net sum.
                if (node_vector[i]->return_v_source_neg() && node_vector[i]->return_v_source_grounded())
                { //grounded voltage source, positive terminal pointing to ground
                    diag_conductance = 1;
                    //only looking for the same grounded voltage source
                    node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1;
                    if (node_ID_2 == -1) //if ground, we have already considered it properly
                    {
                        continue;
                    }
                    //other conductances have to be zero, even if there are other voltage sources (grounded or non-grounded) connected to this node
                }
                else if (node_vector[i]->return_v_source_pos() && node_vector[i]->return_v_source_grounded())
                { //grounded voltage source, negative terminal pointing to ground
                    diag_conductance = 1;
                    //only looking for the same grounded voltage source
                    node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1;
                    if (node_ID_2 == -1) //if ground, we have already considered it properly
                    {
                        continue;
                    }
                    //other conductances have to be zero, even if there are other voltage sources (grounded or non-grounded) connected to this node
                }
                else if (node_vector[i]->return_v_source_pos() && !node_vector[i]->return_v_source_grounded())
                { //non-grounded voltage source, at the positive terminal e.g. v= vc-vb
                    diag_conductance = 1;
                    if (node_vector[i]->return_components()[j]->return_type() == 'V')
                    {
                        //if same voltage source
                        other_conductance = -1;
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID() - 1;
                        g(node_ID_1, node_ID_2) = other_conductance;
                    }
                }
                else if (node_vector[i]->return_v_source_neg() && !node_vector[i]->return_v_source_grounded())
                { //non-grounded voltage source, at the negative terminal, perform relatively normal stuff, except other conductance positive for supernodes (nodes connected voltage source)
                    if (node_vector[i]->return_components()[j]->return_type() == 'R')
                    { //if connected component is a resistor
                        diag_conductance += 1 / (node_vector[i]->return_components()[j]->return_value(0));
                        other_conductance = 1 / (node_vector[i]->return_components()[j]->return_value(0));
                        if (node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1 == node_ID_1)
                        {
                            node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID() - 1;
                        }
                        else
                        {
                            node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1;
                        }
                        if (node_ID_2 == -1)
                        {
                            continue;
                        }
                        if (node_vector[i]->return_components()[j]->return_nodes()[1]->return_v_source_pos() == true)
                        { //if the oer node is the node connected to the positive terminal of the voltage source
                            g(node_ID_1, node_ID_2) += other_conductance;
                        }
                        else
                        {
                            g(node_ID_1, node_ID_2) += -other_conductance;
                        }
                    }
                }
            }
            else
            { //COPIED FROM ORIGINAL FACTORY DC
                if (node_vector[i]->return_components()[j]->return_type() == 'R')
                { //if connected component is a resistor
                    diag_conductance += 1 / (node_vector[i]->return_components()[j]->return_value(0));
                    other_conductance = 1 / (node_vector[i]->return_components()[j]->return_value(0));
                    if (node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1 == node_ID_1)
                    {
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[1]->return_ID() - 1;
                    }
                    else
                    {
                        node_ID_2 = node_vector[i]->return_components()[j]->return_nodes()[0]->return_ID() - 1;
                    }
                    if (node_ID_2 == -1)
                    {
                        continue;
                    }
                    g(node_ID_1, node_ID_2) += -other_conductance;
                }
            }
            g(node_ID_1, node_ID_1) = diag_conductance;
            diag_conductance = 0;
        }
    }

    //////////TEST PRINT CONDUCTANCE MATRIX//////////
    cerr << endl
         << "Conductance Matrix" << endl
         << g << endl;

    /////////CALCULATE CURRENT MATRIX//////////
    MatrixXd current(h, 1);
    vector<float> temp;
    temp = find_current(components, node_vector, 0);
    for (int f = 0; f < temp.size(); f++)
    {
        current(f) = temp[f];
    }
    //////////TEST PRINT CURRENT MATRIX//////////
    cerr << endl
         << "Current Matrix" << endl
         << current << endl;

    //////////CALCULATE VOLTAGE MATRIX//////////
    MatrixXd v(h, 1);
    v = g.fullPivLu().solve(current);

    /////////TEST PRINT VOLTAGE MATRIX/////////
    cerr << endl
         << "Voltage Matrix" << endl
         << v << endl;
}
