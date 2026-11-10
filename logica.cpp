#include "logica.h"
#include "persistencia.h"
#include "utilidades.h"
#include <cstring>
#include <cstdio>     // snprintf: arma un texto con numeros adentro
#include <ctime>

// =====================================================================
// FUNCIONES DE AYUDA DE VEHICULOS
// (static = solo se pueden usar dentro de este archivo)
// =====================================================================

// QUE HACE: recorre vehiculos.bin UN REGISTRO A LA VEZ buscando la placa entre los activos.
// DEVUELVE: el ID del vehiculo activo que la tiene, o 0 si ninguno la tiene.
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

// QUE HACE: revisa si algun registro de ordenes.bin pertenece a ese vehiculo (en cualquier estado).
// Deja la respuesta en *tiene.
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

// QUE HACE: revisa los datos que se pueden registrar o actualizar (todo menos placa y km).
// DEVUELVE: true si todos son validos.
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

// QUE HACE: valida todos los datos y, si estan bien, guarda el vehiculo nuevo.
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

// QUE HACE: lee el vehiculo por ID (acceso directo) y dice si esta activo o eliminado.
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

// QUE HACE: busca el vehiculo activo con esa placa exacta (recorre el archivo).
Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo* v) {
    int id = idConPlaca(placa);
    if (id == 0) {
        return NO_EXISTE;
    }
    return leerVehiculoPorID(id, v);
}

// QUE HACE: recorre el archivo y copia en resultados[] los vehiculos activos
// cuyo propietario contiene el texto buscado (maximo MAX_RESULTADOS).
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

// QUE HACE: copia en resultados[] todos los vehiculos activos (maximo MAX_RESULTADOS).
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

// QUE HACE: cambia marca, modelo, ano y datos del propietario de un vehiculo activo.
Resultado actualizarVehiculo(int id, Vehiculo datos) {
    // 1. El vehiculo debe existir y estar activo
    Vehiculo actual;
    Resultado r = obtenerVehiculoPorID(id, &actual);
    if (r != OK) {
        return r;                     // NO_EXISTE, ELIMINADO o ERROR_ARCHIVO
    }
    // 2. Los datos nuevos deben ser validos
    if (!datosVehiculoValidos(datos)) {
        return DATO_INVALIDO;
    }
    // 3. Copio solo lo que se puede cambiar. Placa, km, ID y fechaCreacion quedan igual.
    strcpy(actual.marca, datos.marca);
    strcpy(actual.modelo, datos.modelo);
    actual.anio = datos.anio;
    strcpy(actual.nombrePropietario, datos.nombrePropietario);
    strcpy(actual.cedulaPropietario, datos.cedulaPropietario);
    strcpy(actual.telefonoPropietario, datos.telefonoPropietario);
    // 4. Lo escribo en su misma posicion
    return sobrescribirVehiculo(actual);
}

// QUE HACE: borrado logico del vehiculo, solo si no tiene ordenes.
Resultado eliminarVehiculo(int id) {
    // 1. Debe existir y estar activo
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r != OK) {
        return r;
    }
    // 2. No puede tener ordenes en ningun estado
    bool tiene = false;
    r = vehiculoTieneOrdenes(id, &tiene);
    if (r != OK) {
        return r;
    }
    if (tiene) {
        return TIENE_ORDENES;
    }
    // 3. Borrado logico
    return marcarVehiculoEliminado(id);
}

// QUE HACE: dice en que byte de vehiculos.bin esta ese vehiculo (para depuracion).
long posicionEnDiscoVehiculo(int id) {
    return posicionVehiculo(id);
}

// =====================================================================
// MECANICOS
// =====================================================================

// QUE HACE: revisa que la especialidad sea una de las 5 permitidas.
bool esEspecialidadValida(const char* especialidad) {
    const char* validas[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(especialidad, validas[i]) == 0) {
            return true;
        }
    }
    return false;
}

// QUE HACE: recorre mecanicos.bin buscando la cedula entre TODOS (activos y de baja).
// DEVUELVE: el ID del que la tiene, o 0 si nadie.
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

// QUE HACE: valida los datos del mecanico y, si estan bien, lo guarda con la fecha de hoy.
Resultado registrarMecanico(Mecanico datos, int* idAsignado) {
    // 1. Validaciones
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
    // 2. Fecha de ingreso = hoy, y se guarda
    fechaActual(datos.fechaIngreso);
    return guardarMecanicoNuevo(datos, idAsignado);
}

// QUE HACE: lee el mecanico por ID y dice si esta activo o dado de baja.
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

// QUE HACE: recorre ordenes.bin y cuenta las ABIERTAS de ese mecanico.
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

// QUE HACE: copia todos los mecanicos (activos y de baja) con su numero de ordenes abiertas.
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

// QUE HACE: da de baja (borrado logico) al mecanico, solo si no tiene ordenes abiertas.
Resultado darDeBajaMecanico(int id) {
    // 1. Debe existir y estar activo
    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, &m);
    if (r != OK) {
        return r;                 // NO_EXISTE, ERROR_ARCHIVO o ELIMINADO (ya de baja)
    }
    // 2. No puede tener ordenes abiertas
    int abiertas = 0;
    r = contarOrdenesAbiertas(id, &abiertas);
    if (r != OK) {
        return r;
    }
    if (abiertas > 0) {
        return TIENE_ORDENES_ABIERTAS;
    }
    // 3. Baja logica: el registro sigue en el archivo para el historial
    return marcarMecanicoEliminado(id);
}

// =====================================================================
// REPUESTOS
// =====================================================================

// QUE HACE: recorre repuestos.bin buscando el codigo entre los ACTIVOS.
// DEVUELVE: el ID del que lo tiene, o 0.
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

// QUE HACE: revisa si el repuesto aparece en alguna linea de alguna orden (cualquier estado).
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

// QUE HACE: valida los datos del repuesto y, si estan bien, lo guarda.
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

// QUE HACE: lee el repuesto por ID y dice si esta activo o eliminado.
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

// QUE HACE: busca el repuesto activo con ese codigo exacto.
Resultado obtenerRepuestoPorCodigo(const char* codigo, Repuesto* r) {
    int id = idConCodigo(codigo);
    if (id == 0) {
        return NO_EXISTE;
    }
    return leerRepuestoPorID(id, r);
}

// QUE HACE: copia en resultados[] los repuestos activos cuyo nombre contiene el texto.
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

// QUE HACE: suma unidades al stock (lee la copia, la cambia y la escribe de vuelta).
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

// QUE HACE: copia en resultados[] todos los repuestos activos.
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

// QUE HACE: dice si el repuesto esta en bajo stock (stock <= stock minimo).
bool esBajoStock(Repuesto r) {
    return r.stock <= r.stockMinimo;
}

// QUE HACE: cambia el precio vigente. Las ordenes no se tocan: tienen su precio historico.
Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio) {
    if (nuevoPrecio <= 0) {
        return DATO_INVALIDO;
    }
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        return res;
    }
    r.precio = nuevoPrecio;
    return sobrescribirRepuesto(r);
}

// QUE HACE: borrado logico del repuesto, solo si no aparece en ninguna orden.
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
// ORDENES
// Reglas de las operaciones que escriben en varios archivos:
// 1. Todas las validaciones se hacen ANTES de la primera escritura.
// 2. Siempre se escribe primero la orden y despues el otro archivo.
// =====================================================================

// QUE HACE: busca si el vehiculo ya tiene una orden ABIERTA.
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

// QUE HACE: busca la linea de ese repuesto dentro de la orden.
// DEVUELVE: su posicion (0 a 14), o -1 si no esta.
static int posicionDeLinea(OrdenServicio o, int idRepuesto) {
    for (int i = 0; i < o.numLineas; i++) {
        if (o.lineas[i].idRepuesto == idRepuesto) {
            return i;
        }
    }
    return -1;
}

// QUE HACE: abre una orden nueva. Escribe la orden y despues el km del vehiculo.
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

// QUE HACE: lee una orden por ID (con sus lineas).
Resultado obtenerOrdenPorID(int id, OrdenServicio* o) {
    return leerOrdenPorID(id, o);
}

// QUE HACE: agrega un repuesto a una orden ABIERTA. Escribe la orden y despues el stock.
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

// QUE HACE: quita la linea completa de un repuesto y devuelve sus unidades al stock.
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

// QUE HACE: cierra una orden ABIERTA con su mano de obra y la fecha de hoy.
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

// QUE HACE: anula una orden ABIERTA o CERRADA y devuelve al stock todas sus unidades.
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

// QUE HACE: calcula el total: mano de obra + suma de (cantidad x precio historico).
float calcularTotalOrden(OrdenServicio o) {
    float total = o.manoObra;
    for (int i = 0; i < o.numLineas; i++) {
        total += o.lineas[i].cantidad * o.lineas[i].precioUnitario;
    }
    return total;
}

// QUE HACE: copia en resultados[] las ordenes que estan en ese estado.
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

// QUE HACE: copia en resultados[] todas las ordenes de ese vehiculo (su historial).
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

// QUE HACE: crea los archivos .bin que falten (la presentacion no puede abrir archivos).
bool prepararArchivos() {
    return inicializarArchivos();
}

// QUE HACE: dice si el taller ya fue configurado (si existe taller.bin).
bool tallerConfigurado() {
    return existeTaller();
}

// QUE HACE: valida y guarda los datos del taller en el primer arranque.
Resultado configurarTaller(Taller t) {
    if (strlen(t.nombre) == 0 || strlen(t.rif) == 0) {
        return DATO_INVALIDO;
    }
    t.fechaCreacion = time(0);
    return guardarTaller(t) ? OK : ERROR_ARCHIVO;
}

// QUE HACE: lee los datos del taller.
Resultado obtenerTaller(Taller* t) {
    return leerTaller(t) ? OK : ERROR_ARCHIVO;
}

// =====================================================================
// MANTENIMIENTO
// =====================================================================

// QUE HACE: agrega un problema a la lista del reporte (si ya no cabe, solo lo cuenta).
static void agregarProblema(ReporteIntegridad* rep, const char* texto) {
    if (rep->numProblemas < MAX_PROBLEMAS) {
        strncpy(rep->problemas[rep->numProblemas], texto, 149);
        rep->problemas[rep->numProblemas][149] = '\0';
        rep->numProblemas++;
    } else {
        rep->problemasNoMostrados++;
    }
}

// QUE HACE: compara lo que dice el header con lo que realmente se conto.
static void compararHeader(const char* nombre, ArchivoHeader h, int fisicos, int activos, ReporteIntegridad* rep) {
    char msg[150];
    if (h.cantidadRegistros != fisicos) {
        snprintf(msg, 150, "ROTO: Header de %s indica %d registros; el archivo contiene %d",
                 nombre, h.cantidadRegistros, fisicos);
        agregarProblema(rep, msg);
    }
    if (h.registrosActivos != activos) {
        snprintf(msg, 150, "ROTO: Header de %s indica %d activos; se contaron %d",
                 nombre, h.registrosActivos, activos);
        agregarProblema(rep, msg);
    }
}

// QUE HACE: recorre los 4 archivos, un registro a la vez, y anota en el reporte:
// a) ordenes con vehiculo o mecanico que no existe
// b) ordenes ABIERTAS con vehiculo o mecanico inactivo
// c) lineas con un repuesto que no existe
// d) repuestos con stock negativo
// e) headers que no coinciden con el contenido real
Resultado verificarIntegridad(ReporteIntegridad* rep) {
    rep->vehiculos = 0;
    rep->mecanicos = 0;
    rep->repuestos = 0;
    rep->ordenes = 0;
    rep->numProblemas = 0;
    rep->problemasNoMostrados = 0;
    char msg[150];
    ArchivoHeader h;
    int fisicos = 0;

    // ---------- VEHICULOS: header (e) ----------
    if (!leerHeader(RUTA_VEHICULOS, &h) ||
        contarRegistrosEnArchivo(RUTA_VEHICULOS, sizeof(Vehiculo), &fisicos) != OK) {
        agregarProblema(rep, "ROTO: No se pudo leer datos/vehiculos.bin");
    } else {
        int hasta = (h.cantidadRegistros < fisicos) ? h.cantidadRegistros : fisicos;
        int activos = 0;
        Vehiculo v;
        for (int id = 1; id <= hasta; id++) {
            if (leerVehiculoPorID(id, &v) == OK) {
                rep->vehiculos++;
                if (!v.eliminado) activos++;
            }
        }
        compararHeader("vehiculos.bin", h, fisicos, activos, rep);
    }

    // ---------- MECANICOS: header (e) ----------
    if (!leerHeader(RUTA_MECANICOS, &h) ||
        contarRegistrosEnArchivo(RUTA_MECANICOS, sizeof(Mecanico), &fisicos) != OK) {
        agregarProblema(rep, "ROTO: No se pudo leer datos/mecanicos.bin");
    } else {
        int hasta = (h.cantidadRegistros < fisicos) ? h.cantidadRegistros : fisicos;
        int activos = 0;
        Mecanico m;
        for (int id = 1; id <= hasta; id++) {
            if (leerMecanicoPorID(id, &m) == OK) {
                rep->mecanicos++;
                if (!m.eliminado) activos++;
            }
        }
        compararHeader("mecanicos.bin", h, fisicos, activos, rep);
    }

    // ---------- REPUESTOS: header (e) y stock negativo (d) ----------
    if (!leerHeader(RUTA_REPUESTOS, &h) ||
        contarRegistrosEnArchivo(RUTA_REPUESTOS, sizeof(Repuesto), &fisicos) != OK) {
        agregarProblema(rep, "ROTO: No se pudo leer datos/repuestos.bin");
    } else {
        int hasta = (h.cantidadRegistros < fisicos) ? h.cantidadRegistros : fisicos;
        int activos = 0;
        Repuesto r;
        for (int id = 1; id <= hasta; id++) {
            if (leerRepuestoPorID(id, &r) == OK) {
                rep->repuestos++;
                if (!r.eliminado) activos++;
                if (r.stock < 0) {
                    snprintf(msg, 150, "ROTO: Repuesto ID %d (%s) tiene stock negativo: %d", r.id, r.codigo, r.stock);
                    agregarProblema(rep, msg);
                }
            }
        }
        compararHeader("repuestos.bin", h, fisicos, activos, rep);
    }

    // ---------- ORDENES: header (e), vehiculo y mecanico (a, b) y lineas (c) ----------
    if (!leerHeader(RUTA_ORDENES, &h) ||
        contarRegistrosEnArchivo(RUTA_ORDENES, sizeof(OrdenServicio), &fisicos) != OK) {
        agregarProblema(rep, "ROTO: No se pudo leer datos/ordenes.bin");
        return OK;
    }
    int hasta = (h.cantidadRegistros < fisicos) ? h.cantidadRegistros : fisicos;
    int activas = 0;
    OrdenServicio o;
    for (int id = 1; id <= hasta; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            continue;
        }
        rep->ordenes++;
        if (!o.eliminado) activas++;

        // a) el vehiculo y el mecanico de la orden existen?
        Vehiculo v;
        Resultado rv = leerVehiculoPorID(o.idVehiculo, &v);
        if (rv != OK) {
            snprintf(msg, 150, "ROTO: Orden #%d apunta al vehiculo ID %d (no existe en vehiculos.bin)", o.id, o.idVehiculo);
            agregarProblema(rep, msg);
        }
        Mecanico m;
        Resultado rm = leerMecanicoPorID(o.idMecanico, &m);
        if (rm != OK) {
            snprintf(msg, 150, "ROTO: Orden #%d apunta al mecanico ID %d (no existe en mecanicos.bin)", o.id, o.idMecanico);
            agregarProblema(rep, msg);
        }

        // b) si esta ABIERTA, su vehiculo y su mecanico deben estar activos
        if (strcmp(o.estado, "ABIERTA") == 0) {
            if (rv == OK && v.eliminado) {
                snprintf(msg, 150, "ROTO: Orden #%d esta ABIERTA y su vehiculo ID %d esta eliminado", o.id, o.idVehiculo);
                agregarProblema(rep, msg);
            }
            if (rm == OK && m.eliminado) {
                snprintf(msg, 150, "ROTO: Orden #%d esta ABIERTA y su mecanico ID %d esta dado de baja", o.id, o.idMecanico);
                agregarProblema(rep, msg);
            }
        }

        // c) las lineas apuntan a repuestos que existen?
        if (o.numLineas < 0 || o.numLineas > MAX_LINEAS) {
            snprintf(msg, 150, "ROTO: Orden #%d tiene un numero de lineas imposible: %d", o.id, o.numLineas);
            agregarProblema(rep, msg);
            continue;
        }
        for (int j = 0; j < o.numLineas; j++) {
            Repuesto r;
            if (leerRepuestoPorID(o.lineas[j].idRepuesto, &r) != OK) {
                snprintf(msg, 150, "ROTO: Orden #%d, linea %d: el repuesto ID %d no existe en repuestos.bin",
                         o.id, j + 1, o.lineas[j].idRepuesto);
                agregarProblema(rep, msg);
            }
        }
    }
    compararHeader("ordenes.bin", h, fisicos, activas, rep);
    return OK;
}

// QUE HACE: crea la carpeta backup_AAAA-MM-DD_HH-MM-SS y copia ahi los 5 archivos .bin.
Resultado crearBackup(char* rutaBackup) {
    // 1. Nombre de la carpeta con la fecha y hora de ahora
    time_t ahora = time(0);
    tm* t = localtime(&ahora);
    strftime(rutaBackup, 100, "backup_%Y-%m-%d_%H-%M-%S", t);

    // 2. Creo la carpeta
    if (crearCarpeta(rutaBackup) != OK) {
        return ERROR_ARCHIVO;
    }
    // 3. Copio cada archivo byte a byte
    const char* origenes[5] = {RUTA_TALLER, RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_REPUESTOS, RUTA_ORDENES};
    const char* nombres[5]  = {"taller.bin", "vehiculos.bin", "mecanicos.bin", "repuestos.bin", "ordenes.bin"};
    for (int i = 0; i < 5; i++) {
        char destino[200];
        strcpy(destino, rutaBackup);
        strcat(destino, "/");
        strcat(destino, nombres[i]);          // ej: backup_2026-11-08_16-20-05/vehiculos.bin
        if (copiarArchivo(origenes[i], destino) != OK) {
            return ERROR_ARCHIVO;
        }
    }
    return OK;

}

// =====================================================================
// REPORTES
// Leen los archivos UN REGISTRO A LA VEZ (nunca el archivo completo en memoria).
// Solo las ordenes CERRADAS suman: las ABIERTAS y ANULADAS no.
// =====================================================================

// QUE HACE: Reporte 1. Busca el vehiculo por placa, copia sus ordenes en ordenes[]
// y suma en *totalCerradas el total de sus ordenes CERRADAS.
Resultado reporteHistorialVehiculo(const char* placa, Vehiculo* v, OrdenServicio ordenes[],
                                   int* cantidad, bool* hayMas, float* totalCerradas) {
    *cantidad = 0;
    *hayMas = false;
    *totalCerradas = 0;

    // 1. El vehiculo (activo) con esa placa
    Resultado r = obtenerVehiculoPorPlaca(placa, v);
    if (r != OK) {
        return r;
    }
    // 2. Recorro ordenes.bin una orden a la vez
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        if (o.idVehiculo != v->id) {
            continue;                       // es de otro vehiculo
        }
        // 3. Solo las CERRADAS suman al total (aunque ya no quepan en la lista)
        if (strcmp(o.estado, "CERRADA") == 0) {
            *totalCerradas += calcularTotalOrden(o);
        }
        // 4. La guardo en la lista si aun cabe
        if (*cantidad < MAX_RESULTADOS) {
            ordenes[*cantidad] = o;
            (*cantidad)++;
        } else {
            *hayMas = true;
        }
    }
    return OK;
}

// QUE HACE: cuenta cuantas unidades de ese repuesto se usaron en ordenes CERRADAS
// (recorre ordenes.bin una orden a la vez y revisa sus lineas).
static Resultado unidadesUsadasEnCerradas(int idRepuesto, int* unidades) {
    *unidades = 0;
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, &h)) {
        return ERROR_ARCHIVO;
    }
    OrdenServicio o;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        if (leerOrdenPorID(id, &o) != OK) {
            return ERROR_ARCHIVO;
        }
        if (strcmp(o.estado, "CERRADA") != 0) {
            continue;                       // ABIERTA o ANULADA: no cuenta
        }
        for (int i = 0; i < o.numLineas; i++) {
            if (o.lineas[i].idRepuesto == idRepuesto) {
                *unidades += o.lineas[i].cantidad;
            }
        }
    }
    return OK;
}

// QUE HACE: Reporte 2. Para cada repuesto cuenta sus unidades usadas y lo mete en el top 10,
// que se mantiene ordenado de mayor a menor (como ordenar cartas en la mano).
Resultado reporteRepuestosMasUsados(FilaTopRepuesto top[], int* cantidad) {
    *cantidad = 0;
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, &h)) {
        return ERROR_ARCHIVO;
    }
    Repuesto rep;
    for (int id = 1; id <= h.cantidadRegistros; id++) {
        // 1. Leo un repuesto y cuento sus unidades
        if (leerRepuestoPorID(id, &rep) != OK) {
            return ERROR_ARCHIVO;
        }
        int unidades = 0;
        if (unidadesUsadasEnCerradas(id, &unidades) != OK) {
            return ERROR_ARCHIVO;
        }
        if (unidades == 0) {
            continue;                       // no se uso: no entra al top
        }
        // 2. Busco en que posicion le toca (despues de los que tienen mas o igual)
        int pos = *cantidad;
        while (pos > 0 && top[pos - 1].unidades < unidades) {
            pos--;
        }
        if (pos >= TOP_REPUESTOS) {
            continue;                       // no le alcanza para estar en el top 10
        }
        // 3. Corro una casilla a la derecha a los que quedan debajo (el ultimo se pierde si ya habia 10)
        int ultimo = (*cantidad < TOP_REPUESTOS) ? *cantidad : TOP_REPUESTOS - 1;
        for (int i = ultimo; i > pos; i--) {
            top[i] = top[i - 1];
        }
        // 4. Lo coloco en su posicion
        top[pos].idRepuesto = id;
        strcpy(top[pos].codigo, rep.codigo);
        strcpy(top[pos].nombre, rep.nombre);
        top[pos].unidades = unidades;
        if (*cantidad < TOP_REPUESTOS) {
            (*cantidad)++;
        }
    }
    return OK;
}

// QUE HACE: Reporte 3. Para cada mecanico (activo o de baja) suma sus ordenes CERRADAS:
// cuantas son, su mano de obra y sus repuestos.
Resultado reporteFacturacionMecanicos(FilaFacturacion filas[], int* cantidad, bool* hayMas) {
    *cantidad = 0;
    *hayMas = false;
    ArchivoHeader hm;
    ArchivoHeader ho;
    if (!leerHeader(RUTA_MECANICOS, &hm) || !leerHeader(RUTA_ORDENES, &ho)) {
        return ERROR_ARCHIVO;
    }
    Mecanico m;
    OrdenServicio o;
    for (int idM = 1; idM <= hm.cantidadRegistros; idM++) {
        if (*cantidad == MAX_RESULTADOS) {
            *hayMas = true;
            break;
        }
        // 1. Leo el mecanico (tambien los dados de baja)
        if (leerMecanicoPorID(idM, &m) != OK) {
            return ERROR_ARCHIVO;
        }
        FilaFacturacion f;
        f.idMecanico = idM;
        strcpy(f.nombre, m.nombre);
        f.dadoDeBaja = m.eliminado;
        f.ordenesCerradas = 0;
        f.manoObra = 0;
        f.repuestos = 0;
        // 2. Recorro las ordenes y sumo solo las CERRADAS de este mecanico
        for (int idO = 1; idO <= ho.cantidadRegistros; idO++) {
            if (leerOrdenPorID(idO, &o) != OK) {
                return ERROR_ARCHIVO;
            }
            if (o.idMecanico != idM || strcmp(o.estado, "CERRADA") != 0) {
                continue;
            }
            f.ordenesCerradas++;
            f.manoObra += o.manoObra;
            f.repuestos += calcularTotalOrden(o) - o.manoObra;   // total sin la mano de obra
        }
        // 3. Guardo la fila
        filas[*cantidad] = f;
        (*cantidad)++;
    }
    return OK;
}