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