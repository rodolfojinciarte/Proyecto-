#pragma once
#include "estructuras.h"
#include "resultados.h"

// =====================================================================
// CAPA DE PRESENTACION: menus y mensajes. Funciones void.
// Usa cin/cout. NO abre archivos: todo lo pide a la logica.
// =====================================================================

// ===================== AYUDAS =====================
const char* textoResultado(Resultado r);    // mensaje para cada Resultado
void textoFecha(time_t t, char* destino);   // time_t -> "AAAA-MM-DD"
void encabezado(const char* titulo);        // borde de arriba con el titulo
void opcion(const char* texto);             // una linea del menu con bordes
void pie();                                 // borde de abajo
bool menuPrimerArranque();                  // pide los datos del taller la primera vez

// ===================== VEHICULOS =====================
void mostrarVehiculo(Vehiculo v);
void mostrarListaVehiculos(Vehiculo lista[], int cantidad, bool hayMas);
void menuRegistrarVehiculo();
void menuBuscarVehiculo();
void menuActualizarVehiculo();
void menuEliminarVehiculo();
void menuListarVehiculos();
void menuVehiculos();

// ===================== MECANICOS =====================
void mostrarMecanico(Mecanico m, int ordenesAbiertas);
void menuRegistrarMecanico();
void menuBuscarMecanico();
void menuListarMecanicos();
void menuDarDeBajaMecanico();
void menuMecanicos();

// ===================== REPUESTOS =====================
void mostrarRepuesto(Repuesto r);
void mostrarListaRepuestos(Repuesto lista[], int cantidad, bool hayMas);
void menuRegistrarRepuesto();
void menuBuscarRepuesto();
void menuEntradaMercancia();
void menuListarRepuestos();
void menuCambiarPrecio();
void menuEliminarRepuesto();
void menuRepuestos();

// ===================== ORDENES =====================
void mostrarResumenOrden(OrdenServicio o);
void menuAbrirOrden();
void menuAgregarRepuestoOrden();
void menuQuitarRepuestoOrden();
void menuCerrarOrden();
void menuAnularOrden();
void menuVerDetalleOrden();
void menuListarOrdenes();
void menuOrdenes();

// ===================== MANTENIMIENTO =====================
void menuVerificarIntegridad();
void menuCrearBackup();
void menuMantenimiento();
