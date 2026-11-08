#include "presentacion.h"
#include "logica.h"
#include "utilidades.h"
#include <iostream>
#include <iomanip>    // setw (ancho de columnas) y setprecision (decimales)
#include <cstring>
#include <ctime>
using namespace std;

// =====================================================================
// AYUDAS
// =====================================================================

// switch = un if con muchos casos: segun el valor de r, devuelve un texto
const char* textoResultado(Resultado r) {
    switch (r) {
        case OK:                         return "Operacion exitosa.";
        case ERROR_ARCHIVO:              return "No se pudo abrir, leer o escribir un archivo de la carpeta datos/.";
        case NO_EXISTE:                  return "No existe un registro con ese ID.";
        case ELIMINADO:                  return "El registro fue eliminado (o dado de baja).";
        case DATO_INVALIDO:              return "Uno de los datos no es valido.";
        case DUPLICADO:                  return "Ya existe un registro activo con ese dato.";
        case TIENE_ORDENES:              return "Tiene ordenes registradas y no puede eliminarse.";
        case TIENE_ORDENES_ABIERTAS:     return "Tiene ordenes ABIERTAS.";
        case ESTADO_INCORRECTO:          return "La orden no esta en el estado necesario.";
        case STOCK_INSUFICIENTE:         return "Stock insuficiente.";
        case LIMITE_LINEAS:              return "La orden ya tiene el maximo de 15 lineas.";
        case MECANICO_OCUPADO:           return "El mecanico ya tiene 3 ordenes abiertas.";
        case VEHICULO_CON_ORDEN_ABIERTA: return "El vehiculo ya tiene una orden ABIERTA.";
        case KILOMETRAJE_MENOR:          return "El kilometraje es menor que el registrado.";
        case ORDEN_VACIA:                return "La orden no tiene lineas ni mano de obra.";
        case NO_ESTA_EN_ORDEN:           return "Ese repuesto no esta en la orden.";
    }
    return "Error desconocido.";
}

// Convierte el numero time_t en texto "AAAA-MM-DD"
void textoFecha(time_t t, char* destino) {
    tm* f = localtime(&t);
    strftime(destino, 11, "%Y-%m-%d", f);
}

// Bordes de los menus: 44 caracteres de ancho
void encabezado(const char* titulo) {
    cout << endl;
    cout << "+==========================================+" << endl;
    cout << "|  " << left << setw(40) << titulo << "|" << endl;
    cout << "+==========================================+" << endl;
}

void opcion(const char* texto) {
    cout << "|  " << left << setw(40) << texto << "|" << endl;
}

void pie() {
    cout << "+==========================================+" << endl;
}

// Primer arranque: taller.bin no existe, se piden los datos del taller
bool menuPrimerArranque() {
    encabezado("CONFIGURACION INICIAL DEL TALLER");
    cout << "Es la primera vez que se ejecuta el sistema." << endl;
    cout << "(Escriba CANCELAR para salir)" << endl;
    while (true) {
        Taller t = {};
        if (!leerTexto("Nombre del taller: ", t.nombre, 100)) return false;
        if (!leerTexto("RIF: ", t.rif, 20)) return false;
        if (!leerTexto("Direccion: ", t.direccion, 200)) return false;
        if (!leerTexto("Telefono: ", t.telefono, 20)) return false;
        Resultado r = configurarTaller(t);
        if (r == OK) {
            cout << "Taller guardado en datos/taller.bin" << endl;
            return true;
        }
        if (r == ERROR_ARCHIVO) {
            cout << "ERROR: No se pudo crear datos/taller.bin" << endl;
            return false;
        }
        cout << "ERROR: El nombre y el RIF no pueden estar vacios. Intente de nuevo." << endl;
    }
}

// =====================================================================
// VEHICULOS
// =====================================================================

// Mensaje cuando no se pudo obtener un vehiculo por su ID
static void avisoVehiculo(Resultado r, int id, Vehiculo v) {
    if (r == NO_EXISTE) {
        cout << "No existe un vehiculo con ID " << id << "." << endl;
    } else if (r == ELIMINADO) {
        cout << "El vehiculo ID " << id << " (" << v.placa << ") fue ELIMINADO." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void mostrarVehiculo(Vehiculo v) {
    char fecha[11];
    textoFecha(v.fechaCreacion, fecha);
    cout << "  ID " << v.id << " - " << v.placa << " - " << v.marca << " " << v.modelo
         << " " << v.anio << " (" << v.kilometraje << " km)";
    if (v.eliminado) {
        cout << "  [ELIMINADO]";
    }
    cout << endl;
    cout << "  Propietario: " << v.nombrePropietario << " (" << v.cedulaPropietario
         << ")  Tel: " << v.telefonoPropietario << endl;
    cout << "  Registrado: " << fecha << endl;
}

void mostrarListaVehiculos(Vehiculo lista[], int cantidad, bool hayMas) {
    if (cantidad == 0) {
        cout << "No hay vehiculos para mostrar." << endl;
        return;
    }
    cout << left << setw(5) << "ID" << setw(10) << "Placa" << setw(26) << "Vehiculo"
         << setw(6) << "Anio" << setw(10) << "Km" << "Propietario" << endl;
    cout << "-------------------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        char vehiculo[110];
        strcpy(vehiculo, lista[i].marca);
        strcat(vehiculo, " ");            // strcat = pega un texto al final de otro
        strcat(vehiculo, lista[i].modelo);
        cout << left << setw(5) << lista[i].id << setw(10) << lista[i].placa
             << setw(26) << vehiculo << setw(6) << lista[i].anio
             << setw(10) << lista[i].kilometraje << lista[i].nombrePropietario << endl;
    }
    cout << "Total mostrado: " << cantidad << endl;
    if (hayMas) {
        cout << "(Hay mas resultados: solo se muestran los primeros " << MAX_RESULTADOS << ")" << endl;
    }
}

void menuRegistrarVehiculo() {
    Vehiculo v = {};
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Placa: ", v.placa, 10)) return;
    if (!leerTexto("Marca: ", v.marca, 50)) return;
    if (!leerTexto("Modelo: ", v.modelo, 50)) return;
    v.anio = leerEntero("Anio (1950-2027): ", 1950, 2027);
    v.kilometraje = leerEntero("Kilometraje: ", 0, 9999999);
    if (!leerTexto("Nombre del propietario: ", v.nombrePropietario, 100)) return;
    if (!leerTexto("Cedula (12345678 o V-12345678): ", v.cedulaPropietario, 20)) return;
    if (!leerTexto("Telefono: ", v.telefonoPropietario, 20)) return;
    if (!confirmar("Guardar el vehiculo?")) {
        cout << "No se guardo el vehiculo." << endl;
        return;
    }
    int id = 0;
    Resultado r = registrarVehiculo(v, &id);
    if (r == OK) {
        cout << "Vehiculo registrado con ID " << id << "." << endl;
    } else if (r == DUPLICADO) {
        cout << "ERROR: La placa '" << v.placa << "' ya esta registrada." << endl;
    } else if (r == DATO_INVALIDO) {
        cout << "ERROR: Datos invalidos. Placa de 6 o 7 letras/numeros, cedula solo numeros"
             << " y marca, modelo y nombre no vacios." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void menuBuscarVehiculo() {
    cout << "1. Por ID   2. Por placa   3. Por propietario" << endl;
    int op = leerEntero("Opcion: ", 1, 3);
    Vehiculo v;
    if (op == 1) {
        int id = leerEntero("ID del vehiculo: ", 1, 999999);
        Resultado r = obtenerVehiculoPorID(id, &v);
        if (r == OK || r == ELIMINADO) {
            // Mensaje de depuracion que pide el Caso de Uso 2
            cout << "[DEPURACION] Registro leido en el byte " << posicionEnDiscoVehiculo(id)
                 << " de datos/vehiculos.bin (sin recorrer el archivo)" << endl;
        }
        if (r == OK) {
            mostrarVehiculo(v);
        } else {
            avisoVehiculo(r, id, v);
        }
    } else if (op == 2) {
        char placa[10];
        if (!leerTexto("Placa exacta: ", placa, 10)) return;
        Resultado r = obtenerVehiculoPorPlaca(placa, &v);
        if (r == OK) {
            mostrarVehiculo(v);
        } else if (r == NO_EXISTE) {
            cout << "No hay un vehiculo activo con placa '" << placa << "'." << endl;
        } else {
            cout << "ERROR: " << textoResultado(r) << endl;
        }
    } else {
        char texto[100];
        if (!leerTexto("Parte del nombre del propietario: ", texto, 100)) return;
        Vehiculo lista[MAX_RESULTADOS];
        int cantidad = 0;
        bool hayMas = false;
        Resultado r = buscarVehiculosPorPropietario(texto, lista, &cantidad, &hayMas);
        if (r != OK) {
            cout << "ERROR: " << textoResultado(r) << endl;
            return;
        }
        mostrarListaVehiculos(lista, cantidad, hayMas);
    }
}

void menuActualizarVehiculo() {
    int id = leerEntero("ID del vehiculo: ", 1, 999999);
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r != OK) {
        avisoVehiculo(r, id, v);
        return;
    }
    mostrarVehiculo(v);
    cout << "Escriba los datos nuevos (la placa y el kilometraje no se cambian)." << endl;
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    Vehiculo d = v;
    if (!leerTexto("Marca: ", d.marca, 50)) return;
    if (!leerTexto("Modelo: ", d.modelo, 50)) return;
    d.anio = leerEntero("Anio (1950-2027): ", 1950, 2027);
    if (!leerTexto("Nombre del propietario: ", d.nombrePropietario, 100)) return;
    if (!leerTexto("Cedula: ", d.cedulaPropietario, 20)) return;
    if (!leerTexto("Telefono: ", d.telefonoPropietario, 20)) return;
    if (!confirmar("Guardar los cambios?")) {
        cout << "No se guardaron los cambios." << endl;
        return;
    }
    r = actualizarVehiculo(id, d);
    if (r == OK) {
        cout << "Vehiculo actualizado." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void menuEliminarVehiculo() {
    int id = leerEntero("ID del vehiculo: ", 1, 999999);
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r != OK) {
        avisoVehiculo(r, id, v);
        return;
    }
    mostrarVehiculo(v);
    if (!confirmar("Seguro que desea eliminar este vehiculo?")) {
        return;
    }
    r = eliminarVehiculo(id);
    if (r == OK) {
        cout << "Vehiculo eliminado (borrado logico)." << endl;
    } else if (r == TIENE_ORDENES) {
        OrdenServicio ordenes[MAX_RESULTADOS];
        int n = 0;
        bool mas = false;
        listarOrdenesPorVehiculo(id, ordenes, &n, &mas);
        cout << "ERROR: El vehiculo '" << v.placa << "' tiene " << n << (mas ? " o mas" : "")
             << " ordenes registradas y no puede eliminarse." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void menuListarVehiculos() {
    Vehiculo lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    Resultado r = listarVehiculos(lista, &cantidad, &hayMas);
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }
    mostrarListaVehiculos(lista, cantidad, hayMas);
}

void menuVehiculos() {
    int op = -1;
    while (op != 0) {
        encabezado("GESTION DE VEHICULOS");
        opcion("1. Registrar vehiculo");
        opcion("2. Buscar vehiculo");
        opcion("3. Actualizar vehiculo");
        opcion("4. Eliminar vehiculo");
        opcion("5. Listar vehiculos");
        opcion("0. Volver al menu principal");
        pie();
        op = leerEntero("Seleccione una opcion: ", 0, 5);
        if (op == 1) menuRegistrarVehiculo();
        if (op == 2) menuBuscarVehiculo();
        if (op == 3) menuActualizarVehiculo();
        if (op == 4) menuEliminarVehiculo();
        if (op == 5) menuListarVehiculos();
    }
}

// =====================================================================
// MECANICOS
// =====================================================================

static void avisoMecanico(Resultado r, int id, Mecanico m) {
    if (r == NO_EXISTE) {
        cout << "No existe un mecanico con ID " << id << "." << endl;
    } else if (r == ELIMINADO) {
        cout << "El mecanico ID " << id << " (" << m.nombre << ") esta DADO DE BAJA." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void mostrarMecanico(Mecanico m, int ordenesAbiertas) {
    cout << "  ID " << m.id << " - " << m.nombre << " (" << m.especialidad << ")";
    if (m.eliminado) {
        cout << "  [DADO DE BAJA]";
    }
    cout << endl;
    cout << "  Cedula: " << m.cedula << "  Ingreso: " << m.fechaIngreso
         << "  Ordenes abiertas: " << ordenesAbiertas << endl;
}

void menuRegistrarMecanico() {
    Mecanico m = {};
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Nombre: ", m.nombre, 100)) return;
    if (!leerTexto("Cedula (12345678 o V-12345678): ", m.cedula, 20)) return;
    cout << "Especialidad: 1.MOTOR 2.FRENOS 3.ELECTRICIDAD 4.SUSPENSION 5.GENERAL" << endl;
    int e = leerEntero("Opcion: ", 1, 5);
    const char* especialidades[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    strcpy(m.especialidad, especialidades[e - 1]);   // opcion 1 = posicion 0
    if (!confirmar("Guardar el mecanico?")) {
        cout << "No se guardo el mecanico." << endl;
        return;
    }
    int id = 0;
    Resultado r = registrarMecanico(m, &id);
    if (r == OK) {
        cout << "Mecanico registrado con ID " << id << "." << endl;
    } else if (r == DUPLICADO) {
        cout << "ERROR: Ya existe un mecanico con la cedula '" << m.cedula << "'." << endl;
    } else if (r == DATO_INVALIDO) {
        cout << "ERROR: El nombre no puede estar vacio y la cedula debe tener solo numeros." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void menuBuscarMecanico() {
    int id = leerEntero("ID del mecanico: ", 1, 999999);
    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, &m);
    if (r == OK || r == ELIMINADO) {        // el dado de baja tambien se muestra, marcado
        int abiertas = 0;
        contarOrdenesAbiertas(id, &abiertas);
        mostrarMecanico(m, abiertas);
    } else {
        avisoMecanico(r, id, m);
    }
}

void menuListarMecanicos() {
    Mecanico lista[MAX_RESULTADOS];
    int abiertas[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    Resultado r = listarMecanicos(lista, abiertas, &cantidad, &hayMas);
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }
    if (cantidad == 0) {
        cout << "No hay mecanicos registrados." << endl;
        return;
    }
    cout << left << setw(5) << "ID" << setw(24) << "Nombre" << setw(14) << "Cedula"
         << setw(14) << "Especialidad" << setw(10) << "Abiertas" << "Estado" << endl;
    cout << "---------------------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << left << setw(5) << lista[i].id << setw(24) << lista[i].nombre
             << setw(14) << lista[i].cedula << setw(14) << lista[i].especialidad
             << setw(10) << abiertas[i] << (lista[i].eliminado ? "DADO DE BAJA" : "ACTIVO") << endl;
    }
    cout << "Total mostrado: " << cantidad << endl;
    if (hayMas) {
        cout << "(Hay mas resultados: solo se muestran los primeros " << MAX_RESULTADOS << ")" << endl;
    }
}

void menuDarDeBajaMecanico() {
    int id = leerEntero("ID del mecanico: ", 1, 999999);
    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, &m);
    if (r != OK) {
        avisoMecanico(r, id, m);
        return;
    }
    int abiertas = 0;
    contarOrdenesAbiertas(id, &abiertas);
    mostrarMecanico(m, abiertas);
    if (!confirmar("Seguro que desea dar de baja a este mecanico?")) {
        return;
    }
    r = darDeBajaMecanico(id);
    if (r == OK) {
        cout << "Mecanico dado de baja. Sus ordenes anteriores conservan su nombre." << endl;
    } else if (r == TIENE_ORDENES_ABIERTAS) {
        cout << "ERROR: El mecanico '" << m.nombre << "' tiene " << abiertas
             << " ordenes abiertas y no puede darse de baja." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void menuMecanicos() {
    int op = -1;
    while (op != 0) {
        encabezado("GESTION DE MECANICOS");
        opcion("1. Registrar mecanico");
        opcion("2. Buscar mecanico");
        opcion("3. Listar mecanicos");
        opcion("4. Dar de baja mecanico");
        opcion("0. Volver al menu principal");
        pie();
        op = leerEntero("Seleccione una opcion: ", 0, 4);
        if (op == 1) menuRegistrarMecanico();
        if (op == 2) menuBuscarMecanico();
        if (op == 3) menuListarMecanicos();
        if (op == 4) menuDarDeBajaMecanico();
    }
}

// =====================================================================
// REPUESTOS
// =====================================================================

static void avisoRepuesto(Resultado r, int id, Repuesto rep) {
    if (r == NO_EXISTE) {
        cout << "No existe un repuesto con ID " << id << "." << endl;
    } else if (r == ELIMINADO) {
        cout << "El repuesto ID " << id << " (" << rep.nombre << ") fue ELIMINADO." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

void mostrarRepuesto(Repuesto r) {
    cout << fixed << setprecision(2);       // precios siempre con 2 decimales
    cout << "  ID " << r.id << " - " << r.codigo << " - " << r.nombre << " (" << r.marca << ")";
    if (r.eliminado) {
        cout << "  [ELIMINADO]";
    }
    cout << endl;
    cout << "  Precio: " << r.precio << "  Stock: " << r.stock << "  Minimo: " << r.stockMinimo;
    if (!r.eliminado && esBajoStock(r)) {
        cout << "  BAJO STOCK";
    }
    cout << endl;
}

void mostrarListaRepuestos(Repuesto lista[], int cantidad, bool hayMas) {
    if (cantidad == 0) {
        cout << "No hay repuestos para mostrar." << endl;
        return;
    }
    cout << fixed << setprecision(2);
    cout << left << setw(5) << "ID" << setw(10) << "Codigo" << setw(24) << "Nombre"
         << setw(12) << "Marca" << setw(10) << "Precio" << setw(7) << "Stock" << "Min" << endl;
    cout << "---------------------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << left << setw(5) << lista[i].id << setw(10) << lista[i].codigo
             << setw(24) << lista[i].nombre << setw(12) << lista[i].marca
             << setw(10) << lista[i].precio << setw(7) << lista[i].stock << lista[i].stockMinimo;
        if (esBajoStock(lista[i])) {
            cout << "  BAJO STOCK";
        }
        cout << endl;
    }
    cout << "Total mostrado: " << cantidad << endl;
    if (hayMas) {
        cout << "(Hay mas resultados: solo se muestran los primeros " << MAX_RESULTADOS << ")" << endl;
    }
}

void menuRegistrarRepuesto() {
    Repuesto r = {};
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Codigo: ", r.codigo, 20)) return;
    if (!leerTexto("Nombre: ", r.nombre, 100)) return;
    if (!leerTexto("Marca: ", r.marca, 50)) return;
    int centavos = leerEntero("Precio en centavos (ej: 2550 = 25.50): ", 1, 99999999);
    r.precio = centavos / 100.0f;
    r.stock = leerEntero("Stock inicial: ", 0, 999999);
    r.stockMinimo = leerEntero("Stock minimo: ", 0, 999999);
    if (!confirmar("Guardar el repuesto?")) {
        cout << "No se guardo el repuesto." << endl;
        return;
    }
    int id = 0;
    Resultado res = registrarRepuesto(r, &id);
    if (res == OK) {
        cout << "Repuesto registrado con ID " << id << "." << endl;
    } else if (res == DUPLICADO) {
        cout << "ERROR: Ya existe un repuesto activo con el codigo '" << r.codigo << "'." << endl;
    } else if (res == DATO_INVALIDO) {
        cout << "ERROR: Codigo, nombre y marca no pueden estar vacios; precio mayor que 0." << endl;
    } else {
        cout << "ERROR: " << textoResultado(res) << endl;
    }
}

void menuBuscarRepuesto() {
    cout << "1. Por ID   2. Por codigo   3. Por nombre" << endl;
    int op = leerEntero("Opcion: ", 1, 3);
    Repuesto r;
    if (op == 1) {
        int id = leerEntero("ID del repuesto: ", 1, 999999);
        Resultado res = obtenerRepuestoPorID(id, &r);
        if (res == OK) {
            mostrarRepuesto(r);
        } else {
            avisoRepuesto(res, id, r);
        }
    } else if (op == 2) {
        char codigo[20];
        if (!leerTexto("Codigo exacto: ", codigo, 20)) return;
        Resultado res = obtenerRepuestoPorCodigo(codigo, &r);
        if (res == OK) {
            mostrarRepuesto(r);
        } else if (res == NO_EXISTE) {
            cout << "No hay un repuesto activo con codigo '" << codigo << "'." << endl;
        } else {
            cout << "ERROR: " << textoResultado(res) << endl;
        }
    } else {
        char texto[100];
        if (!leerTexto("Parte del nombre: ", texto, 100)) return;
        Repuesto lista[MAX_RESULTADOS];
        int cantidad = 0;
        bool hayMas = false;
        Resultado res = buscarRepuestosPorNombre(texto, lista, &cantidad, &hayMas);
        if (res != OK) {
            cout << "ERROR: " << textoResultado(res) << endl;
            return;
        }
        mostrarListaRepuestos(lista, cantidad, hayMas);
    }
}

void menuEntradaMercancia() {
    int id = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        avisoRepuesto(res, id, r);
        return;
    }
    mostrarRepuesto(r);
    int cantidad = leerEntero("Unidades que llegaron: ", 1, 999999);
    res = registrarEntradaRepuesto(id, cantidad);
    if (res == OK) {
        obtenerRepuestoPorID(id, &r);       // lo leo otra vez del disco para mostrar el stock nuevo
        cout << "Entrada registrada. Stock de " << r.nombre << " = " << r.stock << endl;
    } else {
        cout << "ERROR: " << textoResultado(res) << endl;
    }
}

void menuListarRepuestos() {
    Repuesto lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    Resultado res = listarRepuestos(lista, &cantidad, &hayMas);
    if (res != OK) {
        cout << "ERROR: " << textoResultado(res) << endl;
        return;
    }
    mostrarListaRepuestos(lista, cantidad, hayMas);
}

void menuCambiarPrecio() {
    int id = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        avisoRepuesto(res, id, r);
        return;
    }
    mostrarRepuesto(r);
    int centavos = leerEntero("Precio nuevo en centavos (ej: 2550 = 25.50): ", 1, 99999999);
    if (!confirmar("Guardar el precio nuevo?")) {
        return;
    }
    res = cambiarPrecioRepuesto(id, centavos / 100.0f);
    if (res == OK) {
        cout << "Precio actualizado. Las ordenes ya registradas conservan su precio." << endl;
    } else {
        cout << "ERROR: " << textoResultado(res) << endl;
    }
}

void menuEliminarRepuesto() {
    int id = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        avisoRepuesto(res, id, r);
        return;
    }
    mostrarRepuesto(r);
    if (!confirmar("Seguro que desea eliminar este repuesto?")) {
        return;
    }
    res = eliminarRepuesto(id);
    if (res == OK) {
        cout << "Repuesto eliminado (borrado logico)." << endl;
    } else if (res == TIENE_ORDENES) {
        cout << "ERROR: El repuesto '" << r.nombre << "' aparece en ordenes y no puede eliminarse." << endl;
    } else {
        cout << "ERROR: " << textoResultado(res) << endl;
    }
}

void menuRepuestos() {
    int op = -1;
    while (op != 0) {
        encabezado("INVENTARIO DE REPUESTOS");
        opcion("1. Registrar repuesto");
        opcion("2. Buscar repuesto");
        opcion("3. Entrada de mercancia");
        opcion("4. Listar repuestos");
        opcion("5. Cambiar precio");
        opcion("6. Eliminar repuesto");
        opcion("0. Volver al menu principal");
        pie();
        op = leerEntero("Seleccione una opcion: ", 0, 6);
        if (op == 1) menuRegistrarRepuesto();
        if (op == 2) menuBuscarRepuesto();
        if (op == 3) menuEntradaMercancia();
        if (op == 4) menuListarRepuestos();
        if (op == 5) menuCambiarPrecio();
        if (op == 6) menuEliminarRepuesto();
    }
}