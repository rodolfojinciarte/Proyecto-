#include "presentacion.h"
#include "logica.h"
#include "utilidades.h"
#include <iostream>
#include <iomanip>    // setw (ancho de columnas) y setprecision (decimales)
#include <cstring>
#include <cstdio>     // snprintf: arma un texto con numeros adentro
#include <ctime>
using namespace std;

// =====================================================================
// AYUDAS
// =====================================================================

// QUE HACE: devuelve un mensaje en texto para cada Resultado.
// switch = un if con muchos casos: segun el valor de r, devuelve un texto.
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

// QUE HACE: convierte la fecha guardada como numero (time_t) en texto "AAAA-MM-DD".
void textoFecha(time_t t, char* destino) {
    tm* f = localtime(&t);
    strftime(destino, 11, "%Y-%m-%d", f);
}

// QUE HACE: dibuja el borde de arriba de un menu con su titulo (44 caracteres de ancho).
void encabezado(const char* titulo) {
    cout << endl;
    cout << "+==========================================+" << endl;
    cout << "|  " << left << setw(40) << titulo << "|" << endl;
    cout << "+==========================================+" << endl;
}

// QUE HACE: dibuja una linea del menu con sus bordes a los lados.
void opcion(const char* texto) {
    cout << "|  " << left << setw(40) << texto << "|" << endl;
}

// QUE HACE: dibuja el borde de abajo de un menu.
void pie() {
    cout << "+==========================================+" << endl;
}

// QUE HACE: en el primer arranque (no existe taller.bin) pide los datos del taller y los guarda.
// DEVUELVE: true si se guardaron; false si el usuario escribio CANCELAR o hubo un error.
bool menuPrimerArranque() {
    encabezado("CONFIGURACION INICIAL DEL TALLER");
    cout << "Es la primera vez que se ejecuta el sistema." << endl;
    cout << "(Escriba CANCELAR para salir)" << endl;
    while (true) {
        // 1. Pido los datos
        Taller t = {};
        if (!leerTexto("Nombre del taller: ", t.nombre, 100)) return false;
        if (!leerTexto("RIF: ", t.rif, 20)) return false;
        if (!leerTexto("Direccion: ", t.direccion, 200)) return false;
        if (!leerTexto("Telefono: ", t.telefono, 20)) return false;
        // 2. Le pido a la logica que los guarde
        Resultado r = configurarTaller(t);
        if (r == OK) {
            cout << "Taller guardado en datos/taller.bin" << endl;
            return true;
        }
        if (r == ERROR_ARCHIVO) {
            cout << "ERROR: No se pudo crear datos/taller.bin" << endl;
            return false;
        }
        // 3. Datos invalidos: vuelve a preguntar
        cout << "ERROR: El nombre y el RIF no pueden estar vacios. Intente de nuevo." << endl;
    }
}

// =====================================================================
// VEHICULOS
// =====================================================================

// QUE HACE: muestra el mensaje correcto cuando no se pudo obtener un vehiculo por su ID.
static void avisoVehiculo(Resultado r, int id, Vehiculo v) {
    if (r == NO_EXISTE) {
        cout << "No existe un vehiculo con ID " << id << "." << endl;
    } else if (r == ELIMINADO) {
        cout << "El vehiculo ID " << id << " (" << v.placa << ") fue ELIMINADO." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: muestra todos los datos de un vehiculo.
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

// QUE HACE: muestra una lista de vehiculos como tabla, y avisa si hay mas resultados.
void mostrarListaVehiculos(Vehiculo lista[], int cantidad, bool hayMas) {
    if (cantidad == 0) {
        cout << "No hay vehiculos para mostrar." << endl;
        return;
    }
    // Encabezado de la tabla (setw = ancho de cada columna)
    cout << left << setw(5) << "ID" << setw(10) << "Placa" << setw(26) << "Vehiculo"
         << setw(6) << "Anio" << setw(10) << "Km" << "Propietario" << endl;
    cout << "-------------------------------------------------------------------------" << endl;
    // Una fila por vehiculo
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

// QUE HACE: pide los datos de un vehiculo nuevo, confirma y le pide a la logica registrarlo.
void menuRegistrarVehiculo() {
    // 1. Pido los datos (CANCELAR en cualquier texto sale sin guardar)
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
    // 2. Confirmacion
    if (!confirmar("Guardar el vehiculo?")) {
        cout << "No se guardo el vehiculo." << endl;
        return;
    }
    // 3. Le pido a la logica que lo registre y muestro el resultado
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

// QUE HACE: busca vehiculos por ID (acceso directo), por placa o por propietario.
void menuBuscarVehiculo() {
    cout << "1. Por ID   2. Por placa   3. Por propietario" << endl;
    int op = leerEntero("Opcion: ", 1, 3);
    Vehiculo v;
    if (op == 1) {
        // ----- Por ID: acceso aleatorio -----
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
        // ----- Por placa exacta -----
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
        // ----- Por parte del nombre del propietario -----
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

// QUE HACE: muestra un vehiculo, pide los datos nuevos y le pide a la logica actualizarlo.
void menuActualizarVehiculo() {
    // 1. Busco el vehiculo
    int id = leerEntero("ID del vehiculo: ", 1, 999999);
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r != OK) {
        avisoVehiculo(r, id, v);
        return;
    }
    mostrarVehiculo(v);
    // 2. Pido los datos nuevos
    cout << "Escriba los datos nuevos (la placa y el kilometraje no se cambian)." << endl;
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    Vehiculo d = v;
    if (!leerTexto("Marca: ", d.marca, 50)) return;
    if (!leerTexto("Modelo: ", d.modelo, 50)) return;
    d.anio = leerEntero("Anio (1950-2027): ", 1950, 2027);
    if (!leerTexto("Nombre del propietario: ", d.nombrePropietario, 100)) return;
    if (!leerTexto("Cedula: ", d.cedulaPropietario, 20)) return;
    if (!leerTexto("Telefono: ", d.telefonoPropietario, 20)) return;
    // 3. Confirmo y guardo
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

// QUE HACE: muestra un vehiculo, confirma y le pide a la logica eliminarlo (borrado logico).
void menuEliminarVehiculo() {
    // 1. Busco el vehiculo
    int id = leerEntero("ID del vehiculo: ", 1, 999999);
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r != OK) {
        avisoVehiculo(r, id, v);
        return;
    }
    mostrarVehiculo(v);
    // 2. Confirmo
    if (!confirmar("Seguro que desea eliminar este vehiculo?")) {
        return;
    }
    // 3. Elimino y muestro el resultado
    r = eliminarVehiculo(id);
    if (r == OK) {
        cout << "Vehiculo eliminado (borrado logico)." << endl;
    } else if (r == TIENE_ORDENES) {
        // cuento sus ordenes para decir cuantas tiene
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

// QUE HACE: pide a la logica la lista de vehiculos activos y la muestra.
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

// QUE HACE: submenu de vehiculos; se repite hasta que el usuario elige 0.
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

// QUE HACE: muestra el mensaje correcto cuando no se pudo obtener un mecanico por su ID.
static void avisoMecanico(Resultado r, int id, Mecanico m) {
    if (r == NO_EXISTE) {
        cout << "No existe un mecanico con ID " << id << "." << endl;
    } else if (r == ELIMINADO) {
        cout << "El mecanico ID " << id << " (" << m.nombre << ") esta DADO DE BAJA." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: muestra los datos de un mecanico y cuantas ordenes abiertas tiene.
void mostrarMecanico(Mecanico m, int ordenesAbiertas) {
    cout << "  ID " << m.id << " - " << m.nombre << " (" << m.especialidad << ")";
    if (m.eliminado) {
        cout << "  [DADO DE BAJA]";
    }
    cout << endl;
    cout << "  Cedula: " << m.cedula << "  Ingreso: " << m.fechaIngreso
         << "  Ordenes abiertas: " << ordenesAbiertas << endl;
}

// QUE HACE: pide los datos de un mecanico nuevo, confirma y le pide a la logica registrarlo.
void menuRegistrarMecanico() {
    // 1. Pido los datos
    Mecanico m = {};
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Nombre: ", m.nombre, 100)) return;
    if (!leerTexto("Cedula (12345678 o V-12345678): ", m.cedula, 20)) return;
    // 2. La especialidad se escoge por numero para que no haya errores al escribirla
    cout << "Especialidad: 1.MOTOR 2.FRENOS 3.ELECTRICIDAD 4.SUSPENSION 5.GENERAL" << endl;
    int e = leerEntero("Opcion: ", 1, 5);
    const char* especialidades[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    strcpy(m.especialidad, especialidades[e - 1]);   // opcion 1 = posicion 0
    // 3. Confirmo y registro
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

// QUE HACE: busca un mecanico por ID y lo muestra (si esta de baja, lo muestra marcado).
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

// QUE HACE: muestra la tabla de mecanicos (activos y de baja) con sus ordenes abiertas.
void menuListarMecanicos() {
    // 1. Pido la lista a la logica
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
    // 2. La muestro como tabla
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

// QUE HACE: muestra un mecanico, confirma y le pide a la logica darlo de baja.
void menuDarDeBajaMecanico() {
    // 1. Busco el mecanico
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
    // 2. Confirmo y doy de baja
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

// QUE HACE: submenu de mecanicos; se repite hasta que el usuario elige 0.
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

// QUE HACE: muestra el mensaje correcto cuando no se pudo obtener un repuesto por su ID.
static void avisoRepuesto(Resultado r, int id, Repuesto rep) {
    if (r == NO_EXISTE) {
        cout << "No existe un repuesto con ID " << id << "." << endl;
    } else if (r == ELIMINADO) {
        cout << "El repuesto ID " << id << " (" << rep.nombre << ") fue ELIMINADO." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: muestra los datos de un repuesto y lo marca si esta en BAJO STOCK.
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

// QUE HACE: muestra una lista de repuestos como tabla, marcando los de BAJO STOCK.
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

// QUE HACE: pide los datos de un repuesto nuevo (precio en centavos), confirma y lo registra.
void menuRegistrarRepuesto() {
    // 1. Pido los datos
    Repuesto r = {};
    cout << "(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Codigo: ", r.codigo, 20)) return;
    if (!leerTexto("Nombre: ", r.nombre, 100)) return;
    if (!leerTexto("Marca: ", r.marca, 50)) return;
    int centavos = leerEntero("Precio en centavos (ej: 2550 = 25.50): ", 1, 99999999);
    r.precio = centavos / 100.0f;           // .0f para que no sea division entera
    r.stock = leerEntero("Stock inicial: ", 0, 999999);
    r.stockMinimo = leerEntero("Stock minimo: ", 0, 999999);
    // 2. Confirmo y registro
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

// QUE HACE: busca repuestos por ID, por codigo exacto o por parte del nombre.
void menuBuscarRepuesto() {
    cout << "1. Por ID   2. Por codigo   3. Por nombre" << endl;
    int op = leerEntero("Opcion: ", 1, 3);
    Repuesto r;
    if (op == 1) {
        // ----- Por ID -----
        int id = leerEntero("ID del repuesto: ", 1, 999999);
        Resultado res = obtenerRepuestoPorID(id, &r);
        if (res == OK) {
            mostrarRepuesto(r);
        } else {
            avisoRepuesto(res, id, r);
        }
    } else if (op == 2) {
        // ----- Por codigo exacto -----
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
        // ----- Por parte del nombre -----
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

// QUE HACE: registra la llegada de unidades de un repuesto y muestra el stock nuevo.
void menuEntradaMercancia() {
    // 1. Busco el repuesto
    int id = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        avisoRepuesto(res, id, r);
        return;
    }
    mostrarRepuesto(r);
    // 2. Pido las unidades y registro la entrada
    int cantidad = leerEntero("Unidades que llegaron: ", 1, 999999);
    res = registrarEntradaRepuesto(id, cantidad);
    if (res == OK) {
        obtenerRepuestoPorID(id, &r);       // lo leo otra vez del disco para mostrar el stock nuevo
        cout << "Entrada registrada. Stock de " << r.nombre << " = " << r.stock << endl;
    } else {
        cout << "ERROR: " << textoResultado(res) << endl;
    }
}

// QUE HACE: pide a la logica la lista de repuestos activos y la muestra.
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

// QUE HACE: muestra un repuesto, pide el precio nuevo (en centavos), confirma y lo cambia.
void menuCambiarPrecio() {
    // 1. Busco el repuesto
    int id = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        avisoRepuesto(res, id, r);
        return;
    }
    mostrarRepuesto(r);
    // 2. Pido el precio nuevo y confirmo
    int centavos = leerEntero("Precio nuevo en centavos (ej: 2550 = 25.50): ", 1, 99999999);
    if (!confirmar("Guardar el precio nuevo?")) {
        return;
    }
    // 3. Lo cambio
    res = cambiarPrecioRepuesto(id, centavos / 100.0f);
    if (res == OK) {
        cout << "Precio actualizado. Las ordenes ya registradas conservan su precio." << endl;
    } else {
        cout << "ERROR: " << textoResultado(res) << endl;
    }
}

// QUE HACE: muestra un repuesto, confirma y le pide a la logica eliminarlo (borrado logico).
void menuEliminarRepuesto() {
    // 1. Busco el repuesto
    int id = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, &r);
    if (res != OK) {
        avisoRepuesto(res, id, r);
        return;
    }
    mostrarRepuesto(r);
    // 2. Confirmo y elimino
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

// QUE HACE: submenu de repuestos; se repite hasta que el usuario elige 0.
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

// =====================================================================
// ORDENES
// =====================================================================

// QUE HACE: pide el numero de orden y la lee.
// DEVUELVE: true si la pudo leer; si no, muestra el error y devuelve false.
static bool pedirOrden(OrdenServicio* o) {
    int id = leerEntero("Numero de orden: ", 1, 999999);
    Resultado r = obtenerOrdenPorID(id, o);
    if (r == NO_EXISTE) {
        cout << "No existe la orden #" << id << "." << endl;
        return false;
    }
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return false;
    }
    return true;
}

// QUE HACE: muestra el mensaje de error cuando la orden no esta ABIERTA.
static void avisoEstadoOrden(OrdenServicio o) {
    cout << "ERROR: La orden #" << o.id << " esta " << o.estado << " y no puede modificarse." << endl;
}

// QUE HACE: arma un texto con la placa, marca y modelo del vehiculo (aunque este eliminado).
static void describirVehiculo(int id, char* destino) {
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, &v);
    if (r == OK || r == ELIMINADO) {
        strcpy(destino, v.placa);
        strcat(destino, " - ");
        strcat(destino, v.marca);
        strcat(destino, " ");
        strcat(destino, v.modelo);
        if (r == ELIMINADO) {
            strcat(destino, " [ELIMINADO]");
        }
    } else {
        strcpy(destino, "(vehiculo no encontrado)");
    }
}

// QUE HACE: arma un texto con el nombre y la especialidad del mecanico (aunque este de baja).
static void describirMecanico(int id, char* destino) {
    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, &m);
    if (r == OK || r == ELIMINADO) {
        strcpy(destino, m.nombre);
        strcat(destino, " (");
        strcat(destino, m.especialidad);
        strcat(destino, ")");
        if (r == ELIMINADO) {
            strcat(destino, " [BAJA]");
        }
    } else {
        strcpy(destino, "(mecanico no encontrado)");
    }
}

// QUE HACE: muestra una orden en dos lineas (para los listados), con nombres y no solo IDs.
void mostrarResumenOrden(OrdenServicio o) {
    char vehiculo[200];
    char mecanico[200];
    describirVehiculo(o.idVehiculo, vehiculo);
    describirMecanico(o.idMecanico, mecanico);
    cout << fixed << setprecision(2);
    cout << "  #" << o.id << "  " << o.fechaApertura << "  " << left << setw(9) << o.estado
         << "  Total: " << calcularTotalOrden(o) << endl;
    cout << "      Vehiculo: " << vehiculo << "   Mecanico: " << mecanico << endl;
}

// QUE HACE: muestra la orden completa como una factura: datos, lineas y totales.
static void mostrarDetalleOrden(OrdenServicio o) {
    // ----- Datos de la orden -----
    cout << fixed << setprecision(2);
    cout << endl << "+==================================================================+" << endl;
    cout << "  ORDEN DE SERVICIO #" << o.id << endl;
    cout << "+==================================================================+" << endl;
    cout << "  Estado   : " << o.estado << "     Apertura: " << o.fechaApertura
         << "     Cierre: " << (strlen(o.fechaCierre) > 0 ? o.fechaCierre : "-") << endl;

    Vehiculo v;
    Resultado rv = obtenerVehiculoPorID(o.idVehiculo, &v);
    if (rv == OK || rv == ELIMINADO) {
        cout << "  Vehiculo : " << v.placa << " - " << v.marca << " " << v.modelo << " " << v.anio
             << (v.eliminado ? " [ELIMINADO]" : "") << endl;
        cout << "  Dueno    : " << v.nombrePropietario << endl;
    } else {
        cout << "  Vehiculo : (no encontrado, ID " << o.idVehiculo << ")" << endl;
    }
    char mecanico[200];
    describirMecanico(o.idMecanico, mecanico);
    cout << "  Mecanico : " << mecanico << endl;
    cout << "  Km ingreso: " << o.kilometrajeIngreso << endl;
    cout << "  Falla    : " << o.descripcionFalla << endl;
    cout << "+------------------------------------------------------------------+" << endl;

    // ----- Lineas (una por repuesto) -----
    cout << "  " << left << setw(10) << "Codigo" << setw(26) << "Repuesto"
         << right << setw(6) << "Cant" << setw(10) << "P.Unit" << setw(10) << "Total" << endl;
    float repuestos = 0;
    for (int i = 0; i < o.numLineas; i++) {
        Repuesto r;
        Resultado rr = obtenerRepuestoPorID(o.lineas[i].idRepuesto, &r);
        bool hay = (rr == OK || rr == ELIMINADO);
        float totalLinea = o.lineas[i].cantidad * o.lineas[i].precioUnitario;
        repuestos += totalLinea;
        cout << "  " << left << setw(10) << (hay ? r.codigo : "?")
             << setw(26) << (hay ? r.nombre : "(no existe)")
             << right << setw(6) << o.lineas[i].cantidad
             << setw(10) << o.lineas[i].precioUnitario      // precio HISTORICO
             << setw(10) << totalLinea << endl;
    }
    if (o.numLineas == 0) {
        cout << "  (sin repuestos)" << endl;
    }

    // ----- Totales -----
    cout << "+------------------------------------------------------------------+" << endl;
    cout << right << setw(54) << "Repuestos:" << setw(10) << repuestos << endl;
    cout << right << setw(54) << "Mano de obra:" << setw(10) << o.manoObra << endl;
    cout << right << setw(54) << "TOTAL:" << setw(10) << calcularTotalOrden(o) << endl;
    cout << "+==================================================================+" << endl;
    cout << left;     // dejo la alineacion como estaba para los menus
}

// QUE HACE: pide vehiculo, mecanico, km y falla, confirma y le pide a la logica abrir la orden.
void menuAbrirOrden() {
    // 1. Vehiculo: debe existir y estar activo
    int idV = leerEntero("ID del vehiculo: ", 1, 999999);
    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(idV, &v);
    if (r != OK) {
        avisoVehiculo(r, idV, v);
        return;
    }
    cout << "  Vehiculo: " << v.placa << " - " << v.marca << " " << v.modelo
         << " (" << v.kilometraje << " km)" << endl;

    // 2. Mecanico: debe existir y no estar dado de baja
    int idM = leerEntero("ID del mecanico: ", 1, 999999);
    Mecanico m;
    r = obtenerMecanicoPorID(idM, &m);
    if (r == ELIMINADO) {
        cout << "ERROR: El mecanico '" << m.nombre << "' esta dado de baja y no recibe ordenes nuevas." << endl;
        return;
    }
    if (r != OK) {
        avisoMecanico(r, idM, m);
        return;
    }
    cout << "  Mecanico: " << m.nombre << " (" << m.especialidad << ")" << endl;

    // 3. Kilometraje, falla y confirmacion
    int km = leerEntero("Kilometraje de ingreso: ", 0, 9999999);
    char falla[200];
    if (!leerTexto("Descripcion de la falla: ", falla, 200)) return;
    if (!confirmar("Abrir la orden?")) {
        cout << "No se abrio la orden." << endl;
        return;
    }

    // 4. Le pido a la logica que la abra y muestro el resultado
    int idOrden = 0;
    r = abrirOrden(idV, idM, km, falla, &idOrden);
    if (r == OK) {
        cout << "Orden #" << idOrden << " abierta." << endl;
    } else if (r == VEHICULO_CON_ORDEN_ABIERTA) {
        // busco cual es la orden abierta para decir su numero
        OrdenServicio lista[MAX_RESULTADOS];
        int n = 0;
        bool mas = false;
        listarOrdenesPorVehiculo(idV, lista, &n, &mas);
        int numero = 0;
        for (int i = 0; i < n; i++) {
            if (strcmp(lista[i].estado, "ABIERTA") == 0) {
                numero = lista[i].id;
            }
        }
        cout << "ERROR: El vehiculo '" << v.placa << "' ya tiene la orden #" << numero << " ABIERTA." << endl;
    } else if (r == MECANICO_OCUPADO) {
        cout << "ERROR: El mecanico '" << m.nombre << "' ya tiene 3 ordenes abiertas." << endl;
    } else if (r == KILOMETRAJE_MENOR) {
        cout << "ERROR: El kilometraje de ingreso (" << km << ") es menor que el registrado ("
             << v.kilometraje << " km)." << endl;
    } else if (r == DATO_INVALIDO) {
        cout << "ERROR: La descripcion de la falla no puede estar vacia." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: pide orden, repuesto y cantidad, y le pide a la logica agregarlo a la orden.
void menuAgregarRepuestoOrden() {
    // 1. La orden debe existir y estar ABIERTA
    OrdenServicio o;
    if (!pedirOrden(&o)) return;
    if (strcmp(o.estado, "ABIERTA") != 0) {
        avisoEstadoOrden(o);
        return;
    }
    mostrarResumenOrden(o);
    // 2. El repuesto debe existir y estar activo
    int idR = leerEntero("ID del repuesto: ", 1, 999999);
    Repuesto rep;
    Resultado r = obtenerRepuestoPorID(idR, &rep);
    if (r != OK) {
        avisoRepuesto(r, idR, rep);
        return;
    }
    cout << "  " << rep.nombre << " - stock disponible: " << rep.stock << endl;
    // 3. Cantidad y resultado
    int cantidad = leerEntero("Cantidad: ", 1, 999999);
    r = agregarRepuestoAOrden(o.id, idR, cantidad);
    if (r == OK) {
        cout << "Repuesto agregado a la orden #" << o.id << "." << endl;
    } else if (r == STOCK_INSUFICIENTE) {
        cout << "ERROR: Stock insuficiente de '" << rep.nombre << "'. Disponible: " << rep.stock
             << ", solicitado: " << cantidad << "." << endl;
    } else if (r == LIMITE_LINEAS) {
        cout << "ERROR: La orden #" << o.id << " ya tiene " << MAX_LINEAS
             << " lineas; no se puede agregar otro repuesto." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: muestra la orden, pide el repuesto a quitar, confirma y quita su linea completa.
void menuQuitarRepuestoOrden() {
    // 1. La orden debe existir y estar ABIERTA
    OrdenServicio o;
    if (!pedirOrden(&o)) return;
    if (strcmp(o.estado, "ABIERTA") != 0) {
        avisoEstadoOrden(o);
        return;
    }
    mostrarDetalleOrden(o);
    // 2. Repuesto a quitar y confirmacion
    int idR = leerEntero("ID del repuesto a quitar: ", 1, 999999);
    if (!confirmar("Quitar la linea completa de ese repuesto?")) {
        return;
    }
    // 3. Resultado
    Resultado r = quitarRepuestoDeOrden(o.id, idR);
    if (r == OK) {
        cout << "Linea quitada. Las unidades volvieron al inventario." << endl;
    } else if (r == NO_ESTA_EN_ORDEN) {
        cout << "ERROR: El repuesto ID " << idR << " no esta en la orden #" << o.id << "." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: muestra la orden, pide la mano de obra (en centavos), confirma y la cierra.
void menuCerrarOrden() {
    // 1. La orden debe existir y estar ABIERTA
    OrdenServicio o;
    if (!pedirOrden(&o)) return;
    if (strcmp(o.estado, "ABIERTA") != 0) {
        avisoEstadoOrden(o);
        return;
    }
    mostrarDetalleOrden(o);
    // 2. Mano de obra y confirmacion
    int centavos = leerEntero("Mano de obra en centavos (ej: 4000 = 40.00): ", 0, 99999999);
    if (!confirmar("Seguro que desea cerrar la orden?")) {
        return;
    }
    // 3. Resultado
    Resultado r = cerrarOrden(o.id, centavos / 100.0f);
    if (r == OK) {
        obtenerOrdenPorID(o.id, &o);        // la leo otra vez para mostrar el total final
        cout << fixed << setprecision(2);
        cout << "Orden #" << o.id << " CERRADA. Total: " << calcularTotalOrden(o) << endl;
    } else if (r == ORDEN_VACIA) {
        cout << "ERROR: La orden no tiene repuestos; la mano de obra debe ser mayor que 0." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: muestra la orden, confirma y la anula (devuelve todo el stock de sus lineas).
void menuAnularOrden() {
    // 1. La orden debe existir y no estar ya anulada
    OrdenServicio o;
    if (!pedirOrden(&o)) return;
    if (strcmp(o.estado, "ANULADA") == 0) {
        cout << "ERROR: La orden #" << o.id << " ya esta ANULADA." << endl;
        return;
    }
    mostrarDetalleOrden(o);
    // 2. Confirmo y anulo
    if (!confirmar("Seguro que desea anular la orden?")) {
        return;
    }
    Resultado r = anularOrden(o.id);
    if (r == OK) {
        cout << "Orden #" << o.id << " ANULADA. Las unidades de sus lineas volvieron al inventario." << endl;
    } else {
        cout << "ERROR: " << textoResultado(r) << endl;
    }
}

// QUE HACE: pide el numero de orden y muestra su detalle completo.
void menuVerDetalleOrden() {
    OrdenServicio o;
    if (!pedirOrden(&o)) return;
    mostrarDetalleOrden(o);
}

// QUE HACE: lista ordenes por estado (ABIERTA, CERRADA, ANULADA) o el historial de un vehiculo.
void menuListarOrdenes() {
    // 1. Que lista quiere
    cout << "1. Por estado   2. Historial de un vehiculo" << endl;
    int op = leerEntero("Opcion: ", 1, 2);
    OrdenServicio lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    Resultado r;
    if (op == 1) {
        cout << "1. ABIERTA   2. CERRADA   3. ANULADA" << endl;
        int e = leerEntero("Estado: ", 1, 3);
        const char* estados[3] = {"ABIERTA", "CERRADA", "ANULADA"};
        r = listarOrdenesPorEstado(estados[e - 1], lista, &cantidad, &hayMas);
    } else {
        int idV = leerEntero("ID del vehiculo: ", 1, 999999);
        r = listarOrdenesPorVehiculo(idV, lista, &cantidad, &hayMas);
    }
    // 2. La muestro
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }
    if (cantidad == 0) {
        cout << "No hay ordenes para mostrar." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarResumenOrden(lista[i]);
    }
    cout << "Total mostrado: " << cantidad << endl;
    if (hayMas) {
        cout << "(Hay mas resultados: solo se muestran los primeros " << MAX_RESULTADOS << ")" << endl;
    }
}

// QUE HACE: submenu de ordenes; se repite hasta que el usuario elige 0.
void menuOrdenes() {
    int op = -1;
    while (op != 0) {
        encabezado("ORDENES DE SERVICIO");
        opcion("1. Abrir orden");
        opcion("2. Agregar repuesto a orden");
        opcion("3. Quitar repuesto de orden");
        opcion("4. Cerrar orden");
        opcion("5. Anular orden");
        opcion("6. Ver detalle de orden");
        opcion("7. Listar ordenes");
        opcion("0. Volver al menu principal");
        pie();
        op = leerEntero("Seleccione una opcion: ", 0, 7);
        if (op == 1) menuAbrirOrden();
        if (op == 2) menuAgregarRepuestoOrden();
        if (op == 3) menuQuitarRepuestoOrden();
        if (op == 4) menuCerrarOrden();
        if (op == 5) menuAnularOrden();
        if (op == 6) menuVerDetalleOrden();
        if (op == 7) menuListarOrdenes();
    }
}

// =====================================================================
// MANTENIMIENTO
// =====================================================================

// QUE HACE: le pide a la logica verificar los archivos y muestra los problemas y el resumen.
void menuVerificarIntegridad() {
    // 1. Verificacion
    ReporteIntegridad rep;
    Resultado r = verificarIntegridad(&rep);
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }
    // 2. Lista de problemas encontrados
    cout << endl;
    for (int i = 0; i < rep.numProblemas; i++) {
        cout << rep.problemas[i] << endl;
    }
    if (rep.problemasNoMostrados > 0) {
        cout << "... y " << rep.problemasNoMostrados << " problemas mas." << endl;
    }
    // 3. Resumen en un cuadro
    int total = rep.numProblemas + rep.problemasNoMostrados;
    char linea[60];
    encabezado("REPORTE DE INTEGRIDAD DEL SISTEMA");
    snprintf(linea, 60, "Vehiculos verificados : %d", rep.vehiculos);
    opcion(linea);
    snprintf(linea, 60, "Mecanicos verificados : %d", rep.mecanicos);
    opcion(linea);
    snprintf(linea, 60, "Repuestos verificados : %d", rep.repuestos);
    opcion(linea);
    snprintf(linea, 60, "Ordenes verificadas   : %d", rep.ordenes);
    opcion(linea);
    opcion("");
    snprintf(linea, 60, "Problemas encontrados : %d", total);
    opcion(linea);
    opcion(total == 0 ? "Estado: SISTEMA INTEGRO" : "Estado: SE ENCONTRARON PROBLEMAS");
    pie();
}

// QUE HACE: le pide a la logica crear un backup y muestra en que carpeta quedo.
void menuCrearBackup() {
    char ruta[100];
    Resultado r = crearBackup(ruta);
    if (r == OK) {
        cout << "Backup creado en la carpeta: " << ruta << endl;
    } else {
        cout << "ERROR: No se pudo crear el backup (carpeta o archivos de datos/)." << endl;
    }
}

// QUE HACE: submenu de mantenimiento; se repite hasta que el usuario elige 0.
void menuMantenimiento() {
    int op = -1;
    while (op != 0) {
        encabezado("MANTENIMIENTO");
        opcion("1. Verificar integridad");
        opcion("2. Crear backup");
        opcion("0. Volver al menu principal");
        pie();
        op = leerEntero("Seleccione una opcion: ", 0, 2);
        if (op == 1) menuVerificarIntegridad();
        if (op == 2) menuCrearBackup();
    }
}


// =====================================================================
// REPORTES
// La presentacion solo muestra: los calculos los hace la logica.
// =====================================================================

// QUE HACE: Reporte 1. Pide una placa y muestra el vehiculo, todas sus ordenes
// y el total facturado en sus ordenes CERRADAS.
void menuReporteHistorial() {
    // 1. Pido la placa
    char placa[10];
    if (!leerTexto("Placa del vehiculo: ", placa, 10)) return;

    // 2. Le pido el historial a la logica
    Vehiculo v;
    OrdenServicio ordenes[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    float totalCerradas = 0;
    Resultado r = reporteHistorialVehiculo(placa, &v, ordenes, &cantidad, &hayMas, &totalCerradas);
    if (r == NO_EXISTE) {
        cout << "No hay un vehiculo activo con placa '" << placa << "'." << endl;
        return;
    }
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }

    // 3. Datos del vehiculo
    encabezado("HISTORIAL DEL VEHICULO");
    mostrarVehiculo(v);
    cout << endl;
    if (cantidad == 0) {
        cout << "  Este vehiculo no tiene ordenes registradas." << endl;
        return;
    }

    // 4. Tabla de ordenes (el mecanico con su nombre, no solo el ID)
    cout << fixed << setprecision(2);
    cout << "  " << left << setw(8) << "Orden" << setw(12) << "Fecha" << setw(42) << "Mecanico"
         << setw(10) << "Estado" << right << setw(10) << "Total" << endl;
    cout << "  --------------------------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        char mecanico[200];
        describirMecanico(ordenes[i].idMecanico, mecanico);
        mecanico[40] = '\0';               // lo recorto a 40 letras para que no se salga de su columna
        char numero[12];
        snprintf(numero, 12, "#%d", ordenes[i].id);
        cout << "  " << left << setw(8) << numero << setw(12) << ordenes[i].fechaApertura
             << setw(42) << mecanico << setw(10) << ordenes[i].estado
             << right << setw(10) << calcularTotalOrden(ordenes[i]);
        if (strcmp(ordenes[i].estado, "CERRADA") != 0) {
            cout << "  (no suma)";          // ABIERTA y ANULADA no cuentan en el total
        }
        cout << endl;
    }
    if (hayMas) {
        cout << "  (Solo se muestran las primeras " << MAX_RESULTADOS << " ordenes)" << endl;
    }

    // 5. Total facturado
    cout << "  --------------------------------------------------------------------------------" << endl;
    cout << "  Total facturado en ordenes CERRADAS: " << totalCerradas << endl;
    cout << left;
}

// QUE HACE: Reporte 2. Muestra los 10 repuestos con mas unidades usadas en ordenes CERRADAS.
void menuReporteRepuestos() {
    // 1. Le pido el top a la logica
    FilaTopRepuesto top[TOP_REPUESTOS];
    int cantidad = 0;
    Resultado r = reporteRepuestosMasUsados(top, &cantidad);
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }
    // 2. Lo muestro como tabla
    encabezado("REPUESTOS MAS UTILIZADOS");
    if (cantidad == 0) {
        cout << "  Todavia no hay repuestos usados en ordenes CERRADAS." << endl;
        return;
    }
    cout << "  " << left << setw(5) << "Pos" << setw(12) << "Codigo" << setw(32) << "Nombre"
         << right << setw(9) << "Unidades" << endl;
    cout << "  ----------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << "  " << left << setw(5) << (i + 1) << setw(12) << top[i].codigo
             << setw(32) << top[i].nombre << right << setw(9) << top[i].unidades << endl;
    }
    cout << left;
}

// QUE HACE: Reporte 3. Muestra por mecanico (activos y de baja) sus ordenes CERRADAS,
// su mano de obra y sus repuestos, como la tabla del enunciado.
void menuReporteFacturacion() {
    // 1. Le pido las filas a la logica
    FilaFacturacion filas[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    Resultado r = reporteFacturacionMecanicos(filas, &cantidad, &hayMas);
    if (r != OK) {
        cout << "ERROR: " << textoResultado(r) << endl;
        return;
    }
    if (cantidad == 0) {
        cout << "No hay mecanicos registrados." << endl;
        return;
    }
    // 2. Encabezado de la tabla
    const char* borde = "+======================+===========+===========+===========+";
    cout << endl << "  FACTURACION POR MECANICO (solo ordenes CERRADAS)" << endl;
    cout << borde << endl;
    cout << "| Mecanico             |  Ordenes  |  M. obra  | Repuestos |" << endl;
    cout << borde << endl;

    // 3. Una fila por mecanico
    cout << fixed << setprecision(2);
    int totalOrdenes = 0;
    float totalObra = 0;
    float totalRepuestos = 0;
    for (int i = 0; i < cantidad; i++) {
        // nombre recortado para que quepa en la columna; los de baja llevan (BAJA)
        char nombre[30];
        if (filas[i].dadoDeBaja) {
            snprintf(nombre, 30, "%.14s (BAJA)", filas[i].nombre);
        } else {
            snprintf(nombre, 30, "%.20s", filas[i].nombre);
        }
        cout << "| " << left << setw(21) << nombre << "|"
             << right << setw(6) << filas[i].ordenesCerradas << "     |"
             << setw(10) << filas[i].manoObra << " |"
             << setw(10) << filas[i].repuestos << " |" << endl;
        totalOrdenes += filas[i].ordenesCerradas;
        totalObra += filas[i].manoObra;
        totalRepuestos += filas[i].repuestos;
    }
    // 4. Fila de totales
    cout << borde << endl;
    cout << "| " << left << setw(21) << "TOTAL" << "|"
         << right << setw(6) << totalOrdenes << "     |"
         << setw(10) << totalObra << " |"
         << setw(10) << totalRepuestos << " |" << endl;
    cout << borde << endl;
    if (hayMas) {
        cout << "(Solo se muestran los primeros " << MAX_RESULTADOS << " mecanicos)" << endl;
    }
    cout << left;
}

// QUE HACE: submenu de reportes; se repite hasta que el usuario elige 0.
void menuReportes() {
    int op = -1;
    while (op != 0) {
        encabezado("REPORTES");
        opcion("1. Historial de un vehiculo");
        opcion("2. Repuestos mas utilizados");
        opcion("3. Facturacion por mecanico");
        opcion("0. Volver al menu principal");
        pie();
        op = leerEntero("Seleccione una opcion: ", 0, 3);
        if (op == 1) menuReporteHistorial();
        if (op == 2) menuReporteRepuestos();
        if (op == 3) menuReporteFacturacion();
    }
}