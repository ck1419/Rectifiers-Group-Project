#include <string>
#ifndef scientific_converter_hpp
#define scientific_converter_hpp



float scientific_converter(string input){
    char exponential = 'z';
    float multiplier = 1;
    if (isalpha(input.back())){
        exponential = tolower(input.back());
        input.pop_back();
    }
    if (exponential == 'p'){
        multiplier = 1e-12;
    }else if (exponential == 'n'){
        multiplier = 1e-9;
    }else if (exponential == 'u'){
        multiplier = 1e-6;
    }else if (exponential == 'm'){
        multiplier = 1e-3;
    }else if (exponential == 'k'){
        multiplier = 1e3;
    }else if (exponential == 'a'){       //FROM "Mega"
        multiplier = 1e6;
        input.pop_back();
        input.pop_back();
        input.pop_back();
    }else if (exponential == 'g'){
        multiplier = 1e9;
    }else if (exponential == 't'){
        multiplier = 1e12;
    }
    return stof(input) * multiplier;
}



#endif