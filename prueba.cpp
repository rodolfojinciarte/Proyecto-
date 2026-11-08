#include <iostream>
#include <cstring>
#include "persistencia.h"
using namespace std;

int main() {
    cout << "sizeof(ArchivoHeader) = " << sizeof(ArchivoHeader) << endl;
    cout << "sizeof(Vehiculo)      = " << sizeof(Vehiculo) << endl;

    if (!inicializarArchivo(RUTA_VEHICULOS)) {
        cout << "No se pudo crear datos/vehiculos.bin" << endl;
        return 1;
    }

    // Guardo un vehiculo nuevo
    Vehiculo v = {};   // {} = todos los campos en cero
    strcpy(v.placa, "AB123CD");
    strcpy(v.marca, "Toyota");
    strcpy(v.modelo, "Corolla");
    v.anio = 2015;
    v.kilometraje = 45000;
    int id = 0;
    if (guardarVehiculoNuevo(v, &id) == OK) {
        cout << "Guardado con ID " << id << endl;
    }

    // Lo leo de vuelta, saltando directo a su posicion
    Vehiculo leido;
    if (leerVehiculoPorID(id, &leido) == OK) {
        cout << "Leido en el byte " << posicionVehiculo(id)
             << ": " << leido.placa << " " << leido.marca << endl;
    }

    int activos = 0;
    contarVehiculosActivos(&activos);
    cout << "Vehiculos activos en el archivo: " << activos << endl;
    return 0;
}