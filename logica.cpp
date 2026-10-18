#include "logica.h"

void inicializarSistema( SistemaTaller* s, Taller datosTaller){

       // aqui guardas los datos del taller
    s->taller = datosTaller;

    // aqui preparas la lista de vehiculos (3 lineas)
    s->capacidadVehiculos = 4; // en la caja caben 4
    s->numVehiculos = 0; // en la caja hay 0
    s->vehiculos = new Vehiculo[s->capacidadVehiculos];  // compro la caja

    s->capacidadMecanicos = 4;
    s->numMecanicos = 0;
    s->mecanicos = new Mecanico[s->capacidadMecanicos];

    s->capacidadRepuestos = 4;
    s->numRepuestos = 0;
    s->repuestos = new Repuesto[s->capacidadRepuestos];

    s->capacidadOrdenes = 4;
    s->numOrdenes = 0;
    s->ordenes = new OrdenServicio [s->capacidadOrdenes];

    s->siguienteIdVehiculo = 1;
    s->siguienteIdMecanico = 1;
    s->siguienteIdRepuesto = 1;
    s->siguienteIdOrden = 1;

    //Porque el primer vehículo que se registre será el #1, el primer mecánico el #1, etc. Cada vez que se registre uno, este número sube.
}

void  liberarSistema(SistemaTaller* s) {
    // Primero los detalles: su direccion esta guardada dentro de las ordenes.
    // Si borro primero las ordenes, pierdo esas direcciones y se crea una fuga de memoria.
    //si i = 0 y es menor q la cantidad de ordenes, en cada vuelta suma 1 a i
    for(int i= 0; i < s->numOrdenes; i++){
        delete[] s->ordenes[i].detalles;
    }

    delete[] s->vehiculos;
    delete[] s-> mecanicos;
    delete[] s-> repuestos;
    delete[] s->ordenes;

    s->vehiculos = nullptr;
    s->mecanicos = nullptr;
    s->repuestos = nullptr;
    s->ordenes = nullptr;
    //elimina direcciones
}