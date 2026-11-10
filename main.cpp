#include <iostream>
#include <cstring>
#include "logica.h"
#include "presentacion.h"
#include "utilidades.h"
using namespace std;

// QUE HACE: punto de inicio del programa.
// 1. Prepara los archivos, 2. configura el taller si es la primera vez,
// 3. muestra el menu principal hasta que el usuario elige salir.
int main() {
    // ---------- 1. Crear los archivos .bin que falten (con su header inicial) ----------
    if (!prepararArchivos()) {
        cout << "ERROR: No se pudieron crear los archivos en la carpeta datos/." << endl;
        cout << "Verifique que la carpeta datos exista junto al programa." << endl;
        return 1;
    }

    // ---------- 2. Primer arranque: si no existe taller.bin, se piden los datos ----------
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

    // ---------- 3. Menu principal: se repite hasta que el usuario elige salir ----------
    bool salir = false;
    while (!salir) {
        // linea con el nombre del taller, leido de taller.bin
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

        // cada opcion llama a su submenu (capa de presentacion)
        int op = leerEntero("Seleccione una opcion: ", 0, 6);
        if (op == 1) menuVehiculos();
        if (op == 2) menuMecanicos();
        if (op == 3) menuRepuestos();
        if (op == 4) menuOrdenes();
               if (op == 5) menuReportes();
        if (op == 6) menuMantenimiento();
        if (op == 0 && confirmar("Seguro que desea salir?")) {
            salir = true;
        }
    }
    // En el Proyecto 2 los datos NO se pierden: ya estan en los archivos
    cout << "Hasta luego. Los datos quedaron guardados en datos/." << endl;
    return 0;
}
