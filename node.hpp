#ifndef node_hpp
#define node_hpp


//Class for storing node data
class node
{
    private:
        vector<node*> connected_nodes;                  //Pointer to connected nodes
        vector<base_class*> connected_components;       //Pointer to connected components
        int ID;                                         //ID = 0 for grounded/reference node            


    public:
        //Constructor
        node(int c_ID){
            ID = c_ID;
        }


        //Returns connected nodes
        vector<node*> return_nodes(){
            return connected_nodes;
        }


        //Returns connected components
        vector<base_class*> return_components(){
            return connected_components;
        }


        //Returns ID of current node
        int return_ID(){
            return ID;
        }


        //Add the nodes directly connected to the current node after a check for duplicate entries
        void add_node(node *c_node){
            bool not_found = true;
            for (int i=0; i<connected_nodes.size(); i++){
                if ( connected_nodes[i]->return_ID() == c_node->return_ID() ){
                    not_found = false;
                }
            }
            if (not_found){
                connected_nodes.push_back(c_node);
            }
        }


        //Add components connected to the current node after a check for duplicate entries
        void add_component(base_class *c_component){
            bool not_found = true;
            for(int i=0; i<connected_components.size(); i++){
                if ( connected_components[i]->return_name() == c_component->return_name() ){
                    not_found = false;
                }
            }
            if (not_found){
                connected_components.push_back(c_component);
            }
        }
};



#endif