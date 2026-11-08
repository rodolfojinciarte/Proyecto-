#pragma once
#include "estructuras.h"
#include "resultados.h"

// =====================================================================
// CAPA DE LOGICA: aplica las reglas del negocio.
// No usa cin/cout y no abre archivos: para leer y escribir usa la persistencia.
// Ninguna funcion devuelve un puntero a un registro: llenan variables
// del que las llama y devuelven un Resultado (si salio bien y, si no, por que).
// =====================================================================

// ===================== VEHICULOS =====================
// Valida y guarda. Placa unica entre los ACTIVOS, ano 1950-2027, km >= 0, cedula valida.
// Puede devolver: OK, DATO_INVALIDO, DUPLICADO, ERROR_ARCHIVO
Resultado registrarVehiculo(Vehiculo datos, int* idAsignado);

// Lee por ID (acceso directo). OK = activo. ELIMINADO = existe pero esta eliminado
// (igual llena v, para que la presentacion pueda decir cual era). NO_EXISTE, ERROR_ARCHIVO.
Resultado obtenerVehiculoPorID(int id, Vehiculo* v);

// Busca la placa exacta entre los activos. OK, NO_EXISTE, ERROR_ARCHIVO
Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo* v);

// Busca por parte del nombre del propietario (sin importar mayusculas). Solo activos.
// Llena resultados[] con maximo MAX_RESULTADOS. hayMas = true si quedaron mas sin entregar.
Resultado buscarVehiculosPorPropietario(const char* texto, Vehiculo resultados[], int* cantidad, bool* hayMas);

// Todos los activos, maximo MAX_RESULTADOS (igual que la busqueda)
Resultado listarVehiculos(Vehiculo resultados[], int* cantidad, bool* hayMas);

// Cambia marca, modelo, ano y datos del propietario. NO cambia placa, km ni ID.
// OK, NO_EXISTE, ELIMINADO, DATO_INVALIDO, ERROR_ARCHIVO
Resultado actualizarVehiculo(int id, Vehiculo datos);

// Borrado logico. Rechazado si el vehiculo tiene ordenes (en cualquier estado).
// OK, NO_EXISTE, ELIMINADO, TIENE_ORDENES, ERROR_ARCHIVO
Resultado eliminarVehiculo(int id);

// En que byte del archivo esta ese vehiculo (para el mensaje de depuracion del Caso de Uso 2)
long posicionEnDiscoVehiculo(int id);

// ===================== MECANICOS =====================
// true si es MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION o GENERAL
bool esEspecialidadValida(const char* especialidad);

// Cedula valida y UNICA (entre todos, incluso los dados de baja: es la misma persona),
// especialidad permitida. Pone la fecha de ingreso de hoy.
// OK, DATO_INVALIDO, DUPLICADO, ERROR_ARCHIVO
Resultado registrarMecanico(Mecanico datos, int* idAsignado);

// OK = activo. ELIMINADO = dado de baja (igual llena m, para mostrar su nombre).
// NO_EXISTE, ERROR_ARCHIVO
Resultado obtenerMecanicoPorID(int id, Mecanico* m);

// Cuantas ordenes ABIERTAS tiene ese mecanico (recorre ordenes.bin)
Resultado contarOrdenesAbiertas(int idMecanico, int* cantidad);

// Todos (activos y dados de baja) con sus ordenes abiertas. ordenesAbiertas[i] va con resultados[i].
Resultado listarMecanicos(Mecanico resultados[], int ordenesAbiertas[], int* cantidad, bool* hayMas);

// Baja logica. Rechazada si tiene ordenes ABIERTAS.
// OK, NO_EXISTE, ELIMINADO (ya estaba de baja), TIENE_ORDENES_ABIERTAS, ERROR_ARCHIVO
Resultado darDeBajaMecanico(int id);

// ===================== REPUESTOS =====================
// Codigo unico entre los ACTIVOS. Precio > 0, stock >= 0, stock minimo >= 0.
// OK, DATO_INVALIDO, DUPLICADO, ERROR_ARCHIVO
Resultado registrarRepuesto(Repuesto datos, int* idAsignado);

// OK = activo. ELIMINADO (igual llena r). NO_EXISTE, ERROR_ARCHIVO
Resultado obtenerRepuestoPorID(int id, Repuesto* r);

// Codigo exacto, solo activos. OK, NO_EXISTE, ERROR_ARCHIVO
Resultado obtenerRepuestoPorCodigo(const char* codigo, Repuesto* r);

// Parte del nombre (sin importar mayusculas), solo activos, maximo MAX_RESULTADOS
Resultado buscarRepuestosPorNombre(const char* texto, Repuesto resultados[], int* cantidad, bool* hayMas);

// Suma unidades al stock. Cantidad > 0.
// OK, DATO_INVALIDO, NO_EXISTE, ELIMINADO, ERROR_ARCHIVO
Resultado registrarEntradaRepuesto(int id, int cantidad);

// Solo activos, maximo MAX_RESULTADOS
Resultado listarRepuestos(Repuesto resultados[], int* cantidad, bool* hayMas);

// true si stock <= stockMinimo (la presentacion lo marca BAJO STOCK)
bool esBajoStock(Repuesto r);

// Nuevo precio > 0. No afecta ordenes ya registradas (guardan su precio historico).
// OK, DATO_INVALIDO, NO_EXISTE, ELIMINADO, ERROR_ARCHIVO
Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio);

// Borrado logico. Rechazado si aparece en alguna linea de cualquier orden.
// OK, NO_EXISTE, ELIMINADO, TIENE_ORDENES, ERROR_ARCHIVO
Resultado eliminarRepuesto(int id);

// ===================== ORDENES =====================
// Abre una orden. Vehiculo y mecanico activos, el vehiculo sin otra ABIERTA,
// el mecanico con menos de 3 ABIERTAS, km de ingreso >= km del vehiculo.
// Escribe: ordenes.bin (orden nueva) y vehiculos.bin (km actualizado).
// OK, DATO_INVALIDO, NO_EXISTE, ELIMINADO, VEHICULO_CON_ORDEN_ABIERTA,
// MECANICO_OCUPADO, KILOMETRAJE_MENOR, ERROR_ARCHIVO
Resultado abrirOrden(int idVehiculo, int idMecanico, int kilometraje, const char* falla, int* idAsignado);

// Las ordenes nunca se eliminan. OK, NO_EXISTE, ERROR_ARCHIVO
Resultado obtenerOrdenPorID(int id, OrdenServicio* o);

// Solo ABIERTAS. Stock suficiente (se descuenta). Precio actual = precio historico.
// Si el repuesto ya esta en la orden, suma a su linea. Maximo MAX_LINEAS lineas.
// OK, DATO_INVALIDO, NO_EXISTE, ELIMINADO, ESTADO_INCORRECTO, STOCK_INSUFICIENTE,
// LIMITE_LINEAS, ERROR_ARCHIVO
Resultado agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad);

// Solo ABIERTAS. Quita la linea completa y devuelve sus unidades al stock.
// OK, NO_EXISTE, ESTADO_INCORRECTO, NO_ESTA_EN_ORDEN, ERROR_ARCHIVO
Resultado quitarRepuestoDeOrden(int idOrden, int idRepuesto);

// Solo ABIERTAS. Mano de obra >= 0. Debe tener una linea o mano de obra > 0.
// OK, DATO_INVALIDO, NO_EXISTE, ESTADO_INCORRECTO, ORDEN_VACIA, ERROR_ARCHIVO
Resultado cerrarOrden(int idOrden, float manoObra);

// Solo ABIERTAS o CERRADAS. Devuelve al stock las unidades de todas sus lineas.
// OK, NO_EXISTE, ESTADO_INCORRECTO, ERROR_ARCHIVO
Resultado anularOrden(int idOrden);

// Mano de obra + suma de (cantidad x precio historico). No se guarda: se calcula.
float calcularTotalOrden(OrdenServicio o);

// Ordenes en ese estado ("ABIERTA", "CERRADA" o "ANULADA"), maximo MAX_RESULTADOS
Resultado listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int* cantidad, bool* hayMas);

// Historial de un vehiculo (todas sus ordenes), maximo MAX_RESULTADOS
Resultado listarOrdenesPorVehiculo(int idVehiculo, OrdenServicio resultados[], int* cantidad, bool* hayMas);