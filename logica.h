#pragma once
#include "estructuras.h"

// ===== Arrancar y apagar el sistema =====
void inicializarSistema(SistemaTaller* s, Taller datosTaller);
void liberarSistema(SistemaTaller* s);

// ===== Mudanzas: duplicar la capacidad cuando se llena =====
void redimensionarVehiculos(SistemaTaller* s);
void redimensionarMecanicos(SistemaTaller* s);
void redimensionarRepuestos(SistemaTaller* s);
void redimensionarOrdenes(SistemaTaller* s);
void redimensionarDetalles(OrdenServicio* orden);
Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos);
Vehiculo* buscarVehiculoPorID(SistemaTaller* s, int id);
Vehiculo* buscarVehiculoPorPlaca(SistemaTaller* s, const char* placa);
Vehiculo** buscarVehiculosPorPropietario(SistemaTaller* s, const char* subcadena, int* cantidad);
Vehiculo** listarVehiculos(SistemaTaller* s, int* cantidad);
bool actualizarVehiculo(SistemaTaller* s, int id, Vehiculo datos);
bool eliminarVehiculo(SistemaTaller* s, int id);

// MECANICOS
bool esEspecialidadValida(const char* especialidad);
Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos);
Mecanico* buscarMecanicoPorID(SistemaTaller* s, int id);
Mecanico* buscarMecanicoPorCedula(SistemaTaller* s, const char* cedula);
int contarOrdenesAbiertas(SistemaTaller* s, int idMecanico);
Mecanico** listarMecanicos(SistemaTaller* s, int* cantidad);