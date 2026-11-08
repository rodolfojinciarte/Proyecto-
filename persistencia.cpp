#include "persistencia.h"
#include <fstream>    // ifstream (leer), ofstream (crear), fstream (leer y escribir)
#include <ctime>
using namespace std;

// ===================== HEADER =====================

// Si el archivo no existe, lo crea con su header inicial.
// Si ya existe, no lo toca. Devuelve true si quedo listo para usarse.
bool inicializarArchivo(const char* ruta) {
    ifstream prueba(ruta, ios::binary);
    if (prueba.is_open()) {        // ya existe: no se modifica
        prueba.close();
        return true;
    }
    ofstream nuevo(ruta, ios::binary);   // ofstream crea el archivo
    if (!nuevo.is_open()) {
        return false;                    // no se pudo crear (por ejemplo, no existe la carpeta datos)
    }
    ArchivoHeader h;
    h.cantidadRegistros = 0;
    h.proximoID = 1;
    h.registrosActivos = 0;
    h.version = 1;
    // write pide bytes: (char*)&h significa "toma la estructura h como una fila de bytes"
    nuevo.write((char*)&h, sizeof(ArchivoHeader));
    nuevo.close();
    return true;
}

// Lee el header (los primeros bytes) sin leer ningun registro
bool leerHeader(const char* ruta, ArchivoHeader* h) {
    ifstream f(ruta, ios::binary);
    if (!f.is_open()) {
        return false;
    }
    f.read((char*)h, sizeof(ArchivoHeader));
    bool bien = f.good();      // good() = la lectura salio completa
    f.close();
    return bien;
}

// Sobrescribe solo el header (posicion 0), sin tocar los registros
bool actualizarHeader(const char* ruta, ArchivoHeader h) {
    // in | out = abrir para leer Y escribir SIN borrar lo que ya tiene
    fstream f(ruta, ios::in | ios::out | ios::binary);
    if (!f.is_open()) {
        return false;
    }
    f.seekp(0);                // el cursor de escritura al byte 0
    f.write((char*)&h, sizeof(ArchivoHeader));
    f.close();
    return true;
}

// ===================== VEHICULOS =====================

// Formula de acceso aleatorio: el ID n esta en la casilla n - 1
long posicionVehiculo(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(Vehiculo);
}

// CREAR: leer header -> poner ID y fechas -> escribir al final -> actualizar header
Resultado guardarVehiculoNuevo(Vehiculo v, int* idAsignado) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    v.id = h.proximoID;
    v.eliminado = false;
    v.fechaCreacion = time(0);              // fecha y hora de ahora
    v.fechaUltimaModificacion = v.fechaCreacion;

    fstream f(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) {
        return ERROR_ARCHIVO;
    }
    f.seekp(0, ios::end);                   // el cursor al final del archivo
    f.write((char*)&v, sizeof(Vehiculo));
    f.close();

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    if (!actualizarHeader(RUTA_VEHICULOS, h)) {
        return ERROR_ARCHIVO;
    }
    *idAsignado = v.id;                     // le aviso al que llamo que ID quedo
    return OK;
}

// LEER POR ID: calcula la posicion y lee solo ese registro.
// Lo entrega aunque este eliminado: decidir si sirve es trabajo de la logica.
Resultado leerVehiculoPorID(int id, Vehiculo* v) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    if (id < 1 || id > h.cantidadRegistros) {
        return NO_EXISTE;                   // ese ID nunca se asigno
    }
    ifstream f(RUTA_VEHICULOS, ios::binary);
    if (!f.is_open()) {
        return ERROR_ARCHIVO;
    }
    f.seekg(posicionVehiculo(id));          // salto directo, sin recorrer
    f.read((char*)v, sizeof(Vehiculo));
    bool bien = f.good();
    f.close();
    if (!bien) {
        return ERROR_ARCHIVO;
    }
    return OK;
}

// SOBRESCRIBIR: escribe el registro en su misma posicion
Resultado sobrescribirVehiculo(Vehiculo v) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    if (v.id < 1 || v.id > h.cantidadRegistros) {
        return NO_EXISTE;
    }
    v.fechaUltimaModificacion = time(0);
    fstream f(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) {
        return ERROR_ARCHIVO;
    }
    f.seekp(posicionVehiculo(v.id));
    f.write((char*)&v, sizeof(Vehiculo));
    f.close();
    return OK;
}

// BORRADO LOGICO: igual que sobrescribir, con eliminado = true, y registrosActivos--
Resultado marcarVehiculoEliminado(int id) {
    Vehiculo v;
    Resultado r = leerVehiculoPorID(id, &v);
    if (r != OK) {
        return r;
    }
    if (v.eliminado) {
        return OK;            // ya estaba eliminado: no resto dos veces del header
    }
    v.eliminado = true;
    r = sobrescribirVehiculo(v);
    if (r != OK) {
        return r;
    }
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    h.registrosActivos--;
    if (!actualizarHeader(RUTA_VEHICULOS, h)) {
        return ERROR_ARCHIVO;
    }
    return OK;
}

// CONTAR ACTIVOS: lo dice el header, no hace falta recorrer
Resultado contarVehiculosActivos(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    *cantidad = h.registrosActivos;
    return OK;
}