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

// ===================== MECANICOS =====================
// (mismo patron que vehiculos: solo cambia el tipo y la ruta)

long posicionMecanico(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(Mecanico);
}

Resultado guardarMecanicoNuevo(Mecanico m, int* idAsignado) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) return ERROR_ARCHIVO;
    m.id = h.proximoID;
    m.eliminado = false;
    m.fechaCreacion = time(0);
    m.fechaUltimaModificacion = m.fechaCreacion;
    fstream f(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekp(0, ios::end);
    f.write((char*)&m, sizeof(Mecanico));
    f.close();
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    if (!actualizarHeader(RUTA_MECANICOS, h)) return ERROR_ARCHIVO;
    *idAsignado = m.id;
    return OK;
}

Resultado leerMecanicoPorID(int id, Mecanico* m) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) return ERROR_ARCHIVO;
    if (id < 1 || id > h.cantidadRegistros) return NO_EXISTE;
    ifstream f(RUTA_MECANICOS, ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekg(posicionMecanico(id));
    f.read((char*)m, sizeof(Mecanico));
    bool bien = f.good();
    f.close();
    return bien ? OK : ERROR_ARCHIVO;   // si bien es true -> OK, si no -> ERROR_ARCHIVO
}

Resultado sobrescribirMecanico(Mecanico m) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) return ERROR_ARCHIVO;
    if (m.id < 1 || m.id > h.cantidadRegistros) return NO_EXISTE;
    m.fechaUltimaModificacion = time(0);
    fstream f(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekp(posicionMecanico(m.id));
    f.write((char*)&m, sizeof(Mecanico));
    f.close();
    return OK;
}

Resultado marcarMecanicoEliminado(int id) {
    Mecanico m;
    Resultado r = leerMecanicoPorID(id, &m);
    if (r != OK) return r;
    if (m.eliminado) return OK;          // ya estaba dado de baja
    m.eliminado = true;
    r = sobrescribirMecanico(m);
    if (r != OK) return r;
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) return ERROR_ARCHIVO;
    h.registrosActivos--;
    if (!actualizarHeader(RUTA_MECANICOS, h)) return ERROR_ARCHIVO;
    return OK;
}

Resultado contarMecanicosActivos(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) return ERROR_ARCHIVO;
    *cantidad = h.registrosActivos;
    return OK;
}

// ===================== REPUESTOS =====================

long posicionRepuesto(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(Repuesto);
}

Resultado guardarRepuestoNuevo(Repuesto r, int* idAsignado) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) return ERROR_ARCHIVO;
    r.id = h.proximoID;
    r.eliminado = false;
    r.fechaCreacion = time(0);
    r.fechaUltimaModificacion = r.fechaCreacion;
    fstream f(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekp(0, ios::end);
    f.write((char*)&r, sizeof(Repuesto));
    f.close();
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    if (!actualizarHeader(RUTA_REPUESTOS, h)) return ERROR_ARCHIVO;
    *idAsignado = r.id;
    return OK;
}

Resultado leerRepuestoPorID(int id, Repuesto* r) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) return ERROR_ARCHIVO;
    if (id < 1 || id > h.cantidadRegistros) return NO_EXISTE;
    ifstream f(RUTA_REPUESTOS, ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekg(posicionRepuesto(id));
    f.read((char*)r, sizeof(Repuesto));
    bool bien = f.good();
    f.close();
    return bien ? OK : ERROR_ARCHIVO;
}

Resultado sobrescribirRepuesto(Repuesto r) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) return ERROR_ARCHIVO;
    if (r.id < 1 || r.id > h.cantidadRegistros) return NO_EXISTE;
    r.fechaUltimaModificacion = time(0);
    fstream f(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekp(posicionRepuesto(r.id));
    f.write((char*)&r, sizeof(Repuesto));
    f.close();
    return OK;
}

Resultado marcarRepuestoEliminado(int id) {
    Repuesto r;
    Resultado res = leerRepuestoPorID(id, &r);
    if (res != OK) return res;
    if (r.eliminado) return OK;
    r.eliminado = true;
    res = sobrescribirRepuesto(r);
    if (res != OK) return res;
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) return ERROR_ARCHIVO;
    h.registrosActivos--;
    if (!actualizarHeader(RUTA_REPUESTOS, h)) return ERROR_ARCHIVO;
    return OK;
}

Resultado contarRepuestosActivos(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) return ERROR_ARCHIVO;
    *cantidad = h.registrosActivos;
    return OK;
}

// ===================== ORDENES =====================
// Cada orden ya trae adentro sus 15 lineas, asi que al leer
// una orden vienen sus lineas, sin tocar las demas ordenes.

long posicionOrden(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(OrdenServicio);
}

Resultado guardarOrdenNueva(OrdenServicio o, int* idAsignado) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) return ERROR_ARCHIVO;
    o.id = h.proximoID;
    o.eliminado = false;
    o.fechaCreacion = time(0);
    o.fechaUltimaModificacion = o.fechaCreacion;
    fstream f(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekp(0, ios::end);
    f.write((char*)&o, sizeof(OrdenServicio));
    f.close();
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    if (!actualizarHeader(RUTA_ORDENES, h)) return ERROR_ARCHIVO;
    *idAsignado = o.id;
    return OK;
}

Resultado leerOrdenPorID(int id, OrdenServicio* o) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) return ERROR_ARCHIVO;
    if (id < 1 || id > h.cantidadRegistros) return NO_EXISTE;
    ifstream f(RUTA_ORDENES, ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekg(posicionOrden(id));
    f.read((char*)o, sizeof(OrdenServicio));
    bool bien = f.good();
    f.close();
    return bien ? OK : ERROR_ARCHIVO;
}

Resultado sobrescribirOrden(OrdenServicio o) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) return ERROR_ARCHIVO;
    if (o.id < 1 || o.id > h.cantidadRegistros) return NO_EXISTE;
    o.fechaUltimaModificacion = time(0);
    fstream f(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekp(posicionOrden(o.id));
    f.write((char*)&o, sizeof(OrdenServicio));
    f.close();
    return OK;
}

// El PDF pide las 5 operaciones para todas las entidades,
// aunque la logica nunca va a eliminar una orden (se anulan).
Resultado marcarOrdenEliminada(int id) {
    OrdenServicio o;
    Resultado r = leerOrdenPorID(id, &o);
    if (r != OK) return r;
    if (o.eliminado) return OK;
    o.eliminado = true;
    r = sobrescribirOrden(o);
    if (r != OK) return r;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) return ERROR_ARCHIVO;
    h.registrosActivos--;
    if (!actualizarHeader(RUTA_ORDENES, h)) return ERROR_ARCHIVO;
    return OK;
}

Resultado contarOrdenesActivas(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) return ERROR_ARCHIVO;
    *cantidad = h.registrosActivos;
    return OK;
}

// ===================== TALLER =====================
// taller.bin es la excepcion: un solo registro y SIN header.

bool existeTaller() {
    ifstream f(RUTA_TALLER, ios::binary);
    bool existe = f.is_open();
    f.close();
    return existe;
}

bool guardarTaller(Taller t) {
    t.fechaUltimaModificacion = time(0);
    ofstream f(RUTA_TALLER, ios::binary);    // ofstream crea o reemplaza el archivo
    if (!f.is_open()) return false;
    f.write((char*)&t, sizeof(Taller));
    f.close();
    return true;
}

bool leerTaller(Taller* t) {
    ifstream f(RUTA_TALLER, ios::binary);
    if (!f.is_open()) return false;
    f.read((char*)t, sizeof(Taller));
    bool bien = f.good();
    f.close();
    return bien;
}

// ===================== ARRANQUE =====================
// Al iniciar el programa: crea los archivos que falten.
bool inicializarArchivos() {
    return inicializarArchivo(RUTA_VEHICULOS)
        && inicializarArchivo(RUTA_MECANICOS)
        && inicializarArchivo(RUTA_REPUESTOS)
        && inicializarArchivo(RUTA_ORDENES);
    // && = "y": devuelve true solo si los 4 quedaron listos
}