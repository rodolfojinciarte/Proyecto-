#pragma once
#include "estructuras.h"

// VEHICULOS
void mostrarVehiculo(Vehiculo* v);
void mostrarListaVehiculos(Vehiculo** vehiculos, int cantidad);
void menuRegistrarVehiculo(SistemaTaller* s);
void menuBuscarVehiculo(SistemaTaller* s);
void menuActualizarVehiculo(SistemaTaller* s);
void menuEliminarVehiculo(SistemaTaller* s);
void menuListarVehiculos(SistemaTaller* s);
void menuVehiculos(SistemaTaller* s);