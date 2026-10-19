#include <iostream>
#include <iomanip>
#include <cstring>
#include "presentacion.h"
#include "logica.h"
#include "utilidades.h"
using namespace std;

// ===== VEHICULOS =====

// Muestra los datos de un solo vehiculo
void mostrarVehiculo(Vehiculo* v) {
    cout << "  ID " << v->id << " - " << v->placa << " - " << v->marca << " " << v->modelo
         << " " << v->anio << " (" << v->kilometraje << " km)" << endl;
    cout << "  Propietario: " << v->nombrePropietario << " (" << v->cedulaPropietario
         << ") Tel: " << v->telefonoPropietario << endl;
    cout << "  Registrado: " << v->fechaRegistro << endl;
}

// Muestra la tabla de vehiculos
void mostrarListaVehiculos(Vehiculo** vehiculos, int cantidad) {
    cout << left;
    cout << "  " << setw(4) << "ID" << setw(10) << "Placa" << setw(22) << "Vehiculo"
         << setw(6) << "Anio" << setw(10) << "Km" << "Propietario" << endl;
    cout << "  ---------------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        char nombreAuto[110];
        strcpy(nombreAuto, vehiculos[i]->marca);
        strcat(nombreAuto, " ");
        strcat(nombreAuto, vehiculos[i]->modelo);
        cout << "  " << setw(4) << vehiculos[i]->id << setw(10) << vehiculos[i]->placa
             << setw(22) << nombreAuto << setw(6) << vehiculos[i]->anio
             << setw(10) << vehiculos[i]->kilometraje << vehiculos[i]->nombrePropietario << endl;
    }
    cout << "  Total de vehiculos: " << cantidad << endl;
}

// Pide los datos y registra. Si escribe CANCELAR, no se crea nada
void menuRegistrarVehiculo(SistemaTaller* s) {
    Vehiculo v;
    cout << "\n(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Placa: ", v.placa, 10)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Marca: ", v.marca, 50)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Modelo: ", v.modelo, 50)) { cout << "Registro cancelado." << endl; return; }
    v.anio = leerEntero("Anio (1950-2027): ", 1950, 2027);
    v.kilometraje = leerEntero("Kilometraje: ", 0, 2000000);
    if (!leerTexto("Nombre del propietario: ", v.nombrePropietario, 100)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Cedula (V-12345678): ", v.cedulaPropietario, 20)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Telefono: ", v.telefonoPropietario, 20)) { cout << "Registro cancelado." << endl; return; }

    if (!esPlacaValida(v.placa)) {
        cout << "ERROR: La placa '" << v.placa << "' no es valida (6 o 7 letras/numeros)." << endl;
        return;
    }
    if (buscarVehiculoPorPlaca(s, v.placa) != nullptr) {
        cout << "ERROR: La placa '" << v.placa << "' ya esta registrada." << endl;
        return;
    }
    if (!esCedulaValida(v.cedulaPropietario)) {
        cout << "ERROR: La cedula '" << v.cedulaPropietario << "' no es valida." << endl;
        return;
    }
    if (!confirmar("Guardar el vehiculo?")) {
        cout << "Registro cancelado." << endl;
        return;
    }
    Vehiculo* nuevo = agregarVehiculo(s, v);
    if (nuevo == nullptr) {
        cout << "ERROR: No se pudo registrar el vehiculo." << endl;
    } else {
        cout << "Vehiculo registrado:" << endl;
        mostrarVehiculo(nuevo);
    }
}

// Busca por placa exacta o por propietario parcial
void menuBuscarVehiculo(SistemaTaller* s) {
    int opcion = leerEntero("1. Por placa  2. Por propietario: ", 1, 2);
    char texto[100];
    if (!leerTexto("Texto a buscar: ", texto, 100)) { return; }
    if (opcion == 1) {
        Vehiculo* v = buscarVehiculoPorPlaca(s, texto);
        if (v == nullptr) {
            cout << "ERROR: No existe un vehiculo con placa '" << texto << "'." << endl;
        } else {
            mostrarVehiculo(v);
        }
    } else {
        int cantidad;
        Vehiculo** lista = buscarVehiculosPorPropietario(s, texto, &cantidad);
        if (cantidad == 0) {
            cout << "No hay vehiculos de propietarios que contengan '" << texto << "'." << endl;
        } else {
            mostrarListaVehiculos(lista, cantidad);
        }
        delete[] lista;
    }
}

// Cambia marca, modelo, anio y datos del propietario
void menuActualizarVehiculo(SistemaTaller* s) {
    int id = leerEntero("ID del vehiculo: ", 1, 1000000);
    Vehiculo* actual = buscarVehiculoPorID(s, id);
    if (actual == nullptr) {
        cout << "ERROR: No existe el vehiculo con ID " << id << "." << endl;
        return;
    }
    mostrarVehiculo(actual);
    Vehiculo datos;
    if (!leerTexto("Nueva marca: ", datos.marca, 50)) { cout << "Cancelado." << endl; return; }
    if (!leerTexto("Nuevo modelo: ", datos.modelo, 50)) { cout << "Cancelado." << endl; return; }
    datos.anio = leerEntero("Nuevo anio (1950-2027): ", 1950, 2027);
    if (!leerTexto("Nuevo propietario: ", datos.nombrePropietario, 100)) { cout << "Cancelado." << endl; return; }
    if (!leerTexto("Nueva cedula: ", datos.cedulaPropietario, 20)) { cout << "Cancelado." << endl; return; }
    if (!leerTexto("Nuevo telefono: ", datos.telefonoPropietario, 20)) { cout << "Cancelado." << endl; return; }
    if (!confirmar("Guardar los cambios?")) {
        cout << "Cancelado." << endl;
        return;
    }
    if (actualizarVehiculo(s, id, datos)) {
        cout << "Vehiculo actualizado." << endl;
    } else {
        cout << "ERROR: La cedula '" << datos.cedulaPropietario << "' no es valida." << endl;
    }
}

// Elimina si no tiene ordenes
void menuEliminarVehiculo(SistemaTaller* s) {
    int id = leerEntero("ID del vehiculo: ", 1, 1000000);
    Vehiculo* v = buscarVehiculoPorID(s, id);
    if (v == nullptr) {
        cout << "ERROR: No existe el vehiculo con ID " << id << "." << endl;
        return;
    }
    int cantidad;
    OrdenServicio** ordenes = listarOrdenesPorVehiculo(s, id, &cantidad);
    delete[] ordenes;
    if (cantidad > 0) {
        cout << "ERROR: El vehiculo '" << v->placa << "' tiene " << cantidad
             << " ordenes registradas y no puede eliminarse." << endl;
        return;
    }
    char placa[10];
    strcpy(placa, v->placa);
    if (!confirmar("Seguro que desea eliminarlo?")) {
        cout << "Cancelado." << endl;
        return;
    }
    if (eliminarVehiculo(s, id)) {
        cout << "Vehiculo '" << placa << "' eliminado." << endl;
    }
}

// Lista todos los vehiculos
void menuListarVehiculos(SistemaTaller* s) {
    int cantidad;
    Vehiculo** lista = listarVehiculos(s, &cantidad);
    mostrarListaVehiculos(lista, cantidad);
    delete[] lista;
}

// Submenu de vehiculos: se repite hasta que elija 0
void menuVehiculos(SistemaTaller* s) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n+===========================================+" << endl;
        cout << "|           GESTION DE VEHICULOS            |" << endl;
        cout << "+===========================================+" << endl;
        cout << "|  1. Registrar vehiculo                    |" << endl;
        cout << "|  2. Buscar vehiculo                       |" << endl;
        cout << "|  3. Actualizar vehiculo                   |" << endl;
        cout << "|  4. Eliminar vehiculo                     |" << endl;
        cout << "|  5. Listar vehiculos                      |" << endl;
        cout << "|  0. Volver al menu principal              |" << endl;
        cout << "+===========================================+" << endl;
        opcion = leerEntero("Seleccione una opcion: ", 0, 5);

        // Cada numero llama a su funcion de menu
        if (opcion == 1) menuRegistrarVehiculo(s);
        if (opcion == 2) menuBuscarVehiculo(s);
        if (opcion == 3) menuActualizarVehiculo(s);
        if (opcion == 4) menuEliminarVehiculo(s);
        if (opcion == 5) menuListarVehiculos(s);
    }
}

// ===== MECANICOS =====

// Muestra un mecanico con sus ordenes abiertas
void mostrarMecanico(SistemaTaller* s, Mecanico* m) {
    cout << "  ID " << m->id << " - " << m->nombre << " (" << m->especialidad << ")" << endl;
    cout << "  Cedula: " << m->cedula << "  Ingreso: " << m->fechaIngreso << endl;
    // contarOrdenesAbiertas viene de la capa de logica
    cout << "  Ordenes abiertas: " << contarOrdenesAbiertas(s, m->id) << endl;
}

// Pide los datos del mecanico y lo registra
void menuRegistrarMecanico(SistemaTaller* s) {
    Mecanico m;
    cout << "\n(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Nombre: ", m.nombre, 100)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Cedula: ", m.cedula, 20)) { cout << "Registro cancelado." << endl; return; }

    // La especialidad se elige con un numero, asi nunca se escribe mal
    cout << "Especialidad: 1.MOTOR 2.FRENOS 3.ELECTRICIDAD 4.SUSPENSION 5.GENERAL" << endl;
    int esp = leerEntero("Opcion: ", 1, 5);
    const char* nombres[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    strcpy(m.especialidad, nombres[esp - 1]);  // -1 porque el arreglo empieza en 0

    // Validaciones con mensajes que dicen que fallo y con que dato
    if (!esCedulaValida(m.cedula)) {
        cout << "ERROR: La cedula '" << m.cedula << "' no es valida." << endl;
        return;
    }
    if (buscarMecanicoPorCedula(s, m.cedula) != nullptr) {
        cout << "ERROR: La cedula '" << m.cedula << "' ya esta registrada." << endl;
        return;
    }
    if (!confirmar("Guardar el mecanico?")) {
        cout << "Registro cancelado." << endl;
        return;
    }
    Mecanico* nuevo = agregarMecanico(s, m);
    if (nuevo == nullptr) {
        cout << "ERROR: No se pudo registrar el mecanico." << endl;
    } else {
        cout << "Mecanico registrado:" << endl;
        mostrarMecanico(s, nuevo);
    }
}

// Busca un mecanico por su ID
void menuBuscarMecanico(SistemaTaller* s) {
    int id = leerEntero("ID del mecanico: ", 1, 1000000);
    Mecanico* m = buscarMecanicoPorID(s, id);
    if (m == nullptr) {
        cout << "ERROR: No existe el mecanico con ID " << id << "." << endl;
    } else {
        mostrarMecanico(s, m);
    }
}

// Lista todos los mecanicos con sus ordenes ABIERTAS (lo pide el PDF)
void menuListarMecanicos(SistemaTaller* s) {
    int cantidad;
    Mecanico** lista = listarMecanicos(s, &cantidad);
    cout << left;
    cout << "  " << setw(4) << "ID" << setw(25) << "Nombre" << setw(14) << "Especialidad"
         << "Abiertas" << endl;
    cout << "  ---------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << "  " << setw(4) << lista[i]->id << setw(25) << lista[i]->nombre
             << setw(14) << lista[i]->especialidad
             << contarOrdenesAbiertas(s, lista[i]->id) << endl;
    }
    cout << "  Total de mecanicos: " << cantidad << endl;
    delete[] lista;  // libero solo la libreta, no los mecanicos
}

// Submenu de mecanicos: se repite hasta que elija 0
void menuMecanicos(SistemaTaller* s) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n+===========================================+" << endl;
        cout << "|           GESTION DE MECANICOS            |" << endl;
        cout << "+===========================================+" << endl;
        cout << "|  1. Registrar mecanico                    |" << endl;
        cout << "|  2. Buscar mecanico                       |" << endl;
        cout << "|  3. Listar mecanicos                      |" << endl;
        cout << "|  0. Volver al menu principal              |" << endl;
        cout << "+===========================================+" << endl;
        opcion = leerEntero("Seleccione una opcion: ", 0, 3);
        if (opcion == 1) menuRegistrarMecanico(s);
        if (opcion == 2) menuBuscarMecanico(s);
        if (opcion == 3) menuListarMecanicos(s);
    }
}

// ===== REPUESTOS =====

// Muestra un solo repuesto
void mostrarRepuesto(Repuesto* r) {
    cout << fixed << setprecision(2);  // precios con 2 decimales
    cout << "  ID " << r->id << " - " << r->codigo << " - " << r->nombre
         << " (" << r->marca << ")" << endl;
    cout << "  Precio: " << r->precio << "  Stock: " << r->stock
         << "  Minimo: " << r->stockMinimo;
    if (esBajoStock(r)) {
        cout << "  BAJO STOCK";
    }
    cout << endl;
}

// Tabla de repuestos; marca BAJO STOCK si stock <= minimo
void mostrarListaRepuestos(Repuesto** lista, int cantidad) {
    cout << left << fixed << setprecision(2);
    cout << "  " << setw(4) << "ID" << setw(10) << "Codigo" << setw(28) << "Nombre"
         << setw(10) << "Precio" << setw(7) << "Stock" << "Alerta" << endl;
    cout << "  -----------------------------------------------------------------" << endl;
    for (int i = 0; i < cantidad; i++) {
        cout << "  " << setw(4) << lista[i]->id << setw(10) << lista[i]->codigo
             << setw(28) << lista[i]->nombre << setw(10) << lista[i]->precio
             << setw(7) << lista[i]->stock;
        if (esBajoStock(lista[i])) {
            cout << "BAJO STOCK";
        }
        cout << endl;
    }
    cout << "  Total de repuestos: " << cantidad << endl;
}

// Pide los datos del repuesto y lo registra
void menuRegistrarRepuesto(SistemaTaller* s) {
    Repuesto r;
    cout << "\n(Escriba CANCELAR en cualquier texto para salir)" << endl;
    if (!leerTexto("Codigo: ", r.codigo, 20)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Nombre: ", r.nombre, 100)) { cout << "Registro cancelado." << endl; return; }
    if (!leerTexto("Marca: ", r.marca, 50)) { cout << "Registro cancelado." << endl; return; }

    // El precio se pide en centavos para usar leerEntero (2550 = 25.50)
    int centavos = leerEntero("Precio en centavos (ej: 2550 = 25.50): ", 1, 100000000);
    r.precio = centavos / 100.0f;
    r.stock = leerEntero("Stock inicial: ", 0, 1000000);
    r.stockMinimo = leerEntero("Stock minimo: ", 0, 1000000);

    if (buscarRepuestoPorCodigo(s, r.codigo) != nullptr) {
        cout << "ERROR: El codigo '" << r.codigo << "' ya esta registrado." << endl;
        return;
    }
    if (!confirmar("Guardar el repuesto?")) {
        cout << "Registro cancelado." << endl;
        return;
    }
    Repuesto* nuevo = agregarRepuesto(s, r);
    if (nuevo == nullptr) {
        cout << "ERROR: No se pudo registrar el repuesto." << endl;
    } else {
        cout << "Repuesto registrado:" << endl;
        mostrarRepuesto(nuevo);
    }
}

// Busca por ID o por codigo/nombre parcial
void menuBuscarRepuesto(SistemaTaller* s) {
    int opcion = leerEntero("1. Por ID  2. Por codigo o nombre: ", 1, 2);
    if (opcion == 1) {
        int id = leerEntero("ID: ", 1, 1000000);
        Repuesto* r = buscarRepuestoPorID(s, id);
        if (r == nullptr) {
            cout << "ERROR: No existe el repuesto con ID " << id << "." << endl;
        } else {
            mostrarRepuesto(r);
        }
    } else {
        char texto[100];
        if (!leerTexto("Texto a buscar: ", texto, 100)) { return; }
        int cantidad;
        Repuesto** lista = buscarRepuestosPorTexto(s, texto, &cantidad);
        if (cantidad == 0) {
            cout << "No hay repuestos que contengan '" << texto << "'." << endl;
        } else {
            mostrarListaRepuestos(lista, cantidad);
        }
        delete[] lista;  // libero solo la libreta
    }
}

// Suma unidades al stock
void menuEntradaMercancia(SistemaTaller* s) {
    int id = leerEntero("ID del repuesto: ", 1, 1000000);
    Repuesto* r = buscarRepuestoPorID(s, id);
    if (r == nullptr) {
        cout << "ERROR: No existe el repuesto con ID " << id << "." << endl;
        return;
    }
    int cantidad = leerEntero("Unidades que llegaron: ", 1, 1000000);
    if (registrarEntrada(s, id, cantidad)) {
        cout << "Entrada registrada. Stock de '" << r->nombre << "': " << r->stock << endl;
    }
}

// Lista todos los repuestos
void menuListarRepuestos(SistemaTaller* s) {
    int cantidad;
    Repuesto** lista = listarRepuestos(s, &cantidad);
    mostrarListaRepuestos(lista, cantidad);
    delete[] lista;
}

// Cambia el precio (las ordenes viejas no cambian, guardan su precio)
void menuCambiarPrecio(SistemaTaller* s) {
    int id = leerEntero("ID del repuesto: ", 1, 1000000);
    Repuesto* r = buscarRepuestoPorID(s, id);
    if (r == nullptr) {
        cout << "ERROR: No existe el repuesto con ID " << id << "." << endl;
        return;
    }
    mostrarRepuesto(r);
    int centavos = leerEntero("Nuevo precio en centavos: ", 1, 100000000);
    if (!confirmar("Guardar el nuevo precio?")) {
        cout << "Cancelado." << endl;
        return;
    }
    if (cambiarPrecio(s, id, centavos / 100.0f)) {
        cout << "Precio actualizado." << endl;
        mostrarRepuesto(r);
    }
}

// Elimina solo si no se ha usado en ninguna orden
void menuEliminarRepuesto(SistemaTaller* s) {
    int id = leerEntero("ID del repuesto: ", 1, 1000000);
    Repuesto* r = buscarRepuestoPorID(s, id);
    if (r == nullptr) {
        cout << "ERROR: No existe el repuesto con ID " << id << "." << endl;
        return;
    }
    char nombre[100];
    strcpy(nombre, r->nombre);  // lo guardo antes de borrarlo, para el mensaje
    if (!confirmar("Seguro que desea eliminarlo?")) {
        cout << "Cancelado." << endl;
        return;
    }
    // Si existe y aun asi falla, es porque esta usado en alguna orden
    if (eliminarRepuesto(s, id)) {
        cout << "Repuesto '" << nombre << "' eliminado." << endl;
    } else {
        cout << "ERROR: El repuesto '" << nombre << "' esta usado en una orden y no puede eliminarse." << endl;
    }
}

// Submenu de repuestos: se repite hasta que elija 0
void menuRepuestos(SistemaTaller* s) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n+===========================================+" << endl;
        cout << "|         INVENTARIO DE REPUESTOS           |" << endl;
        cout << "+===========================================+" << endl;
        cout << "|  1. Registrar repuesto                    |" << endl;
        cout << "|  2. Buscar repuesto                       |" << endl;
        cout << "|  3. Entrada de mercancia                  |" << endl;
        cout << "|  4. Listar repuestos                      |" << endl;
        cout << "|  5. Cambiar precio                        |" << endl;
        cout << "|  6. Eliminar repuesto                     |" << endl;
        cout << "|  0. Volver al menu principal              |" << endl;
        cout << "+===========================================+" << endl;
        opcion = leerEntero("Seleccione una opcion: ", 0, 6);
        if (opcion == 1) menuRegistrarRepuesto(s);
        if (opcion == 2) menuBuscarRepuesto(s);
        if (opcion == 3) menuEntradaMercancia(s);
        if (opcion == 4) menuListarRepuestos(s);
        if (opcion == 5) menuCambiarPrecio(s);
        if (opcion == 6) menuEliminarRepuesto(s);
    }
}

// ===================== ORDENES =====================
#include <iomanip>   // para mostrar los precios con 2 decimales

// ABRIR ORDEN: pide vehiculo, mecanico, km y falla
void abrirOrdenMenu(SistemaTaller* s) {
    int idVeh = leerEntero("ID del vehiculo: ", 1, 999999);
    int idMec = leerEntero("ID del mecanico: ", 1, 999999);
    int km = leerEntero("Kilometraje de ingreso: ", 0, 9999999);
    char falla[200];
    if (!leerTexto("Descripcion de la falla: ", falla, 200)) return; // si escribio CANCELAR, se sale

    // la logica valida todo y devuelve nullptr si algo esta mal
    OrdenServicio* o = abrirOrden(s, idVeh, idMec, km, falla);
    if (o == nullptr) {
        cout << "No se pudo abrir la orden (vehiculo o mecanico no existe, o ya tiene una orden abierta)." << endl;
    } else {
        cout << "Orden abierta con ID " << o->id << endl;
    }
}

// AGREGAR REPUESTO: mete piezas a una orden abierta
void agregarRepuestoMenu(SistemaTaller* s) {
    int idOrden = leerEntero("ID de la orden: ", 1, 999999);
    int idRep = leerEntero("ID del repuesto: ", 1, 999999);
    int cant = leerEntero("Cantidad: ", 1, 999999);
    if (agregarRepuestoAOrden(s, idOrden, idRep, cant)) {
        cout << "Repuesto agregado a la orden." << endl;
    } else {
        cout << "No se pudo (orden no abierta, repuesto no existe o no hay stock)." << endl;
    }
}

// QUITAR REPUESTO: saca una pieza y devuelve el stock
void quitarRepuestoMenu(SistemaTaller* s) {
    int idOrden = leerEntero("ID de la orden: ", 1, 999999);
    int idRep = leerEntero("ID del repuesto a quitar: ", 1, 999999);
    if (quitarRepuestoDeOrden(s, idOrden, idRep)) {
        cout << "Repuesto quitado." << endl;
    } else {
        cout << "No se pudo quitar el repuesto." << endl;
    }
}

// CERRAR ORDEN: cobra la mano de obra (en centavos, igual que el precio)
void cerrarOrdenMenu(SistemaTaller* s) {
    int idOrden = leerEntero("ID de la orden: ", 1, 999999);
    int centavos = leerEntero("Mano de obra en centavos (ej: 5000 = 50.00): ", 0, 99999999);
    float manoObra = centavos / 100.0f;   // convierto centavos a dinero
    if (!confirmar("Seguro que desea cerrar la orden?")) return;
    if (cerrarOrden(s, idOrden, manoObra)) {
        cout << "Orden cerrada." << endl;
    } else {
        cout << "No se pudo cerrar (no existe o no esta abierta)." << endl;
    }
}

// ANULAR ORDEN: la cancela y devuelve los repuestos al inventario
void anularOrdenMenu(SistemaTaller* s) {
    int idOrden = leerEntero("ID de la orden: ", 1, 999999);
    if (!confirmar("Seguro que desea anular la orden?")) return;
    if (anularOrden(s, idOrden)) {
        cout << "Orden anulada. Stock devuelto." << endl;
    } else {
        cout << "No se pudo anular." << endl;
    }
}

// VER DETALLE: muestra la orden como una factura
void verDetalleOrden(SistemaTaller* s) {
    int idOrden = leerEntero("ID de la orden: ", 1, 999999);
    OrdenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr) {
        cout << "Esa orden no existe." << endl;
        return;
    }
    cout << fixed << setprecision(2);   // siempre 2 decimales
    cout << "+==============================================+" << endl;
    cout << "  " << s->taller.nombre << "  RIF: " << s->taller.rif << endl;
    cout << "  ORDEN #" << o->id << "   Estado: " << o->estado << endl;
    cout << "  Apertura: " << o->fechaApertura << "   Cierre: " << o->fechaCierre << endl;
    cout << "  Vehiculo ID: " << o->idVehiculo << "   Mecanico ID: " << o->idMecanico << endl;
    cout << "  Km ingreso: " << o->kilometrajeIngreso << endl;
    cout << "  Falla: " << o->descripcionFalla << endl;
    cout << "+----------------------------------------------+" << endl;
    cout << "  Repuesto              Cant   P.Unit   Subtotal" << endl;

    // recorro los detalles (las piezas usadas)
    for (int i = 0; i < o->numDetalles; i++) {
        DetalleOrden d = o->detalles[i];
        Repuesto* r = buscarRepuestoPorID(s, d.idRepuesto);
        cout << "  " << setw(20) << left << (r != nullptr ? r->nombre : "(eliminado)")
             << right << setw(6) << d.cantidad
             << setw(9) << d.precioUnitario
             << setw(11) << d.cantidad * d.precioUnitario << endl;
    }
    cout << "+----------------------------------------------+" << endl;
    cout << "  Mano de obra: " << o->manoObra << endl;
    cout << "  TOTAL: " << calcularTotalOrden(o) << endl;
    cout << "+==============================================+" << endl;
}

// LISTAR ORDENES: por estado
void listarOrdenesMenu(SistemaTaller* s) {
    cout << "1. ABIERTA  2. CERRADA  3. ANULADA" << endl;
    int op = leerEntero("Estado: ", 1, 3);
    const char* estado = "ABIERTA";
    if (op == 2) estado = "CERRADA";
    if (op == 3) estado = "ANULADA";

    int cantidad = 0;
    OrdenServicio** lista = listarOrdenesPorEstado(s, estado, &cantidad); // libreta de direcciones
    if (cantidad == 0) {
        cout << "No hay ordenes en ese estado." << endl;
    }
    for (int i = 0; i < cantidad; i++) {
        cout << "Orden #" << lista[i]->id << " | Vehiculo " << lista[i]->idVehiculo
             << " | Mecanico " << lista[i]->idMecanico
             << " | " << lista[i]->fechaApertura << endl;
    }
    delete[] lista;   // borro SOLO la libreta, no las ordenes
}

// SUBMENU DE ORDENES
void menuOrdenes(SistemaTaller* s) {
    int op = -1;
    while (op != 0) {
        cout << endl;
        cout << "+==============================+" << endl;
        cout << "|      ORDENES DE SERVICIO     |" << endl;
        cout << "+==============================+" << endl;
        cout << "| 1. Abrir orden               |" << endl;
        cout << "| 2. Agregar repuesto          |" << endl;
        cout << "| 3. Quitar repuesto           |" << endl;
        cout << "| 4. Cerrar orden              |" << endl;
        cout << "| 5. Anular orden              |" << endl;
        cout << "| 6. Ver detalle               |" << endl;
        cout << "| 7. Listar ordenes            |" << endl;
        cout << "| 0. Volver                    |" << endl;
        cout << "+==============================+" << endl;
        op = leerEntero("Seleccione una opcion: ", 0, 7);
        if (op == 1) abrirOrdenMenu(s);
        if (op == 2) agregarRepuestoMenu(s);
        if (op == 3) quitarRepuestoMenu(s);
        if (op == 4) cerrarOrdenMenu(s);
        if (op == 5) anularOrdenMenu(s);
        if (op == 6) verDetalleOrden(s);
        if (op == 7) listarOrdenesMenu(s);
    }
}