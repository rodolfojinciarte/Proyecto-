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