#include <iostream>
#include <cstring>
using namespace std;

struct Carro {  
    char placa[10];
    int anio;

};

int main() {
    
int edad = 20;
cout << "tu edad es de:"<< edad << endl;
cout << "la direccion de tu edad es:" << &edad << endl;

int*p = &edad;

cout << "en p esta la direccion:" << p << endl;
cout << "P vale:" << *p << endl;

*p = 25;

cout << "ahora p vale:" << *p << endl;

Carro c;
strcpy ( c.placa, "AB123CD");

c.anio = 2015;

cout << "carro: " << c.placa << " del " << c.anio << endl;

Carro* pc = &c;


pc-> anio = 2020;

cout << " el nuevo ano de tu carro es:" << c.anio << endl;

return 0;

}

