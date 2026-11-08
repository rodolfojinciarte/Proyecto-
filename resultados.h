#pragma once

// Respuestas posibles de las funciones de persistencia y de logica.
// El PDF exige que la presentacion sepa SI fallo y POR QUE.
// Un enum es una lista de nombres; por dentro cada uno es un numero
// (OK = 0, ERROR_ARCHIVO = 1, NO_EXISTE = 2, ...).
enum Resultado {
    OK,                          // todo salio bien
    ERROR_ARCHIVO,               // no se pudo abrir, leer o escribir un archivo
    NO_EXISTE,                   // ese ID no esta en el archivo
    ELIMINADO,                   // existe, pero esta eliminado (o dado de baja)
    DATO_INVALIDO,               // un dato no cumple las reglas (ano, precio...)
    DUPLICADO,                   // placa, cedula o codigo repetido
    TIENE_ORDENES,               // no se puede eliminar: tiene ordenes
    TIENE_ORDENES_ABIERTAS,      // no se puede dar de baja: tiene ordenes abiertas
    ESTADO_INCORRECTO,           // la orden no esta en el estado necesario
    STOCK_INSUFICIENTE,          // no hay suficientes unidades
    LIMITE_LINEAS,               // la orden ya tiene sus 15 lineas
    MECANICO_OCUPADO,            // el mecanico ya tiene 3 ordenes abiertas
    VEHICULO_CON_ORDEN_ABIERTA,  // el vehiculo ya tiene una orden abierta
    KILOMETRAJE_MENOR,           // km de ingreso menor que el del vehiculo
    ORDEN_VACIA,                 // no se puede cerrar sin lineas ni mano de obra
    NO_ESTA_EN_ORDEN             // ese repuesto no esta en la orden
};