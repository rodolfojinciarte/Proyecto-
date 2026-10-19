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
    strcpy(datos.telefono, "0261-0000000");

    // 2. Abro el taller
    SistemaTaller sistema;
    inicializarSistema(&sistema, datos);

    // 3. Registro 5 vehiculos (el quinto obliga a mudarse)
    const char* placas[5] = {"AB123CD", "XY987ZW", "CC111AA", "DD222BB", "EE333CC"};
    for (int i = 0; i < 5; i++) {
        Vehiculo v;
        strcpy(v.placa, placas[i]);
        strcpy(v.marca, "Toyota");
        strcpy(v.modelo, "Corolla");
        v.anio = 2015;
        v.kilometraje = 45000;
        strcpy(v.nombrePropietario, "Maria Gonzalez");
        strcpy(v.cedulaPropietario, "V-12345678");
        strcpy(v.telefonoPropietario, "0414-0000000");
        agregarVehiculo(&sistema, v);
    }

    // 4. Muestro la lista
    int cantidad;
    Vehiculo** lista = listarVehiculos(&sistema, &cantidad);
    for (int i = 0; i < cantidad; i++) {
        cout << lista[i]->id << " " << lista[i]->placa << " " << lista[i]->fechaRegistro << endl;
    }
    delete[] lista;

    cout << "Caben: " << sistema.capacidadVehiculos << endl;

    // 5. Cierro el taller
    liberarSistema(&sistema);
    cout << "Taller cerrado. Memoria devuelta." << endl;
    return 0;
}