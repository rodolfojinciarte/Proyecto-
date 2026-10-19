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

// ORDENES
OrdenServicio* buscarOrdenPorID(SistemaTaller* s, int id);
OrdenServicio* buscarOrdenAbiertaDeVehiculo(SistemaTaller* s, int idVehiculo);
OrdenServicio* abrirOrden(SistemaTaller* s, int idVehiculo, int idMecanico, int kilometraje, const char* falla);
bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad);

bool quitarRepuestoDeOrden(SistemaTaller* s, int idOrden, int idRepuesto);
bool cerrarOrden(SistemaTaller* s, int idOrden, float manoObra);
bool anularOrden(SistemaTaller* s, int idOrden);
float calcularTotalOrden(OrdenServicio* o);
OrdenServicio** listarOrdenesPorEstado(SistemaTaller* s, const char* estado, int* cantidad);
OrdenServicio** listarOrdenesPorVehiculo(SistemaTaller* s, int idVehiculo, int* cantidad);