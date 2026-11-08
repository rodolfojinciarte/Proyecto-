#include "persistencia.h"
#include <fstream>    // ifstream (leer), ofstream (crear), fstream (leer y escribir)
#include <ctime>
using namespace std;

// Para crear carpetas: en Windows se usa _mkdir; en Linux, mkdir.
// #ifdef elige cual incluir segun el sistema operativo.
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

// =====================================================================
// HEADER
// =====================================================================

// QUE HACE: si el archivo no existe, lo crea con su header inicial
// (0 registros, proximo ID 1, 0 activos, version 1). Si ya existe, no lo toca.
// DEVUELVE: true si el archivo quedo listo para usarse.
bool inicializarArchivo(const char* ruta) {
    // 1. Intento abrirlo para leer: si abre, ya existe
    ifstream prueba(ruta, ios::binary);
    if (prueba.is_open()) {
        prueba.close();
        return true;
    }
    // 2. No existe: lo creo (ofstream crea el archivo)
    ofstream nuevo(ruta, ios::binary);
    if (!nuevo.is_open()) {
        return false;                    // no se pudo crear (por ejemplo, no existe la carpeta datos)
    }
    // 3. Escribo el header inicial
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

// QUE HACE: lee el header (los primeros 16 bytes) sin leer ningun registro.
// DEVUELVE: true si lo pudo leer completo.
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

// QUE HACE: sobrescribe solo el header (posicion 0), sin tocar los registros.
// DEVUELVE: true si lo pudo escribir.
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

// =====================================================================
// VEHICULOS
// =====================================================================

// QUE HACE: calcula en que byte empieza el vehiculo con ese ID.
// Formula de acceso aleatorio: el ID n esta en la casilla n - 1.
long posicionVehiculo(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(Vehiculo);
}

// QUE HACE (CREAR): lee el header, pone ID y fechas, escribe al final
// y actualiza el header. Le dice al que llama el ID asignado.
Resultado guardarVehiculoNuevo(Vehiculo v, int* idAsignado) {
    // 1. Leo el header para saber el proximo ID
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    // 2. ID, eliminado = false y marcas de tiempo
    v.id = h.proximoID;
    v.eliminado = false;
    v.fechaCreacion = time(0);              // fecha y hora de ahora
    v.fechaUltimaModificacion = v.fechaCreacion;

    // 3. Escribo el registro al final del archivo
    fstream f(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!f.is_open()) {
        return ERROR_ARCHIVO;
    }
    f.seekp(0, ios::end);                   // el cursor al final del archivo
    f.write((char*)&v, sizeof(Vehiculo));
    f.close();

    // 4. Actualizo el header
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    if (!actualizarHeader(RUTA_VEHICULOS, h)) {
        return ERROR_ARCHIVO;
    }
    *idAsignado = v.id;                     // le aviso al que llamo que ID quedo
    return OK;
}

// QUE HACE (LEER POR ID): calcula la posicion y lee SOLO ese registro, sin recorrer.
// Lo entrega aunque este eliminado: decidir si sirve es trabajo de la logica.
Resultado leerVehiculoPorID(int id, Vehiculo* v) {
    // 1. Reviso que el ID exista segun el header
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    if (id < 1 || id > h.cantidadRegistros) {
        return NO_EXISTE;                   // ese ID nunca se asigno
    }
    // 2. Salto directo a su posicion y lo leo
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

// QUE HACE (SOBRESCRIBIR): escribe el registro en su misma posicion
// y actualiza su fecha de ultima modificacion.
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

// QUE HACE (BORRADO LOGICO): igual que sobrescribir, con eliminado = true,
// y resta 1 a registrosActivos en el header.
Resultado marcarVehiculoEliminado(int id) {
    // 1. Leo el registro
    Vehiculo v;
    Resultado r = leerVehiculoPorID(id, &v);
    if (r != OK) {
        return r;
    }
    if (v.eliminado) {
        return OK;            // ya estaba eliminado: no resto dos veces del header
    }
    // 2. Lo marco y lo reescribo en su lugar
    v.eliminado = true;
    r = sobrescribirVehiculo(v);
    if (r != OK) {
        return r;
    }
    // 3. Actualizo el header
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

// QUE HACE (CONTAR ACTIVOS): lo dice el header, no hace falta recorrer.
Resultado contarVehiculosActivos(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    *cantidad = h.registrosActivos;
    return OK;
}

// =====================================================================
// MECANICOS
// (mismo patron que vehiculos: solo cambia el tipo y la ruta)
// =====================================================================

// QUE HACE: byte donde empieza el mecanico con ese ID
long posicionMecanico(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(Mecanico);
}

// QUE HACE (CREAR): ID y fechas, escribe al final, actualiza el header
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

// QUE HACE (LEER POR ID): salta a su posicion y lo lee (activo o de baja)
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

// QUE HACE (SOBRESCRIBIR): lo reescribe en su misma posicion
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

// QUE HACE (BORRADO LOGICO = dar de baja): eliminado = true y activos--
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

// QUE HACE (CONTAR ACTIVOS): desde el header
Resultado contarMecanicosActivos(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) return ERROR_ARCHIVO;
    *cantidad = h.registrosActivos;
    return OK;
}

// =====================================================================
// REPUESTOS
// (mismo patron que vehiculos)
// =====================================================================

// QUE HACE: byte donde empieza el repuesto con ese ID
long posicionRepuesto(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(Repuesto);
}

// QUE HACE (CREAR): ID y fechas, escribe al final, actualiza el header
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

// QUE HACE (LEER POR ID): salta a su posicion y lo lee (activo o eliminado)
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

// QUE HACE (SOBRESCRIBIR): lo reescribe en su misma posicion
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

// QUE HACE (BORRADO LOGICO): eliminado = true y activos--
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

// QUE HACE (CONTAR ACTIVOS): desde el header
Resultado contarRepuestosActivos(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) return ERROR_ARCHIVO;
    *cantidad = h.registrosActivos;
    return OK;
}

// =====================================================================
// ORDENES
// Cada orden ya trae adentro sus 15 lineas, asi que al leer
// una orden vienen sus lineas, sin tocar las demas ordenes.
// =====================================================================

// QUE HACE: byte donde empieza la orden con ese ID
long posicionOrden(int id) {
    return sizeof(ArchivoHeader) + (long)(id - 1) * sizeof(OrdenServicio);
}

// QUE HACE (CREAR): ID y fechas, escribe al final, actualiza el header
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

// QUE HACE (LEER POR ID): salta a su posicion y la lee, con sus lineas
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

// QUE HACE (SOBRESCRIBIR): la reescribe en su misma posicion
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

// QUE HACE (BORRADO LOGICO): el PDF pide las 5 operaciones para todas
// las entidades, aunque la logica nunca elimina una orden (se anulan).
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

// QUE HACE (CONTAR ACTIVAS): desde el header
Resultado contarOrdenesActivas(int* cantidad) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) return ERROR_ARCHIVO;
    *cantidad = h.registrosActivos;
    return OK;
}

// =====================================================================
// TALLER
// taller.bin es la excepcion: un solo registro y SIN header.
// =====================================================================

// QUE HACE: dice si taller.bin ya existe (si no, es el primer arranque)
bool existeTaller() {
    ifstream f(RUTA_TALLER, ios::binary);
    bool existe = f.is_open();
    f.close();
    return existe;
}

// QUE HACE: escribe (o reemplaza) el unico registro del taller
bool guardarTaller(Taller t) {
    t.fechaUltimaModificacion = time(0);
    ofstream f(RUTA_TALLER, ios::binary);    // ofstream crea o reemplaza el archivo
    if (!f.is_open()) return false;
    f.write((char*)&t, sizeof(Taller));
    f.close();
    return true;
}

// QUE HACE: lee el unico registro del taller
bool leerTaller(Taller* t) {
    ifstream f(RUTA_TALLER, ios::binary);
    if (!f.is_open()) return false;
    f.read((char*)t, sizeof(Taller));
    bool bien = f.good();
    f.close();
    return bien;
}

// =====================================================================
// ARRANQUE
// =====================================================================

// QUE HACE: al iniciar el programa, crea los archivos que falten.
// DEVUELVE: true solo si los 4 quedaron listos (&& = "y").
bool inicializarArchivos() {
    return inicializarArchivo(RUTA_VEHICULOS)
        && inicializarArchivo(RUTA_MECANICOS)
        && inicializarArchivo(RUTA_REPUESTOS)
        && inicializarArchivo(RUTA_ORDENES);
}

// =====================================================================
// MANTENIMIENTO
// =====================================================================

// QUE HACE: cuenta los registros por el TAMANO del archivo, sin confiar en el header:
// (bytes del archivo - bytes del header) / bytes de cada registro.
// Sirve para descubrir si el header dice una cantidad distinta a la real.
Resultado contarRegistrosEnArchivo(const char* ruta, long tamRegistro, int* cantidad) {
    ifstream f(ruta, ios::binary);
    if (!f.is_open()) return ERROR_ARCHIVO;
    f.seekg(0, ios::end);              // cursor al final
    long tam = (long)f.tellg();        // tellg = en que byte esta el cursor = tamano del archivo
    f.close();
    long datos = tam - (long)sizeof(ArchivoHeader);
    *cantidad = (datos > 0) ? (int)(datos / tamRegistro) : 0;
    return OK;
}

// QUE HACE: crea una carpeta (la usa el backup)
Resultado crearCarpeta(const char* ruta) {
#ifdef _WIN32
    int r = _mkdir(ruta);
#else
    int r = mkdir(ruta, 0777);
#endif
    return (r == 0) ? OK : ERROR_ARCHIVO;   // 0 = se creo bien
}

// QUE HACE: copia un archivo BYTE A BYTE: lee un byte del original
// y lo escribe en la copia, hasta llegar al final.
Resultado copiarArchivo(const char* origen, const char* destino) {
    ifstream entrada(origen, ios::binary);
    if (!entrada.is_open()) return ERROR_ARCHIVO;
    ofstream salida(destino, ios::binary);
    if (!salida.is_open()) return ERROR_ARCHIVO;
    char byte;
    while (entrada.get(byte)) {        // get lee un byte; da false al llegar al final
        salida.put(byte);              // put escribe ese byte
    }
    entrada.close();
    salida.close();
    return OK;
}
