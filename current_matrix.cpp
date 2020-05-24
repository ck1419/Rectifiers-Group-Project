#include "component.hpp"
using namespace std;

/*
KCL SCENARIOS:
1. ground - current source - node

2. node - known current - node - unknown current component/source - node

3. node - known current - node - unknown current component/source - node
   node - known current ^

4. node - known current - node - ground
                        - node - unknown current component/source - node
                        ....
                        ....

TODO:
- Scenario 4
- Convert currents across components to current matrix
*/


vector<float> find_current(vector<base_class*> all_components, vector<node*> all_nodes, int t){
    //Set current for accross component for current sources connected to ground/reference node (scenario 1)
    for(int i=0; i<all_components.size(); i++){
        vector<node*> connected_nodes = all_components[i]->return_nodes();
        if (all_components[i]->return_type()=='I' && connected_nodes.size()==2){
            if (connected_nodes[0]->return_ID()=='0'){
                all_components[i]->set_current( all_components[i]->return_value(t) );
            }
            else if (connected_nodes[1]->return_ID()=='0'){
                all_components[i]->set_current( -all_components[i]->return_value(t) );
            }
        }
    }
    


    //Scenario 2 and Scenario 3
    bool calculating = true;
    while (calculating){
        calculating = false;
        //Checks for whether a node only has one unknown current component connected to it
        for(int i=0; i<all_nodes.size(); i++){
            //Variables to store information on current node
            vector<base_class*> connected_components = all_nodes[i]->return_components();
            base_class *final_component;        //Used to store the component that doesnt have a current. Used only if it is the only unknwon component
            int components_without_current = 0;         //Counts the amount of components with unknown currents
            float current_total = 0;        //Total of the currents going into a node. Used only if one component is missing a current
            //Loops through the node to check each of the component for a current value and collects data on the components
            for(int x=0; x<connected_components.size(); x++){
                if(connected_components[x]->current_calculated()){
                    current_total += connected_components[x]->return_current();
                }else{
                    components_without_current += 1;
                    final_component = connected_components[x];
                }
            }
            //If only one component is missing it's current value, apply KCL on it
            if(components_without_current==1){      
                if (final_component->return_type() == 'I'){     //Current source value added if component is a current source
                    if(final_component->return_nodes()[0] == all_nodes[i]){   //Checks for polarity of source
                        current_total += final_component->return_value(t);
                    }else{
                        current_total -= final_component->return_value(t);
                    }
                }
                final_component->set_current(current_total);    //Sets the current going across the component to the total going into the node
                calculating = true;     //Continue the loop again if a change was made
            }
        }   
    }



    //Fills in the current matrix
    vector<float> current_matrix;
    for(int i=0; i<all_nodes.size(); i++){
        current_matrix.push_back(0);
    }
    for(int i=0; i<all_nodes.size(); i++){
        float current = 0;
        vector<base_class*> connected_components = all_nodes[i]->return_components();
        for(int x=0; x<connected_components.size(); x++){
            current += connected_components[x]->return_current();
        }
        current_matrix.push_back(current);
    }
    return current_matrix;
}