#include <iostream>
#include <cstring>
#include "logica.h"
#include "persistencia.h"
using namespace std;

// Muestra si salio como se esperaba
void ver(const char* que, Resultado obtenido, Resultado esperado) {
    cout << que << ": " << (obtenido == esperado ? "bien" : "ERROR") << endl;
}

int main() {
    inicializarArchivos();
    int id = 0;

    // ---------- Datos base: 5 vehiculos, 1 mecanico, 17 repuestos ----------
    const char* placas[5] = {"AB123CD", "XY987ZW", "CCC333", "DDD444", "EEE555"};
    for (int i = 0; i < 5; i++) {
        Vehiculo v = {};
        strcpy(v.placa, placas[i]);
        strcpy(v.marca, "Toyota");
        strcpy(v.modelo, "Corolla");
        v.anio = 2015;
        v.kilometraje = 30000;
        strcpy(v.nombrePropietario, "Maria Gonzalez");
        strcpy(v.cedulaPropietario, "12345678");
        registrarVehiculo(v, &id);
    }
    Mecanico m = {};
    strcpy(m.nombre, "Jose Perez");
    strcpy(m.cedula, "11111111");
    strcpy(m.especialidad, "FRENOS");
    registrarMecanico(m, &id);

    for (int i = 1; i <= 17; i++) {          // repuesto 1 = A, repuesto 2 = B, 3 al 17 = relleno
        Repuesto r = {};
        sprintf(r.codigo, "R%02d", i);        // R01, R02, ...
        strcpy(r.nombre, i == 1 ? "Pastillas" : (i == 2 ? "Liquido de frenos" : "Repuesto"));
        strcpy(r.marca, "Bosch");
        r.precio = (i == 1) ? 25.50f : 12.50f;
        r.stock = 10;
        r.stockMinimo = 2;
        registrarRepuesto(r, &id);
    }

    // ---------- Caso de Uso 3 ----------
    cout << "===== ORDEN 1 =====" << endl;
    int idO1 = 0;
    ver("Abrir orden AB123CD km 45000", abrirOrden(1, 1, 45000, "Ruido al frenar", &idO1), OK);
    ver("Segunda orden para AB123CD", abrirOrden(1, 1, 46000, "Otra", &id), VEHICULO_CON_ORDEN_ABIERTA);
    ver("Km menor al registrado", abrirOrden(2, 1, 100, "Prueba", &id), KILOMETRAJE_MENOR);

    agregarRepuestoAOrden(idO1, 1, 2);
    agregarRepuestoAOrden(idO1, 2, 1);
    agregarRepuestoAOrden(idO1, 1, 1);        // ya estaba: debe acumular
    OrdenServicio o;
    obtenerOrdenPorID(idO1, &o);
    cout << "Lineas: " << o.numLineas << " (A con " << o.lineas[0].cantidad << " unidades)" << endl;

    ver("Agregar 100 unidades (no hay)", agregarRepuestoAOrden(idO1, 2, 100), STOCK_INSUFICIENTE);

    Repuesto r;
    obtenerRepuestoPorID(2, &r);
    cout << "Stock de B antes de quitar: " << r.stock << endl;
    quitarRepuestoDeOrden(idO1, 2);
    obtenerRepuestoPorID(2, &r);
    cout << "Stock de B despues de quitar: " << r.stock << endl;

    ver("Cerrar con mano de obra 40", cerrarOrden(idO1, 40.0f), OK);
    obtenerOrdenPorID(idO1, &o);
    cout << "Total: " << calcularTotalOrden(o) << " (esperado 116.5)" << endl;
    ver("Agregar a orden CERRADA", agregarRepuestoAOrden(idO1, 1, 1), ESTADO_INCORRECTO);

    // ---------- Caso de Uso 4 del Proyecto 1: precio historico ----------
    cambiarPrecioRepuesto(1, 99.0f);
    obtenerOrdenPorID(idO1, &o);
    cout << "Total despues de cambiar el precio: " << calcularTotalOrden(o) << " (debe seguir 116.5)" << endl;

    // ---------- Caso de Uso 4: orden con 15 lineas ----------
    cout << endl << "===== ORDEN 2: 15 LINEAS =====" << endl;
    int idO2 = 0;
    abrirOrden(2, 1, 30000, "Mantenimiento completo", &idO2);
    int agregadas = 0;
    for (int rep = 3; rep <= 17; rep++) {
        if (agregarRepuestoAOrden(idO2, rep, 1) == OK) {
            agregadas++;
        }
    }
    cout << "Lineas agregadas: " << agregadas << endl;
    ver("Linea 16", agregarRepuestoAOrden(idO2, 1, 1), LIMITE_LINEAS);

    // ---------- Caso de Uso 5: anulacion ----------
    cout << endl << "===== ANULAR ORDEN 1 =====" << endl;
    obtenerRepuestoPorID(1, &r);
    cout << "Stock de A antes de anular: " << r.stock << endl;
    ver("Anular orden CERRADA", anularOrden(idO1), OK);
    obtenerRepuestoPorID(1, &r);
    cout << "Stock de A despues de anular: " << r.stock << " (debe volver a 10)" << endl;
    ver("Anular otra vez", anularOrden(idO1), ESTADO_INCORRECTO);

    // ---------- Mecanico con maximo 3 abiertas ----------
    cout << endl << "===== MAXIMO 3 ABIERTAS =====" << endl;
    ver("Orden para CCC333 (2da abierta de Jose)", abrirOrden(3, 1, 30000, "Prueba", &id), OK);
    ver("Orden para DDD444 (3ra abierta de Jose)", abrirOrden(4, 1, 30000, "Prueba", &id), OK);
    ver("Orden para EEE555 (seria la 4ta)", abrirOrden(5, 1, 30000, "Prueba", &id), MECANICO_OCUPADO);
    return 0;
}