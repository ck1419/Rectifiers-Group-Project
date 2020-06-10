#include "base_class.hpp"
#include "basic_component.hpp"
#include "node.hpp"
#include "nonlinear_component.hpp"
#include "source.hpp"
#ifndef add_nodes_to_vector_hpp
#define add_nodes_to_vector_hpp



//Search vector for a node, and adds it to the vector if the node isn't found
vector<node *> add_nodes_to_vector(node *input_node1, node *input_node2, base_class *current_component, vector<node *> array)
{
    bool not_found1 = true;
    bool not_found2 = true;
    node* vector_node1 = input_node1;
    node* vector_node2 = input_node2;
    for(int i=0; i<array.size(); i++){
        if (array[i]->return_ID() == input_node1->return_ID()){
            not_found1 = false;
            vector_node1 = array[i];
	    }
        if (array[i]->return_ID() == input_node2->return_ID()){
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



#endif
