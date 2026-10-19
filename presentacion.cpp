#include <iostream>
#include <iomanip>
#include <cstring>
#include "presentacion.h"
#include "logica.h"
#include "utilidades.h"
using namespace std;

// ===== VEHICULOS =====

// Muestra los datos de un solo vehiculo
void mostrarVehiculo(Vehiculo* v) {
    cout << "  ID " << v->id << " - " << v->placa << " - " << v->marca << " " << v->modelo
         << " " << v->anio << " (" << v->kilometraje << " km)" << endl;
    cout << "  Propietario: " << v->nombrePropietario << " (" << v->cedulaPropietario
         << ") Tel: " << v->telefonoPropietario << endl;
    cout << "  Registrado: " << v->fechaRegistro << endl;
}

// Muestra la tabla de vehiculos
void mostrarListaVehiculos(Vehiculo** vehiculos, int cantidad) {
    cout << left;
    cout << "  " << setw(4) << "ID" << setw(10) << "Placa" << setw(22) << "Vehiculo"
         << setw(6) << "Anio" << setw(10) << "Km" << "Propietario" << endl;
    cout << "  ---------------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        char nombreAuto[110];
        strcpy(nombreAuto, vehiculos[i]->marca);
        strcat(nombreAuto, " ");
        strcat(nombreAuto, vehiculos[i]->modelo);
        cout << "  " << setw(4) << vehiculos[i]->id << setw(10) << vehiculos[i]->placa
             << setw(22) << nombreAuto << setw(6) << vehiculos[i]->anio
             << setw(10) << vehiculos[i]->kilometraje << vehiculos[i]->nombrePropietario << endl;
    }
    cout << "  Total de vehiculos: " << cantidad << endl;
}

// Pide los datos y registra. Si escribe CANCELAR, no se crea nada
void menuRegistrarVehiculo(SistemaTaller* s) {
    Vehiculo v;
    cout << "\n(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Placa: ", v.placa, 10)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Marca: ", v.marca, 50)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Modelo: ", v.modelo, 50)) { cout << "Registro cancelado." << endl; return; }
    v.anio = leerEntero("Anio (1950-2027): ", 1950, 2027);
    v.kilometraje = leerEntero("Kilometraje: ", 0, 2000000);
    if (!leerTexto("Nombre del propietario: ", v.nombrePropietario, 100)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Cedula (V-12345678): ", v.cedulaPropietario, 20)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Telefono: ", v.telefonoPropietario, 20)) { cout << "Registro cancelado." << endl; return; }

    if (!esPlacaValida(v.placa)) {
        cout << "ERROR: La placa '" << v.placa << "' no es valida (6 o 7 letras/numeros)." << endl;
        return;
    }
    if (buscarVehiculoPorPlaca(s, v.placa) != nullptr) {
        cout << "ERROR: La placa '" << v.placa << "' ya esta registrada." << endl;
        return;
    }
    if (!esCedulaValida(v.cedulaPropietario)) {
        cout << "ERROR: La cedula '" << v.cedulaPropietario << "' no es valida." << endl;
        return;
    }
    if (!confirmar("Guardar el vehiculo?")) {
        cout << "Registro cancelado." << endl;
        return;
    }
    Vehiculo* nuevo = agregarVehiculo(s, v);
    if (nuevo == nullptr) {
        cout << "ERROR: No se pudo registrar el vehiculo." << endl;
    } else {
        cout << "Vehiculo registrado:" << endl;
        mostrarVehiculo(nuevo);
    }
}

// Busca por placa exacta o por propietario parcial
void menuBuscarVehiculo(SistemaTaller* s) {
    int opcion = leerEntero("1. Por placa  2. Por propietario: ", 1, 2);
    char texto[100];
    if (!leerTexto("Texto a buscar: ", texto, 100)) { return; }
    if (opcion == 1) {
        Vehiculo* v = buscarVehiculoPorPlaca(s, texto);
        if (v == nullptr) {
            cout << "ERROR: No existe un vehiculo con placa '" << texto << "'." << endl;
        } else {
            mostrarVehiculo(v);
        }
    } else {
        int cantidad;
        Vehiculo** lista = buscarVehiculosPorPropietario(s, texto, &cantidad);
        if (cantidad == 0) {
            cout << "No hay vehiculos de propietarios que contengan '" << texto << "'." << endl;
        } else {
            mostrarListaVehiculos(lista, cantidad);
        }
        delete[] lista;
    }
}

// Cambia marca, modelo, anio y datos del propietario
void menuActualizarVehiculo(SistemaTaller* s) {
    int id = leerEntero("ID del vehiculo: ", 1, 1000000);
    Vehiculo* actual = buscarVehiculoPorID(s, id);
    if (actual == nullptr) {
        cout << "ERROR: No existe el vehiculo con ID " << id << "." << endl;
        return;
    }
    mostrarVehiculo(actual);
    Vehiculo datos;
    if (!leerTexto("Nueva marca: ", datos.marca, 50)) { cout << "Cancelado." << endl; return; }
    if (!leerTexto("Nuevo modelo: ", datos.modelo, 50)) { cout << "Cancelado." << endl; return; }
    datos.anio = leerEntero("Nuevo anio (1950-2027): ", 1950, 2027);
    if (!leerTexto("Nuevo propietario: ", datos.nombrePropietario, 100)) { cout << "Cancelado." << endl; return; }
    if (!leerTexto("Nueva cedula: ", datos.cedulaPropietario, 20)) { cout << "Cancelado." << endl; return; }
    if (!leerTexto("Nuevo telefono: ", datos.telefonoPropietario, 20)) { cout << "Cancelado." << endl; return; }
    if (!confirmar("Guardar los cambios?")) {
        cout << "Cancelado." << endl;
        return;
    }
    if (actualizarVehiculo(s, id, datos)) {
        cout << "Vehiculo actualizado." << endl;
    } else {
        cout << "ERROR: La cedula '" << datos.cedulaPropietario << "' no es valida." << endl;
    }
}

// Elimina si no tiene ordenes
void menuEliminarVehiculo(SistemaTaller* s) {
    int id = leerEntero("ID del vehiculo: ", 1, 1000000);
    Vehiculo* v = buscarVehiculoPorID(s, id);
    if (v == nullptr) {
        cout << "ERROR: No existe el vehiculo con ID " << id << "." << endl;
        return;
    }
    int cantidad;
    OrdenServicio** ordenes = listarOrdenesPorVehiculo(s, id, &cantidad);
    delete[] ordenes;
    if (cantidad > 0) {
        cout << "ERROR: El vehiculo '" << v->placa << "' tiene " << cantidad
             << " ordenes registradas y no puede eliminarse." << endl;
        return;
    }
    char placa[10];
    strcpy(placa, v->placa);
    if (!confirmar("Seguro que desea eliminarlo?")) {
        cout << "Cancelado." << endl;
        return;
    }
    if (eliminarVehiculo(s, id)) {
        cout << "Vehiculo '" << placa << "' eliminado." << endl;
    }
}

// Lista todos los vehiculos
void menuListarVehiculos(SistemaTaller* s) {
    int cantidad;
    Vehiculo** lista = listarVehiculos(s, &cantidad);
    mostrarListaVehiculos(lista, cantidad);
    delete[] lista;
}

// Submenu de vehiculos
void menuVehiculos(SistemaTaller* s) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n╔═══════════════════════════════════════════╗" << endl;
        cout << "║          GESTIÓN DE VEHÍCULOS             ║" << endl;
        cout << "╠═══════════════════════════════════════════╣" << endl;
        cout << "║  1. Registrar vehículo                    ║" << endl;
        cout << "║  2. Buscar vehículo                       ║" << endl;
        cout << "║  3. Actualizar vehículo                   ║" << endl;
        cout << "║  4. Eliminar vehículo                     ║" << endl;
        cout << "║  5. Listar vehículos                      ║" << endl;
        cout << "║  0. Volver al menú principal              ║" << endl;
        cout << "╚═══════════════════════════════════════════╝" << endl;
        opcion = leerEntero("Seleccione una opción: ", 0, 5);
        if (opcion == 1) menuRegistrarVehiculo(s);
        if (opcion == 2) menuBuscarVehiculo(s);
        if (opcion == 3) menuActualizarVehiculo(s);
        if (opcion == 4) menuEliminarVehiculo(s);
        if (opcion == 5) menuListarVehiculos(s);
    }
}