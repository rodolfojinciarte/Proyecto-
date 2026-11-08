#pragma once
#include <ctime>   // para time_t (fecha y hora guardadas como un numero)

// ===================== CONSTANTES =====================
const int MAX_LINEAS = 15;      // lineas de detalle por orden (el PDF pide minimo 15)
const int MAX_RESULTADOS = 20;  // maximo de resultados que entrega una busqueda

// ===================== HEADER =====================
// Va al inicio de cada archivo .bin (menos taller.bin).
// Es como la portada de un cuaderno: dice cuantas hojas hay y que numero sigue.
struct ArchivoHeader {
    int cantidadRegistros;  // total historico (incluye los eliminados)
    int proximoID;          // siguiente ID a asignar
    int registrosActivos;   // cuantos tienen eliminado == false
    int version;            // version del formato del archivo (empieza en 1)
};

// ===================== ENTIDADES =====================
// Regla del PDF: nada de punteros ni string, solo tamanos fijos,
// para que sizeof sea siempre el mismo y se puedan guardar en disco.

struct Taller {
    char nombre[100];
    char rif[20];
    char direccion[200];
    char telefono[20];
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Vehiculo {
    int id;
    char placa[10];
    char marca[50];
    char modelo[50];
    int anio;
    int kilometraje;
    char nombrePropietario[100];
    char cedulaPropietario[20];
    char telefonoPropietario[20];

    bool eliminado;                  // borrado logico
    time_t fechaCreacion;            // reemplaza a fechaRegistro del Proyecto 1
    time_t fechaUltimaModificacion;
};

struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20];
    char especialidad[20];
    char fechaIngreso[11];

    bool eliminado;                  // true = dado de baja
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Repuesto {
    int id;
    char codigo[20];
    char nombre[100];
    char marca[50];
    float precio;
    int stock;
    int stockMinimo;

    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Una linea de la factura: que repuesto, cuantos y a que precio
struct DetalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario;   // precio historico: foto del precio al agregarlo
};

struct OrdenServicio {
    int id;
    int idVehiculo;
    int idMecanico;
    char fechaApertura[11];
    char fechaCierre[11];
    char estado[12];             // "ABIERTA", "CERRADA" o "ANULADA"
    char descripcionFalla[200];
    int kilometrajeIngreso;
    float manoObra;

    // NUESTRO DISENO DEL DETALLE: arreglo FIJO dentro de la orden.
    // Asi todas las ordenes ocupan los mismos bytes en el archivo
    // y al leer una orden ya vienen sus lineas.
    DetalleOrden lineas[MAX_LINEAS];
    int numLineas;               // cuantas de las 15 casillas estan usadas

    bool eliminado;              // las ordenes no se eliminan: siempre false
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};