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

// MECANICOS
void mostrarMecanico(SistemaTaller* s, Mecanico* m);
void menuRegistrarMecanico(SistemaTaller* s);
void menuBuscarMecanico(SistemaTaller* s);
void menuListarMecanicos(SistemaTaller* s);
void menuMecanicos(SistemaTaller* s);

// REPUESTOS
void mostrarRepuesto(Repuesto* r);
void mostrarListaRepuestos(Repuesto** lista, int cantidad);
void menuRegistrarRepuesto(SistemaTaller* s);
void menuBuscarRepuesto(SistemaTaller* s);
void menuEntradaMercancia(SistemaTaller* s);
void menuListarRepuestos(SistemaTaller* s);
void menuCambiarPrecio(SistemaTaller* s);
void menuEliminarRepuesto(SistemaTaller* s);
void menuRepuestos(SistemaTaller* s);

// ORDENES (menus)
void abrirOrdenMenu(SistemaTaller* s);        // abre una orden nueva
void agregarRepuestoMenu(SistemaTaller* s);   // mete un repuesto a la orden
void quitarRepuestoMenu(SistemaTaller* s);    // saca un repuesto de la orden
void cerrarOrdenMenu(SistemaTaller* s);       // cobra mano de obra y cierra
void anularOrdenMenu(SistemaTaller* s);       // anula y devuelve el stock
void verDetalleOrden(SistemaTaller* s);       // muestra la factura
void listarOrdenesMenu(SistemaTaller* s);     // lista por estado
void menuOrdenes(SistemaTaller* s);           // el submenu