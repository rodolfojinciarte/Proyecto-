#include <iostream>
#include <cstring>
#include "estructuras.h"
#include "logica.h"
#include "utilidades.h"
#include "presentacion.h"
using namespace std;

int main() {
    // 1. Datos del taller (se llenan una sola vez, el Taller no tiene CRUD)
    Taller datos;
    strcpy(datos.nombre, "Autoservicio El Lago");
    strcpy(datos.rif, "J-12345678-9");
    strcpy(datos.direccion, "Maracaibo");
    strcpy(datos.telefono, "0261-0000000");

    // 2. Abro el taller: compro las cajas con capacidad 4
    SistemaTaller sistema;
    inicializarSistema(&sistema, datos);

    // 3. Menu principal: se repite hasta que el usuario elija salir
    bool salir = false;
    while (!salir) {
        // Bordes con + = | para que se vean bien en cualquier terminal
        cout << "\n+===========================================+" << endl;
        cout << "|        SISTEMA DE GESTION DE TALLER       |" << endl;
        cout << "|  Taller: " << sistema.taller.nombre << endl;
        cout << "+===========================================+" << endl;
        cout << "|  1. Gestion de Vehiculos                  |" << endl;
        cout << "|  2. Gestion de Mecanicos                  |" << endl;
        cout << "|  3. Inventario de Repuestos               |" << endl;
        cout << "|  4. Ordenes de Servicio                   |" << endl;
        cout << "|  0. Salir                                 |" << endl;
        cout << "+===========================================+" << endl;

        // leerEntero no deja escribir letras ni numeros fuera de 0 a 4
        int opcion = leerEntero("Seleccione una opcion: ", 0, 4);

        // Cada opcion llama a su submenu (capa de presentacion)
        if (opcion == 1) menuVehiculos(&sistema);
        if (opcion == 2) menuMecanicos(&sistema);
        if (opcion == 3) cout << "Proximamente." << endl;
        if (opcion == 4) cout << "Proximamente." << endl;

        // El PDF pide confirmar antes de salir porque los datos se pierden
        if (opcion == 0) {
            if (confirmar("Seguro que desea salir? Los datos se perderan")) {
                salir = true;
            }
        }
    }

    // 4. Cierro el taller: devuelvo toda la memoria
    liberarSistema(&sistema);
    cout << "Hasta luego." << endl;
    return 0;
}