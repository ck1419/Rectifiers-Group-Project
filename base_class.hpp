#ifndef base_class_hpp
#define base_class_hpp


#include <vector>
#include <string>
#include <cmath>
using namespace std;


class node;


//Base class just for inherited functionality so everything can use the same methods when iterating through arrays of components
class base_class
{
    protected:
        node* node1;                    //Used to tell which nodes this component is connected to
        node* node2;
        string name;       
        double value;
        double prev_cv = 0;              //Used for transient sims to store the previous voltage/current value
	    double tot_acc = 0;
        char type;


    public:
        //Returns the nodes the component is connected to in a vector form
        virtual vector<node*> return_nodes(){
            vector<node*> temp{node1, node2};
            return temp;
        }


        //Returns type of component
        virtual char return_type(){
            return type;
        }


        //Returns name of the current component
        virtual string return_name(){
            return name;
        }


        //Sets the previous value of current/voltage
        virtual void set_prev_cv(double cv){
            prev_cv = cv;
        }
        
        //Returns the previous value of current/voltage
        virtual double return_prev_cv(){
            return prev_cv;
        }

        //Returns current/voltage/resistance depending on subclass (Will be overwritten in the appropriate subclasses)
        virtual double return_value(double t, bool final_loop_checker){
            return 1;
        }

        virtual double return_tot_acc(){
            return tot_acc;
        }

        virtual void set_tot_acc(double acc){
            tot_acc = acc;
        }

        //Used for nonlinear components and will be overwritten. Function is here to allow access to them in the base_class vector
        virtual base_class* return_Req(){
            return this;
        }
        virtual base_class* return_Ieq(){
            return this;
        }
        virtual base_class* return_Rl(){
            return this;
        }

};


#endif
