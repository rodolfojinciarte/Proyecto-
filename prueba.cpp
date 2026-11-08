#include <iostream>
#include <cstring>
#include "logica.h"
#include "persistencia.h"
using namespace std;

// Para no repetir: arma un repuesto con estos datos
Repuesto armar(const char* codigo, const char* nombre, float precio, int stock, int minimo) {
    Repuesto r = {};
    strcpy(r.codigo, codigo);
    strcpy(r.nombre, nombre);
    strcpy(r.marca, "Bosch");
    r.precio = precio;
    r.stock = stock;
    r.stockMinimo = minimo;
    return r;
}

int main() {
    inicializarArchivos();
    int id = 0;

    cout << "Registro FIL001: " << (registrarRepuesto(armar("FIL001", "Filtro de aceite", 25.50f, 5, 5), &id) == OK ? "OK" : "fallo") << ", ID " << id << endl;
    cout << "Registro BUJ001: " << (registrarRepuesto(armar("BUJ001", "Bujia", 8.00f, 20, 4), &id) == OK ? "OK" : "fallo") << ", ID " << id << endl;
    cout << "Codigo repetido: " << (registrarRepuesto(armar("FIL001", "Otro", 1.0f, 1, 1), &id) == DUPLICADO ? "rechazado (bien)" : "ERROR") << endl;
    cout << "Precio 0: " << (registrarRepuesto(armar("XXX001", "Malo", 0.0f, 1, 1), &id) == DATO_INVALIDO ? "rechazado (bien)" : "ERROR") << endl;

    Repuesto r;
    obtenerRepuestoPorID(1, &r);
    cout << r.nombre << ": stock " << r.stock << (esBajoStock(r) ? " BAJO STOCK" : "") << endl;

    registrarEntradaRepuesto(1, 10);
    obtenerRepuestoPorID(1, &r);
    cout << "Despues de la entrada: stock " << r.stock << (esBajoStock(r) ? " BAJO STOCK" : " (ya no marcado)") << endl;

    cambiarPrecioRepuesto(1, 30.00f);
    obtenerRepuestoPorID(1, &r);
    cout << "Precio nuevo del filtro: " << r.precio << endl;

    Repuesto lista[MAX_RESULTADOS];
    int cant = 0;
    bool hayMas = false;
    buscarRepuestosPorNombre("filt", lista, &cant, &hayMas);
    cout << "Buscar 'filt': " << cant << " resultado(s)" << endl;

    // Orden de prueba que usa la bujia (ID 2)
    OrdenServicio o = {};
    strcpy(o.estado, "CERRADA");
    o.lineas[0].idRepuesto = 2;
    o.lineas[0].cantidad = 4;
    o.lineas[0].precioUnitario = 8.00f;
    o.numLineas = 1;
    int idO = 0;
    guardarOrdenNueva(o, &idO);

    cout << "Eliminar bujia (usada en orden): " << (eliminarRepuesto(2) == TIENE_ORDENES ? "rechazado (bien)" : "ERROR") << endl;
    cout << "Eliminar filtro (no usado): " << (eliminarRepuesto(1) == OK ? "OK" : "fallo") << endl;
    cout << "Buscar filtro despues: " << (obtenerRepuestoPorID(1, &r) == ELIMINADO ? "ELIMINADO (bien)" : "ERROR") << endl;

    cout << "Registrar FIL001 otra vez (el anterior esta eliminado): "
         << (registrarRepuesto(armar("FIL001", "Filtro nuevo", 27.0f, 3, 1), &id) == OK ? "OK, ID " : "fallo ") << id << endl;
    return 0;
}