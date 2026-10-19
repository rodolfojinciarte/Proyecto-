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

// Submenu de vehiculos: se repite hasta que elija 0
void menuVehiculos(SistemaTaller* s) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n+===========================================+" << endl;
        cout << "|           GESTION DE VEHICULOS            |" << endl;
        cout << "+===========================================+" << endl;
        cout << "|  1. Registrar vehiculo                    |" << endl;
        cout << "|  2. Buscar vehiculo                       |" << endl;
        cout << "|  3. Actualizar vehiculo                   |" << endl;
        cout << "|  4. Eliminar vehiculo                     |" << endl;
        cout << "|  5. Listar vehiculos                      |" << endl;
        cout << "|  0. Volver al menu principal              |" << endl;
        cout << "+===========================================+" << endl;
        opcion = leerEntero("Seleccione una opcion: ", 0, 5);

        // Cada numero llama a su funcion de menu
        if (opcion == 1) menuRegistrarVehiculo(s);
        if (opcion == 2) menuBuscarVehiculo(s);
        if (opcion == 3) menuActualizarVehiculo(s);
        if (opcion == 4) menuEliminarVehiculo(s);
        if (opcion == 5) menuListarVehiculos(s);
    }
}

// ===== MECANICOS =====

// Muestra un mecanico con sus ordenes abiertas
void mostrarMecanico(SistemaTaller* s, Mecanico* m) {
    cout << "  ID " << m->id << " - " << m->nombre << " (" << m->especialidad << ")" << endl;
    cout << "  Cedula: " << m->cedula << "  Ingreso: " << m->fechaIngreso << endl;
    // contarOrdenesAbiertas viene de la capa de logica
    cout << "  Ordenes abiertas: " << contarOrdenesAbiertas(s, m->id) << endl;
}

// Pide los datos del mecanico y lo registra
void menuRegistrarMecanico(SistemaTaller* s) {
    Mecanico m;
    cout << "\n(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Nombre: ", m.nombre, 100)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Cedula: ", m.cedula, 20)) { cout << "Registro cancelado." << endl; return; }

    // La especialidad se elige con un numero, asi nunca se escribe mal
    cout << "Especialidad: 1.MOTOR 2.FRENOS 3.ELECTRICIDAD 4.SUSPENSION 5.GENERAL" << endl;
    int esp = leerEntero("Opcion: ", 1, 5);
    const char* nombres[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    strcpy(m.especialidad, nombres[esp - 1]);  // -1 porque el arreglo empieza en 0

    // Validaciones con mensajes que dicen que fallo y con que dato
    if (!esCedulaValida(m.cedula)) {
        cout << "ERROR: La cedula '" << m.cedula << "' no es valida." << endl;
        return;
    }
    if (buscarMecanicoPorCedula(s, m.cedula) != nullptr) {
        cout << "ERROR: La cedula '" << m.cedula << "' ya esta registrada." << endl;
        return;
    }
    if (!confirmar("Guardar el mecanico?")) {
        cout << "Registro cancelado." << endl;
        return;
    }
    Mecanico* nuevo = agregarMecanico(s, m);
    if (nuevo == nullptr) {
        cout << "ERROR: No se pudo registrar el mecanico." << endl;
    } else {
        cout << "Mecanico registrado:" << endl;
        mostrarMecanico(s, nuevo);
    }
}

// Busca un mecanico por su ID
void menuBuscarMecanico(SistemaTaller* s) {
    int id = leerEntero("ID del mecanico: ", 1, 1000000);
    Mecanico* m = buscarMecanicoPorID(s, id);
    if (m == nullptr) {
        cout << "ERROR: No existe el mecanico con ID " << id << "." << endl;
    } else {
        mostrarMecanico(s, m);
    }
}

// Lista todos los mecanicos con sus ordenes ABIERTAS (lo pide el PDF)
void menuListarMecanicos(SistemaTaller* s) {
    int cantidad;
    Mecanico** lista = listarMecanicos(s, &cantidad);
    cout << left;
    cout << "  " << setw(4) << "ID" << setw(25) << "Nombre" << setw(14) << "Especialidad"
         << "Abiertas" << endl;
    cout << "  ---------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << "  " << setw(4) << lista[i]->id << setw(25) << lista[i]->nombre
             << setw(14) << lista[i]->especialidad
             << contarOrdenesAbiertas(s, lista[i]->id) << endl;
    }
    cout << "  Total de mecanicos: " << cantidad << endl;
    delete[] lista;  // libero solo la libreta, no los mecanicos
}

// Submenu de mecanicos: se repite hasta que elija 0
void menuMecanicos(SistemaTaller* s) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n+===========================================+" << endl;
        cout << "|           GESTION DE MECANICOS            |" << endl;
        cout << "+===========================================+" << endl;
        cout << "|  1. Registrar mecanico                    |" << endl;
        cout << "|  2. Buscar mecanico                       |" << endl;
        cout << "|  3. Listar mecanicos                      |" << endl;
        cout << "|  0. Volver al menu principal              |" << endl;
        cout << "+===========================================+" << endl;
        opcion = leerEntero("Seleccione una opcion: ", 0, 3);
        if (opcion == 1) menuRegistrarMecanico(s);
        if (opcion == 2) menuBuscarMecanico(s);
        if (opcion == 3) menuListarMecanicos(s);
    }
}