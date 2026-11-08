#include <iostream>
#include <cstring>
#include "persistencia.h"
using namespace std;

int main() {
    // Tamanos (van en el README)
    cout << "sizeof(ArchivoHeader) = " << sizeof(ArchivoHeader) << endl;
    cout << "sizeof(Taller)        = " << sizeof(Taller) << endl;
    cout << "sizeof(Vehiculo)      = " << sizeof(Vehiculo) << endl;
    cout << "sizeof(Mecanico)      = " << sizeof(Mecanico) << endl;
    cout << "sizeof(Repuesto)      = " << sizeof(Repuesto) << endl;
    cout << "sizeof(DetalleOrden)  = " << sizeof(DetalleOrden) << endl;
    cout << "sizeof(OrdenServicio) = " << sizeof(OrdenServicio) << endl;

    if (!inicializarArchivos()) {
        cout << "No se pudieron crear los archivos en datos/" << endl;
        return 1;
    }

    // Un mecanico
    Mecanico m = {};
    strcpy(m.nombre, "Jose Perez");
    strcpy(m.especialidad, "FRENOS");
    int idM = 0;
    guardarMecanicoNuevo(m, &idM);

    // Una orden con una linea
    OrdenServicio o = {};
    strcpy(o.estado, "ABIERTA");
    o.lineas[0].idRepuesto = 1;
    o.lineas[0].cantidad = 2;
    o.lineas[0].precioUnitario = 25.50f;
    o.numLineas = 1;
    int idO = 0;
    guardarOrdenNueva(o, &idO);

    // Leo la orden de vuelta: sus lineas vienen con ella
    OrdenServicio leida;
    if (leerOrdenPorID(idO, &leida) == OK) {
        cout << "Orden #" << leida.id << " (" << leida.estado << ") con "
             << leida.numLineas << " linea(s). Primera: "
             << leida.lineas[0].cantidad << " x " << leida.lineas[0].precioUnitario << endl;
    }

    // Pruebo el borrado logico del vehiculo 1
    marcarVehiculoEliminado(1);
    Vehiculo v;
    if (leerVehiculoPorID(1, &v) == OK) {
        cout << "Vehiculo 1: " << v.placa << (v.eliminado ? " (ELIMINADO)" : " (activo)") << endl;
    }
    if (leerVehiculoPorID(99, &v) == NO_EXISTE) {
        cout << "Vehiculo 99: no existe" << endl;
    }
    int activos = 0;
    contarVehiculosActivos(&activos);
    cout << "Vehiculos activos: " << activos << endl;
    return 0;
}