#include <iostream>
#include <cstring>
#include "estructuras.h"
#include "logica.h"
using namespace std;

int main() {
    // 1. Datos del taller
    Taller datos;
    strcpy(datos.nombre, "Autoservicio");
    strcpy(datos.rif, "J-12345678-9");
    strcpy(datos.direccion, "Maracaibo");
    strcpy(datos.telefono, "0424-6700838");

    // 2. Abro el taller
    SistemaTaller sistema;
    inicializarSistema(&sistema, datos);

    cout << "Taller: " << sistema.taller.nombre << endl;
    cout << "Caben " << sistema.capacidadVehiculos << " vehiculos, hay " << sistema.numVehiculos << endl;
    cout << "Proximo ID de vehiculo: " << sistema.siguienteIdVehiculo << endl;

    // 3. Pruebo una mudanza
    redimensionarVehiculos(&sistema);
    cout << "Despues de la mudanza caben: " << sistema.capacidadVehiculos << endl;

    // 4. Cierro el taller
    liberarSistema(&sistema);
    cout << "Taller cerrado. Memoria devuelta." << endl;

    return 0;
}