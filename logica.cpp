#include "logica.h"
#include "persistencia.h"
#include "utilidades.h"
#include <cstring>

// =====================================================================
// FUNCIONES DE AYUDA (static = solo se pueden usar dentro de este archivo)
// =====================================================================

// Recorre vehiculos.bin UN REGISTRO A LA VEZ buscando la placa entre los activos.
// Devuelve el ID del vehiculo activo que la tiene, o 0 si ninguno la tiene.
static int idConPlaca(const char* placa) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return 0;
    }
    Vehiculo v;   // un solo vehiculo en memoria a la vez
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerVehiculoPorID(id, &v) == OK && !v.eliminado && strcmp(v.placa, placa) == 0) {
            return id;
        }
    }
    return 0;
}

// Revisa si algun registro de ordenes.bin pertenece a ese vehiculo (en cualquier estado)
static Resultado vehiculoTieneOrdenes(int idVehiculo, bool* tiene) {
    *tiene = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        if (o.idVehiculo == idVehiculo) {
            *tiene = true;
            return OK;
        }
    }
    return OK;
}

// Revisa los datos que se pueden registrar o actualizar (todo menos placa y km)
static bool datosVehiculoValidos(Vehiculo d) {
    if (strlen(d.marca) == 0 || strlen(d.modelo) == 0 || strlen(d.nombrePropietario) == 0) {
        return false;   // nombres, marca y modelo no pueden estar vacios
    }
    if (d.anio < 1950 || d.anio > 2027) {
        return false;
    }
    if (!esCedulaValida(d.cedulaPropietario)) {
        return false;
    }
    return true;
}

// =====================================================================
// VEHICULOS
// =====================================================================

Resultado registrarVehiculo(Vehiculo datos, int* idAsignado) {
    // 1. Todas las validaciones ANTES de escribir
    if (!datosVehiculoValidos(datos)) {
        return DATO_INVALIDO;
    }
    if (!esPlacaValida(datos.placa)) {
        return DATO_INVALIDO;
    }
    if (datos.kilometraje < 0) {
        return DATO_INVALIDO;
    }
    if (idConPlaca(datos.placa) != 0) {
        return DUPLICADO;   // la placa ya la tiene un vehiculo ACTIVO
    }
    // 2. La persistencia pone el ID, las fechas y lo escribe al final
    return guardarVehiculoNuevo(datos, idAsignado);
}

Resultado obtenerVehiculoPorID(int id, Vehiculo* v) {
    Resultado r = leerVehiculoPorID(id, v);   // acceso directo: no recorre el archivo
    if (r != OK) {
        return r;                             // NO_EXISTE o ERROR_ARCHIVO
    }
    if (v->eliminado) {
        return ELIMINADO;                     // existe, pero fue eliminado
    }
    return OK;
}

Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo* v) {
    int id = idConPlaca(placa);
    if (id == 0) {
        return NO_EXISTE;
    }
    return leerVehiculoPorID(id, v);
}

Resultado buscarVehiculosPorPropietario(const char* texto, Vehiculo resultados[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    Vehiculo v;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerVehiculoPorID(id, &v) != OK) {
            return ERROR_ARCHIVO;
        }
        if (v.eliminado) {
            continue;                 // continue = salta al siguiente vehiculo
        }
        if (!contieneSinMayusculas(v.nombrePropietario, texto)) {
            continue;
        }
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;           // ya no caben: aviso que hay mas
            break;                    // break = salgo del for
        }
        resultados[*cantidad] = v;    // guardo una COPIA en el arreglo del que llamo
        (*cantidad)++;
    }
    return OK;
}

Resultado listarVehiculos(Vehiculo resultados[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, &h)) {
        return ERROR_ARCHIVO;
    }
    Vehiculo v;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerVehiculoPorID(id, &v) != OK) {
            return ERROR_ARCHIVO;
        }
        if (v.eliminado) {
            continue;
        }
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        resultados[*cantidad] = v;
        (*cantidad)++;
    }
    return OK;
}

Resultado actualizarVehiculo(int id, Vehiculo datos) {
    Vehiculo actual;
    Resultado r = obtenerVehiculoPorID(id, &actual);
    if (r != OK) {
        return r;                     // NO_EXISTE, ELIMINADO o ERROR_ARCHIVO
    }
    if (!datosVehiculoValidos(datos)) {
        return DATO_INVALIDO;
    }
    // Copio solo lo que se puede cambiar. Placa, km, ID y fechaCreacion quedan igual.
    strcpy(actual.marca, datos.marca);
    strcpy(actual.modelo, datos.modelo);
    actual.anio = datos.anio;
    strcpy(actual.nombrePropietario, datos.nombrePropietario);
    strcpy(actual.cedulaPropietario, datos.cedulaPropietario);
    strcpy(actual.telefonoPropietario, datos.telefonoPropietario);
    return sobrescribirVehiculo(actual);   // lo escribe en su misma posicion
}

Resultado eliminarVehiculo(int id) {
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r != OK) {
        return r;
    }
    bool tiene = false;
    r = vehiculoTieneOrdenes(id, &tiene);
    if (r != OK) {
        return r;
    }
    if (tiene) {
        return TIENE_ORDENES;
    }
    return marcarVehiculoEliminado(id);    // borrado logico
}

long posicionEnDiscoVehiculo(int id) {
    return posicionVehiculo(id);
}

// =====================================================================
// MECANICOS
// =====================================================================

bool esEspecialidadValida(const char* especialidad) {
    const char* validas[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(especialidad, validas[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Recorre mecanicos.bin buscando la cedula entre TODOS (activos y de baja).
// Devuelve el ID del que la tiene, o 0 si nadie.
static int idConCedula(const char* cedula) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) {
        return 0;
    }
    Mecanico m;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerMecanicoPorID(id, &m) == OK && strcmp(m.cedula, cedula) == 0) {
            return id;
        }
    }
    return 0;
}

Resultado registrarMecanico(Mecanico datos, int* idAsignado) {
    if (strlen(datos.nombre) == 0) {
        return DATO_INVALIDO;
    }
    if (!esCedulaValida(datos.cedula)) {
        return DATO_INVALIDO;
    }
    if (!esEspecialidadValida(datos.especialidad)) {
        return DATO_INVALIDO;
    }
    if (idConCedula(datos.cedula) != 0) {
        return DUPLICADO;
    }
    fechaActual(datos.fechaIngreso);          // fecha de ingreso = hoy
    return guardarMecanicoNuevo(datos, idAsignado);
}

Resultado obtenerMecanicoPorID(int id, Mecanico* m) {
    Resultado r = leerMecanicoPorID(id, m);
    if (r != OK) {
        return r;
    }
    if (m->eliminado) {
        return ELIMINADO;                     // dado de baja
    }
    return OK;
}

Resultado contarOrdenesAbiertas(int idMecanico, int* cantidad) {
    *cantidad = 0;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        // dos condiciones a la vez: es de este mecanico Y esta abierta
        if (o.idMecanico == idMecanico && strcmp(o.estado, "ABIERTA") == 0) {
            (*cantidad)++;
        }
    }
    return OK;
}

Resultado listarMecanicos(Mecanico resultados[], int ordenesAbiertas[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, &h)) {
        return ERROR_ARCHIVO;
    }
    Mecanico m;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerMecanicoPorID(id, &m) != OK) {
            return ERROR_ARCHIVO;
        }
        // aqui NO se saltan los dados de baja: el PDF pide listarlos marcados
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        int abiertas = 0;
        if (contarOrdenesAbiertas(m.id, &abiertas) != OK) {
            return ERROR_ARCHIVO;
        }
        resultados[*cantidad] = m;
        ordenesAbiertas[*cantidad] = abiertas;
        (*cantidad)++;
    }
    return OK;
}

Resultado darDeBajaMecanico(int id) {
    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, &m);
    if (r != OK) {
        return r;                 // NO_EXISTE, ERROR_ARCHIVO o ELIMINADO (ya de baja)
    }
    int abiertas = 0;
    r = contarOrdenesAbiertas(id, &abiertas);
    if (r != OK) {
        return r;
    }
    if (abiertas > 0) {
        return TIENE_ORDENES_ABIERTAS;
    }
    return marcarMecanicoEliminado(id);   // baja logica: el registro sigue en el archivo
}