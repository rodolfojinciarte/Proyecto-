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

// =====================================================================
// REPUESTOS
// =====================================================================

// Recorre repuestos.bin buscando el codigo entre los ACTIVOS. ID o 0.
static int idConCodigo(const char* codigo) {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) {
        return 0;
    }
    Repuesto r;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerRepuestoPorID(id, &r) == OK && !r.eliminado && strcmp(r.codigo, codigo) == 0) {
            return id;
        }
    }
    return 0;
}

// Revisa si el repuesto aparece en alguna linea de alguna orden (cualquier estado)
static Resultado repuestoUsadoEnOrdenes(int idRepuesto, bool* usado) {
    *usado = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        // for dentro de for: cada orden (id) y, dentro de ella, cada linea (j)
        for (int j = 0; j < o.numLineas; j++) {
            if (o.lineas[j].idRepuesto == idRepuesto) {
                *usado = true;
                return OK;
            }
        }
    }
    return OK;
}

Resultado registrarRepuesto(Repuesto datos, int* idAsignado) {
    if (strlen(datos.codigo) == 0 || strlen(datos.nombre) == 0 || strlen(datos.marca) == 0) {
        return DATO_INVALIDO;
    }
    if (datos.precio <= 0 || datos.stock < 0 || datos.stockMinimo < 0) {
        return DATO_INVALIDO;
    }
    if (idConCodigo(datos.codigo) != 0) {
        return DUPLICADO;
    }
    return guardarRepuestoNuevo(datos, idAsignado);
}

Resultado obtenerRepuestoPorID(int id, Repuesto* r) {
    Resultado res = leerRepuestoPorID(id, r);
    if (res != OK) {
        return res;
    }
    if (r->eliminado) {
        return ELIMINADO;
    }
    return OK;
}

Resultado obtenerRepuestoPorCodigo(const char* codigo, Repuesto* r) {
    int id = idConCodigo(codigo);
    if (id == 0) {
        return NO_EXISTE;
    }
    return leerRepuestoPorID(id, r);
}

Resultado buscarRepuestosPorNombre(const char* texto, Repuesto resultados[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) {
        return ERROR_ARCHIVO;
    }
    Repuesto r;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerRepuestoPorID(id, &r) != OK) {
            return ERROR_ARCHIVO;
        }
        if (r.eliminado || !contieneSinMayusculas(r.nombre, texto)) {
            continue;
        }
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        resultados[*cantidad] = r;
        (*cantidad)++;
    }
    return OK;
}

Resultado registrarEntradaRepuesto(int id, int cantidad) {
    if (cantidad <= 0) {
        return DATO_INVALIDO;
    }
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        return res;
    }
    r.stock = r.stock + cantidad;       // cambio la copia...
    return sobrescribirRepuesto(r);     // ...y la escribo en su lugar del archivo
}

Resultado listarRepuestos(Repuesto resultados[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) {
        return ERROR_ARCHIVO;
    }
    Repuesto r;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerRepuestoPorID(id, &r) != OK) {
            return ERROR_ARCHIVO;
        }
        if (r.eliminado) {
            continue;
        }
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        resultados[*cantidad] = r;
        (*cantidad)++;
    }
    return OK;
}

bool esBajoStock(Repuesto r) {
    return r.stock <= r.stockMinimo;
}

Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio) {
    if (nuevoPrecio <= 0) {
        return DATO_INVALIDO;
    }
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        return res;
    }
    r.precio = nuevoPrecio;             // las ordenes no se tocan: tienen su precioUnitario
    return sobrescribirRepuesto(r);
}

Resultado eliminarRepuesto(int id) {
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        return res;
    }
    bool usado = false;
    res = repuestoUsadoEnOrdenes(id, &usado);
    if (res != OK) {
        return res;
    }
    if (usado) {
        return TIENE_ORDENES;
    }
    return marcarRepuestoEliminado(id);
}

// =====================================================================
// ORDENES - BLOQUE A
// =====================================================================

// Busca si el vehiculo ya tiene una orden ABIERTA
static Resultado vehiculoTieneOrdenAbierta(int idVehiculo, bool* tiene) {
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
        if (o.idVehiculo == idVehiculo && strcmp(o.estado, "ABIERTA") == 0) {
            *tiene = true;
            return OK;
        }
    }
    return OK;
}

// Devuelve la posicion (0 a 14) de la linea de ese repuesto en la orden, o -1 si no esta
static int posicionDeLinea(OrdenServicio o, int idRepuesto) {
    for (int i = 0; i < o.numLineas; i++) {
        if (o.lineas[i].idRepuesto == idRepuesto) {
            return i;
        }
    }
    return -1;
}

Resultado abrirOrden(int idVehiculo, int idMecanico, int kilometraje, const char* falla, int* idAsignado) {
    // ---------- 1. TODAS las validaciones antes de escribir ----------
    if (strlen(falla) == 0 || kilometraje < 0) {
        return DATO_INVALIDO;
    }
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(idVehiculo, &v);
    if (r != OK) {
        return r;                       // NO_EXISTE, ELIMINADO o ERROR_ARCHIVO
    }
    Mecanico m;
    r = obtenerMecanicoPorID(idMecanico, &m);
    if (r != OK) {
        return r;                       // ELIMINADO = dado de baja: no recibe ordenes nuevas
    }
    bool tieneAbierta = false;
    r = vehiculoTieneOrdenAbierta(idVehiculo, &tieneAbierta);
    if (r != OK) {
        return r;
    }
    if (tieneAbierta) {
        return VEHICULO_CON_ORDEN_ABIERTA;
    }
    int abiertas = 0;
    r = contarOrdenesAbiertas(idMecanico, &abiertas);
    if (r != OK) {
        return r;
    }
    if (abiertas >= 3) {
        return MECANICO_OCUPADO;
    }
    if (kilometraje < v.kilometraje) {
        return KILOMETRAJE_MENOR;
    }

    // ---------- 2. Armo la orden nueva ----------
    OrdenServicio o = {};               // {} = todo en cero, numLineas = 0
    o.idVehiculo = idVehiculo;
    o.idMecanico = idMecanico;
    fechaActual(o.fechaApertura);
    strcpy(o.fechaCierre, "");          // vacia mientras este abierta
    strcpy(o.estado, "ABIERTA");
    strncpy(o.descripcionFalla, falla, 199);
    o.kilometrajeIngreso = kilometraje;
    o.manoObra = 0;

    // ---------- 3. Escrituras: primero la orden, despues el vehiculo ----------
    r = guardarOrdenNueva(o, idAsignado);
    if (r != OK) {
        return r;
    }
    v.kilometraje = kilometraje;        // al abrir se actualiza el km del vehiculo
    return sobrescribirVehiculo(v);
}

Resultado obtenerOrdenPorID(int id, OrdenServicio* o) {
    return leerOrdenPorID(id, o);
}

Resultado agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad) {
    // ---------- 1. Validaciones ----------
    if (cantidad <= 0) {
        return DATO_INVALIDO;
    }
    OrdenServicio o;
    Resultado r = leerOrdenPorID(idOrden, &o);
    if (r != OK) {
        return r;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        return ESTADO_INCORRECTO;
    }
    Repuesto rep;
    r = obtenerRepuestoPorID(idRepuesto, &rep);
    if (r != OK) {
        return r;
    }
    if (rep.stock < cantidad) {
        return STOCK_INSUFICIENTE;
    }
    int pos = posicionDeLinea(o, idRepuesto);
    if (pos == -1 && o.numLineas == MAX_LINEAS) {
        return LIMITE_LINEAS;           // es un repuesto nuevo y ya no hay casillas libres
    }

    // ---------- 2. Cambio la COPIA de la orden ----------
    if (pos != -1) {
        o.lineas[pos].cantidad += cantidad;      // ya estaba: acumulo en su linea
    } else {
        o.lineas[o.numLineas].idRepuesto = idRepuesto;
        o.lineas[o.numLineas].cantidad = cantidad;
        o.lineas[o.numLineas].precioUnitario = rep.precio;   // precio historico
        o.numLineas++;
    }

    // ---------- 3. Escrituras: primero la orden, despues el stock ----------
    r = sobrescribirOrden(o);
    if (r != OK) {
        return r;
    }
    rep.stock -= cantidad;
    return sobrescribirRepuesto(rep);
}

// =====================================================================
// ORDENES - BLOQUE B
// =====================================================================

Resultado quitarRepuestoDeOrden(int idOrden, int idRepuesto) {
    // ---------- 1. Validaciones ----------
    OrdenServicio o;
    Resultado r = leerOrdenPorID(idOrden, &o);
    if (r != OK) {
        return r;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        return ESTADO_INCORRECTO;
    }
    int pos = posicionDeLinea(o, idRepuesto);
    if (pos == -1) {
        return NO_ESTA_EN_ORDEN;
    }
    Repuesto rep;
    r = leerRepuestoPorID(idRepuesto, &rep);   // lectura cruda: lo necesito igual
    if (r != OK) {
        return r;
    }

    // ---------- 2. Quito la linea: corro las de la derecha una casilla ----------
    int unidades = o.lineas[pos].cantidad;
    for (int i = pos; i < o.numLineas - 1; i++) {
        o.lineas[i] = o.lineas[i + 1];
    }
    o.numLineas--;

    // ---------- 3. Escrituras: primero la orden, despues el stock ----------
    r = sobrescribirOrden(o);
    if (r != OK) {
        return r;
    }
    rep.stock += unidades;                     // devuelvo las unidades al inventario
    return sobrescribirRepuesto(rep);
}

Resultado cerrarOrden(int idOrden, float manoObra) {
    if (manoObra < 0) {
        return DATO_INVALIDO;
    }
    OrdenServicio o;
    Resultado r = leerOrdenPorID(idOrden, &o);
    if (r != OK) {
        return r;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        return ESTADO_INCORRECTO;
    }
    if (o.numLineas == 0 && manoObra <= 0) {
        return ORDEN_VACIA;
    }
    o.manoObra = manoObra;
    strcpy(o.estado, "CERRADA");
    fechaActual(o.fechaCierre);                // fecha de cierre = hoy
    return sobrescribirOrden(o);               // solo escribe un archivo
}

Resultado anularOrden(int idOrden) {
    // ---------- 1. Validaciones ----------
    OrdenServicio o;
    Resultado r = leerOrdenPorID(idOrden, &o);
    if (r != OK) {
        return r;
    }
    if (strcmp(o.estado, "ANULADA") == 0) {
        return ESTADO_INCORRECTO;              // solo ABIERTAS o CERRADAS
    }
    Repuesto rep;
    for (int i = 0; i < o.numLineas; i++) {    // reviso que todos los repuestos se puedan leer
        if (leerRepuestoPorID(o.lineas[i].idRepuesto, &rep) != OK) {
            return ERROR_ARCHIVO;
        }
    }

    // ---------- 2. Escrituras: primero la orden, despues cada stock ----------
    strcpy(o.estado, "ANULADA");
    r = sobrescribirOrden(o);
    if (r != OK) {
        return r;
    }
    for (int i = 0; i < o.numLineas; i++) {
        leerRepuestoPorID(o.lineas[i].idRepuesto, &rep);
        rep.stock += o.lineas[i].cantidad;     // devuelvo las unidades de esa linea
        r = sobrescribirRepuesto(rep);
        if (r != OK) {
            return r;
        }
    }
    return OK;
}

float calcularTotalOrden(OrdenServicio o) {
    float total = o.manoObra;
    for (int i = 0; i < o.numLineas; i++) {
        total += o.lineas[i].cantidad * o.lineas[i].precioUnitario;
    }
    return total;
}

Resultado listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        if (strcmp(o.estado, estado) != 0) {
            continue;
        }
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        resultados[*cantidad] = o;
        (*cantidad)++;
    }
    return OK;
}

Resultado listarOrdenesPorVehiculo(int idVehiculo, OrdenServicio resultados[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        if (o.idVehiculo != idVehiculo) {
            continue;
        }
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        resultados[*cantidad] = o;
        (*cantidad)++;
    }
    return OK;
}

// =====================================================================
// TALLER Y ARRANQUE
// =====================================================================

bool prepararArchivos() {
    return inicializarArchivos();
}

bool tallerConfigurado() {
    return existeTaller();
}

Resultado configurarTaller(Taller t) {
    if (strlen(t.nombre) == 0 || strlen(t.rif) == 0) {
        return DATO_INVALIDO;
    }
    t.fechaCreacion = time(0);
    return guardarTaller(t) ? OK : ERROR_ARCHIVO;
}

Resultado obtenerTaller(Taller* t) {
    return leerTaller(t) ? OK : ERROR_ARCHIVO;
}