#ifndef nonlinear_component_hpp
#define nonlinear_component_hpp


#include "base_class.hpp"
#include <iostream>
using namespace std;

class nonlinear_component: public base_class
{
    protected:
        string model;
        base_class* Ieq;
        base_class* Req;
        double Isat, Vtemp;


    public:
        //Constructor for diodes
        nonlinear_component(char c_type, node *c_node1, node *c_node2, string c_name, string c_model){
            type = c_type;
            node1 = c_node1;
            node2 = c_node2;
            name = c_name;
            model = c_model;
            Isat = 3e-9;
            Vtemp = 0.025;
            prev_cv = 0;
        }



        //Returns value of previous iteration
        void set_prev_cv(double cv){
            if (type == 'D'){
                if(Req!=nullptr){
                    delete Req;
                }
                if(Ieq!=nullptr){
                    delete Ieq;
                }
                prev_cv = cv;         
                double I_component = Isat * ( pow(M_E, prev_cv/Vtemp)-1 );
                if(prev_cv == 0){
                    Req = new basic_component('R', 100000000000000000, node1, node2, "Req");
                }else{
                    Req = new basic_component('R', 1/(I_component/Vtemp), node1, node2, "Req");
                }
                Ieq = new source('I', "dc" , node2, node1, "Ieq", 0, (I_component - (I_component/Vtemp)*prev_cv), 0);
            }
        }


        //Returns the equivalent resistor
        base_class* return_Req(){
            return Req;
        }


        //Returns the equivalent current source
        base_class* return_Ieq(){
            return Ieq;
        }
};


#endif
