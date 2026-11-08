#include <iostream>
#include <cstring>
#include "logica.h"
#include "presentacion.h"
#include "utilidades.h"
using namespace std;

int main() {
    // 1. Crear los archivos .bin que falten (con su header inicial)
    if (!prepararArchivos()) {
        cout << "ERROR: No se pudieron crear los archivos en la carpeta datos/." << endl;
        cout << "Verifique que la carpeta datos exista junto al programa." << endl;
        return 1;
    }

    // 2. Primer arranque: si no existe taller.bin, se piden los datos del taller
    if (!tallerConfigurado()) {
        if (!menuPrimerArranque()) {
            cout << "No se configuro el taller. Saliendo." << endl;
            return 1;
        }
    }
    Taller t;
    if (obtenerTaller(&t) != OK) {
        cout << "ERROR: No se pudo leer datos/taller.bin" << endl;
        return 1;
    }

    // 3. Menu principal
    bool salir = false;
    while (!salir) {
        char lineaTaller[120];
        strcpy(lineaTaller, "Taller: ");
        strcat(lineaTaller, t.nombre);

        encabezado("SISTEMA DE GESTION DE TALLER");
        opcion(lineaTaller);
        pie();
        opcion("1. Gestion de Vehiculos");
        opcion("2. Gestion de Mecanicos");
        opcion("3. Inventario de Repuestos");
        opcion("4. Ordenes de Servicio");
        opcion("5. Reportes");
        opcion("6. Mantenimiento");
        opcion("0. Salir");
        pie();

        int op = leerEntero("Seleccione una opcion: ", 0, 6);
        if (op == 1) menuVehiculos();
        if (op == 2) cout << "Proximamente." << endl;   // ronda 2
        if (op == 3) cout << "Proximamente." << endl;   // ronda 2
        if (op == 4) cout << "Proximamente." << endl;   // ronda 3
        if (op == 5) cout << "Proximamente." << endl;   // lunes
        if (op == 6) cout << "Proximamente." << endl;   // lunes
        if (op == 0 && confirmar("Seguro que desea salir?")) {
            salir = true;
        }
    }
    // En el Proyecto 2 los datos NO se pierden: ya estan en los archivos
    cout << "Hasta luego. Los datos quedaron guardados en datos/." << endl;
    return 0;
}