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