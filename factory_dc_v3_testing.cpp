#include "base_class.hpp"
#include "source.hpp"
#include "basic_component.hpp"
#include "nonlinear_component.hpp"
#include "node.hpp"
#include "current_matrix_v5.hpp"
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
    //NEW VARIABLE DECLARATIONS (MERGED WITH OLD ONES)
    vector<node*> node_vector;      //keep track of nodes and their connected components
    vector<base_class*> components; //stores all the components in the circuit
    string node1, node2, name, temp_value, output_type, input, source_type;
    char type;
    float value;
    float stop_time = 0;
    float time_step = 1;
    int component_counter = 0; //component counter for figuring out current number of component
    int voltage_source_counter = 0;
    int capacitor_counter = 0;
    int inductor_counter = 0;
    bool diode_checker = 0;
    bool looper = 1;
    bool final_loop = 0;
    bool cond = 1;
    int diode_counter = 0;
    bool oc_looper = 0;

    /////////TAKE IN INDIVIDUAL LINES and define COMPONENTS/////////
    while (getline(cin, input)){

        float frequency = 0;
        float amplitude = 0;

        istringstream ss(input);
        ss >> name;
        if (name[0]=='.'){          //Checks for end of file (.tran 0 <stop time> 0 <timestep>) followed by (.end)
            ss >> temp_value;       //Removes dummy 0 in .trans
            ss >> temp_value;
            stop_time = scientific_converter(temp_value);
            ss >> temp_value;       //Removes dummy 0 after 
            ss >> temp_value;
            time_step = scientific_converter(temp_value);
            break;                  //Stops loop to avoid reading .end creating run time errors
        } else {                    //Parser for RLC and sources
            ss >> node1;
            ss >> node2;
            ss >> temp_value;
            if (tolower(temp_value[0])=='s'){       //Checks for sine sources   SINE(<dc offset> <amplitude> <frequency>)
                source_type = "sine";
                value = scientific_converter(temp_value.substr(5));     //Removes ' SINE( '
                ss >> temp_value;
                amplitude = scientific_converter(temp_value);
                ss >> temp_value;
                temp_value.pop_back();              //Removes )
                frequency = scientific_converter(temp_value);
            }else{                                  //Not a sine source
                source_type = "dc";
                value = scientific_converter(temp_value);
            }
        }


        //creating conditionals for parsing in the various components from the netlist
        bool is_component = (name[0] == 'R' || name[0] == 'C' || name[0] == 'L');                //support for C and L comes later
        bool is_source = (name[0] == 'I' || name[0] == 'V'); //support for current sources and voltage sources
	    bool is_nonlinear_component = (name[0] == 'D'); //support for diodes


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


        //Add components/sources to component vector (WITH SINE SOURCE SUPPORT)
        if (is_component){
            components.push_back(new basic_component(input[0], value, truenode1, truenode2, name));
        }else if (is_source){
            components.push_back(new source(input[0], source_type, truenode1, truenode2, name, frequency, value, amplitude));
        }else if (is_nonlinear_component){
	        components.push_back(new nonlinear_component(input[0], truenode1, truenode2, name, "ideal_diode"));
        }


        if (input[0]=='V'){
            voltage_source_counter++;
        } else if (input[0]=='C'){
            capacitor_counter++;
        } else if (input[0]=='D'){
            diode_checker=1;
        } else if (input[0]=='L'){
	        inductor_counter++;
	    }


        //Add nodes to the node vector
        node_vector = add_nodes_to_vector(truenode1, truenode2, components[component_counter], node_vector);
        component_counter += 1;
        input.clear();
    }


    /////////ARRAY STORAGE/////////
    const int h = node_vector.size()+voltage_source_counter+capacitor_counter+inductor_counter-1;
    const int w = node_vector.size()+voltage_source_counter+inductor_counter-1;
    MatrixXd g(h, h);
    MatrixXd goc(w, w);

    /////////RUNS TRANSIENT SIM//////////
    cout << "time";
    for(int b=0; b<node_vector.size();b++){
	if(node_vector[b]->return_ID()!=0){
	    cout << '\t' << "V(" << node_vector[b]->return_ID() << ")";
    	}
    }
    for(int c=0; c<components.size();c++){
          cout << '\t' << "I(" << components[c]->return_name() << ")";
    }

    cout << endl;
    for (long double t=0; t<=stop_time; t+=time_step){
        while (looper){
	    if(oc_looper){
                //SET Goc MATRIX TO 0
                for (int i=0; i<h; i++){
                    for(int j=0; j<w; j++){
                        goc(i,j) = 0;
                    }
                }

                //CREATES Goc MATRIX
                for (int i=0; i<components.size(); i++){
                    int node1_ID = components[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = components[i]->return_nodes()[1]->return_ID()-1;
                    if (components[i]->return_type()=='R'){
                        if (node1_ID!=-1){
                            goc(node1_ID, node1_ID) += 1/components[i]->return_value(0, 0);
                        }
                        if (node2_ID!=-1){
                            goc(node2_ID, node2_ID) += 1/components[i]->return_value(0, 0);
                        }
                        if (node1_ID!=-1 && node2_ID!=-1){
                            goc(node1_ID, node2_ID) -= 1/components[i]->return_value(0, 0);
                            goc(node2_ID, node1_ID) -= 1/components[i]->return_value(0, 0);
                        }
                    } else if (components[i]->return_type()=='D'){
                            components[i]->set_prev_cv(0);
                        if (node1_ID!=-1){
                            goc(node1_ID, node1_ID) += 1/components[i]->return_Req()->return_value(0, 0);
                        }
                        if (node2_ID!=-1){
                            goc(node2_ID, node2_ID) += 1/components[i]->return_Req()->return_value(0, 0);
                        }
                        if (node1_ID!=-1 && node2_ID!=-1){
                            goc(node1_ID, node2_ID) -= 1/components[i]->return_Req()->return_value(0, 0);
                            goc(node2_ID, node1_ID) -= 1/components[i]->return_Req()->return_value(0, 0);
                        }
                    } else if (components[i]->return_type()=='V' || components[i]->return_type()=='L'){
                        float position = stoi(components[i]->return_name().substr(1))+node_vector.size()-2;
                        if (components[i]->return_type()=='L'){
                            position += voltage_source_counter;
                        }
                        if (node1_ID!=-1){
                            goc(position, node1_ID) = 1;
                            goc(node1_ID, position) = 1;
                        }
                        if (node2_ID!=-1){
                            goc(position, node2_ID) = -1;
                            goc(node2_ID, position) = -1;
                        }
		    }
		}
	    /////////CALCULATE CURRENT MATRIX FOR OC//////////
            MatrixXd currentoc(w,1);
            vector<float> temp = find_currentoc(components, node_vector.size()-1, voltage_source_counter, inductor_counter, t);
            for(int f=0; f < temp.size(); f++){
                currentoc(f, 0) = temp[f];
            }

            //////////CALCULATE VOLTAGE MATRIX FOR OC//////////
            MatrixXd voc(w,1);
            voc = goc.fullPivLu().solve(currentoc);

            //////////SETTING DIODE INITIAL VD
            for (int i=0; i<components.size(); i++){
                if (components[i]->return_type() == 'D'){
		    float diode_v1 = 0;
		    float diode_v2 = 0;
                    if (components[i]->return_nodes()[0]->return_ID() != 0){
                        diode_v1 = voc((components[i]->return_nodes()[0]->return_ID()-1) , 0);
		    }
                    if (components[i]->return_nodes()[1]->return_ID() != 0){
		  	diode_v2 = voc((components[i]->return_nodes()[1]->return_ID()-1) , 0);
		    }
		    components[i]->set_prev_cv(diode_v2-diode_v1);
                }
            }

	    oc_looper = 0;
	    }


            if(cond){
                final_loop = 0;


                //SET G MATRIX TO 0
                for (int i=0; i<h; i++){
                    for(int j=0; j<w; j++){
                        g(i,j) = 0;
                    }
                }


                //CREATES G MATRIX
                for (int i=0; i<components.size(); i++){
                    int node1_ID = components[i]->return_nodes()[0]->return_ID()-1;
                    int node2_ID = components[i]->return_nodes()[1]->return_ID()-1;
                    if (components[i]->return_type()=='R'){
                        if (node1_ID!=-1){
                            g(node1_ID, node1_ID) += 1/components[i]->return_value(0, 0);
                        }
                        if (node2_ID!=-1){
                            g(node2_ID, node2_ID) += 1/components[i]->return_value(0, 0);
                        }
                        if (node1_ID!=-1 && node2_ID!=-1){
                            g(node1_ID, node2_ID) -= 1/components[i]->return_value(0, 0);
                            g(node2_ID, node1_ID) -= 1/components[i]->return_value(0, 0);
                        }
	            } else if (components[i]->return_type()=='D'){
                        if (node1_ID!=-1){
                            g(node1_ID, node1_ID) += 1/components[i]->return_Req()->return_value(0, 0);
                        }
                        if (node2_ID!=-1){
                            g(node2_ID, node2_ID) += 1/components[i]->return_Req()->return_value(0, 0);
                        }
                        if (node1_ID!=-1 && node2_ID!=-1){
                            g(node1_ID, node2_ID) -= 1/components[i]->return_Req()->return_value(0, 0);
                            g(node2_ID, node1_ID) -= 1/components[i]->return_Req()->return_value(0, 0);
                        }
                    } else if (components[i]->return_type()=='V' || components[i]->return_type()=='C'){
                        float position = stoi(components[i]->return_name().substr(1))+node_vector.size()-2;
                        if (components[i]->return_type()=='C'){
                            position += voltage_source_counter;
                        }
                        if (node1_ID!=-1){
                            g(position, node1_ID) = 1;
                            g(node1_ID, position) = 1;
                        }
                        if (node2_ID!=-1){
                            g(position, node2_ID) = -1;
                            g(node2_ID, position) = -1;
                        }
                    } else if (components[i]->return_type()=='L'){
                        float position = stoi(components[i]->return_name().substr(1))+node_vector.size()-2+voltage_source_counter+capacitor_counter;
                        g(position, position) = 1;
                        if(node1_ID!=-1){
                            g(position, node1_ID) = -time_step/(components[i]->return_value(0, 0));
                            g(node1_ID, position) = -1;
                        }
                        if(node2_ID!=-1){
                            g(position, node2_ID) = time_step/(components[i]->return_value(0, 0));
                            g(node2_ID, position) = 1;
                        }
		    }
                }

                if(!diode_checker){
                    cond = 0;
                    final_loop = 1;
                }else{
                    diode_counter++;
                }
                if(diode_counter == 5){
                    final_loop = 1;
		    oc_looper = 1;
                }
            }



            /////////CALCULATE CURRENT MATRIX//////////
            MatrixXd current(h,1);
            vector<float> temp = find_current(components, node_vector.size()-1, voltage_source_counter, capacitor_counter, inductor_counter, t, time_step, final_loop);
            for(int f=0; f < temp.size(); f++){
                current(f, 0) = temp[f];
            }


            //////////CALCULATE VOLTAGE MATRIX//////////
            MatrixXd v(h,1);
            v = g.fullPivLu().solve(current);
            if(final_loop){
                cout << t;
                for(int b=0; b<node_vector.size();b++){
                    if(node_vector[b]->return_ID()!=0){
                            cout << '\t' << v((node_vector[b]->return_ID()-1),0);
                    }
                }
	        }


            ////////INPUTTING NEW VALUES INTO PREV VARIABLES////////////////
            for (int i=0; i<components.size(); i++){
                ///////////SETTING CAPACTIOR PREVIOUS VALUES (CURRENT)//////////////
                if (components[i]->return_type()=='C' && final_loop){
                    if (components[i]->return_nodes()[0]->return_ID() != 0){
                        float cap_current = v((stoi(components[i]->return_name().substr(1))+node_vector.size()+voltage_source_counter-2), 0);
                        components[i]->set_prev_cv(cap_current);
                    }

                ///////////SETTING INDUCTOR PREVIOUS VALUES (VOLTAGE)///////////////
                }else if (components[i]->return_type() == 'L' && final_loop){
                    if (components[i]->return_nodes()[0]->return_ID() != 0){
                        float ind_current = v((stoi(components[i]->return_name().substr(1))+node_vector.size()+voltage_source_counter+capacitor_counter-2), 0);;
                        components[i]->set_tot_acc(ind_current);
		            }

                //////////SETTING DIODE PREVIOUS VALUES
                }else if (components[i]->return_type() == 'D' && !final_loop){
                    float diode_v1 = 0;
                    float diode_v2 = 0;
                    if (components[i]->return_nodes()[0]->return_ID() != 0){
                        diode_v1 = v((components[i]->return_nodes()[0]->return_ID()-1) , 0);
                    }
                    if (components[i]->return_nodes()[1]->return_ID() != 0){
                        diode_v2 = v((components[i]->return_nodes()[1]->return_ID()-1) , 0);
                    }
                    components[i]->set_prev_cv(diode_v2-diode_v1);
                }
            }


            for (int s=0; s<components.size(); s++){
                ///////////OUTPUTTING CAPACITOR CURRENT//////////////
                if (components[s]->return_type()=='C' && final_loop){
                    cout << '\t' << components[s]->return_prev_cv();

                ///////////OUTPUTTING INDUCTOR CURRENT///////////////
                }else if (components[s]->return_type() == 'L' && final_loop){
                    cout << '\t' << components[s]->return_tot_acc();

                ///////////OUTPUTTING RESISTOR CURRENT///////////////
                }else if (components[s]->return_type() == 'R' && final_loop){
                    float resistor_voltage1 = 0;
                    float resistor_voltage2 = 0;
                    if (components[s]->return_nodes()[1]->return_ID()-1 != -1){
                        resistor_voltage1 = v((components[s]->return_nodes()[1]->return_ID()-1), 0);
                    }
                    if (components[s]->return_nodes()[0]->return_ID()-1 != -1){
                        resistor_voltage2 = v((components[s]->return_nodes()[0]->return_ID()-1), 0);
       		    }
                    float resistor_voltage = resistor_voltage1-resistor_voltage2;
                    cout << '\t' << resistor_voltage * 1/(components[s]->return_value(t, final_loop));

                ///////////OUTPUTTING CURRENT SOURCE CURRENT///////////////
                }else if (components[s]->return_type() == 'I' && final_loop){
                    cout << '\t' << components[s]->return_value(t, final_loop);

                ///////////OUTPUTTING VOLTAGE SOURCE CURRENT///////////////
                }else if (components[s]->return_type() == 'V' && final_loop){
                    cout << '\t' << v(stoi(components[s]->return_name().substr(1)) + node_vector.size()-2, 0);

		///////////OUTPUTTING DIODE CURRENT///////////////////
		}else if (components[s]->return_type() == 'D' && final_loop){
                    float resistor_voltage1 = 0;
                    float resistor_voltage2 = 0;
                    if (components[s]->return_nodes()[1]->return_ID()-1 != -1){
                        resistor_voltage1 = v((components[s]->return_nodes()[1]->return_ID()-1), 0);
                    }
                    if (components[s]->return_nodes()[0]->return_ID()-1 != -1){
                        resistor_voltage2 = v((components[s]->return_nodes()[0]->return_ID()-1), 0);
                    }
                    float resistor_voltage = resistor_voltage1-resistor_voltage2;
                    cout << '\t' << resistor_voltage * 1/(components[s]->return_Req()->return_value(t, final_loop)) + components[s]->return_Ieq()->return_value(t, final_loop);

		}
            }


            cout << endl;
            
            cerr << "TIME: " << t << endl;
            cerr << "VOLTAGE " << endl << v << endl;
            cerr << "CURRENT " << endl << current << endl << endl << endl;
            
            if(final_loop){
                diode_counter = 0;
                break;
            }
	    }
    }
    cerr << "CONDUCTANCE MATRIX: " << endl << g << endl;
}
