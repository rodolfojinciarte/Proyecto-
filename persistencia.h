#pragma once
#include "estructuras.h"
#include "resultados.h"

// ===================== RUTAS DE LOS ARCHIVOS =====================
const char RUTA_TALLER[]     = "datos/taller.bin";
const char RUTA_VEHICULOS[]  = "datos/vehiculos.bin";
const char RUTA_MECANICOS[]  = "datos/mecanicos.bin";
const char RUTA_REPUESTOS[]  = "datos/repuestos.bin";
const char RUTA_ORDENES[]    = "datos/ordenes.bin";

// ===================== HEADER (sirve para cualquier archivo) =====================
bool inicializarArchivo(const char* ruta);                    // lo crea si no existe; true = listo para usarse
bool leerHeader(const char* ruta, ArchivoHeader* h);          // lee solo el header
bool actualizarHeader(const char* ruta, ArchivoHeader h);     // sobrescribe solo el header

// ===================== VEHICULOS =====================
long posicionVehiculo(int id);                                // en que byte empieza ese vehiculo
Resultado guardarVehiculoNuevo(Vehiculo v, int* idAsignado);  // escribe al final y dice el ID asignado
Resultado leerVehiculoPorID(int id, Vehiculo* v);             // lo entrega activo o eliminado
Resultado sobrescribirVehiculo(Vehiculo v);                   // lo reescribe en su misma posicion
Resultado marcarVehiculoEliminado(int id);                    // borrado logico
Resultado contarVehiculosActivos(int* cantidad);              // desde el header, sin recorrer

// ===================== MECANICOS =====================
long posicionMecanico(int id);
Resultado guardarMecanicoNuevo(Mecanico m, int* idAsignado);
Resultado leerMecanicoPorID(int id, Mecanico* m);
Resultado sobrescribirMecanico(Mecanico m);
Resultado marcarMecanicoEliminado(int id);
Resultado contarMecanicosActivos(int* cantidad);

// ===================== REPUESTOS =====================
long posicionRepuesto(int id);
Resultado guardarRepuestoNuevo(Repuesto r, int* idAsignado);
Resultado leerRepuestoPorID(int id, Repuesto* r);
Resultado sobrescribirRepuesto(Repuesto r);
Resultado marcarRepuestoEliminado(int id);
Resultado contarRepuestosActivos(int* cantidad);

// ===================== ORDENES =====================
long posicionOrden(int id);
Resultado guardarOrdenNueva(OrdenServicio o, int* idAsignado);
Resultado leerOrdenPorID(int id, OrdenServicio* o);
Resultado sobrescribirOrden(OrdenServicio o);
Resultado marcarOrdenEliminada(int id);
Resultado contarOrdenesActivas(int* cantidad);

// ===================== TALLER (sin header, un solo registro) =====================
bool existeTaller();                 // true si taller.bin ya existe
bool guardarTaller(Taller t);        // escribe (o reescribe) el unico registro
bool leerTaller(Taller* t);          // lee el unico registro

// ===================== ARRANQUE =====================
bool inicializarArchivos();          // crea los 4 archivos de entidades si faltan