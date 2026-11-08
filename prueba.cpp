#include <iostream>
#include <cstring>
#include "logica.h"
#include "persistencia.h"   // solo para crear archivos y una orden de prueba
using namespace std;

int main() {
    inicializarArchivos();

    // ================= VEHICULOS (Caso de Uso 2) =================
    cout << "===== VEHICULOS =====" << endl;
    const char* placas[6] = {"AAA111", "BBB222", "CCC333", "DDD444", "EEE555", "FFF666"};
    for (int i = 0; i < 6; i++) {
        Vehiculo v = {};
        strcpy(v.placa, placas[i]);
        strcpy(v.marca, "Toyota");
        strcpy(v.modelo, "Corolla");
        v.anio = 2015;
        v.kilometraje = 40000;
        strcpy(v.nombrePropietario, "Maria Gonzalez");
        strcpy(v.cedulaPropietario, "12345678");
        strcpy(v.telefonoPropietario, "04141234567");
        int id = 0;
        Resultado r = registrarVehiculo(v, &id);
        cout << "Registro " << placas[i] << ": " << (r == OK ? "OK, ID " : "fallo ") << id << endl;
    }

    Vehiculo rep = {};
    strcpy(rep.placa, "AAA111");
    strcpy(rep.marca, "Ford");
    strcpy(rep.modelo, "Fiesta");
    rep.anio = 2010;
    strcpy(rep.nombrePropietario, "Pedro Rojas");
    strcpy(rep.cedulaPropietario, "87654321");
    int idRep = 0;
    cout << "Placa repetida AAA111: "
         << (registrarVehiculo(rep, &idRep) == DUPLICADO ? "rechazada (bien)" : "ERROR") << endl;

    cout << "Eliminar ID 3: " << (eliminarVehiculo(3) == OK ? "OK" : "fallo") << endl;

    Vehiculo v;
    if (obtenerVehiculoPorID(5, &v) == OK) {
        cout << "[DEPURACION] Vehiculo ID 5 leido en el byte "
             << posicionEnDiscoVehiculo(5) << ": " << v.placa << endl;
    }
    if (obtenerVehiculoPorID(3, &v) == ELIMINADO) {
        cout << "Vehiculo ID 3 (" << v.placa << "): fue ELIMINADO" << endl;
    }

    Vehiculo nuevo = rep;
    strcpy(nuevo.placa, "CCC333");
    int idNuevo = 0;
    cout << "Re-registrar CCC333: "
         << (registrarVehiculo(nuevo, &idNuevo) == OK ? "OK, ID " : "fallo ") << idNuevo << endl;

    Vehiculo lista[MAX_RESULTADOS];
    int cant = 0;
    bool hayMas = false;
    buscarVehiculosPorPropietario("gonz", lista, &cant, &hayMas);
    cout << "Buscar 'gonz': " << cant << " vehiculo(s) activos" << endl;

    // ================= MECANICOS (Caso de Uso 6) =================
    cout << endl << "===== MECANICOS =====" << endl;
    Mecanico m = {};
    strcpy(m.nombre, "Jose Perez");
    strcpy(m.cedula, "11111111");
    strcpy(m.especialidad, "FRENOS");
    int idM = 0;
    cout << "Registro Jose Perez: " << (registrarMecanico(m, &idM) == OK ? "OK, ID " : "fallo ") << idM << endl;

    cout << "Cedula repetida: " << (registrarMecanico(m, &idM) == DUPLICADO ? "rechazada (bien)" : "ERROR") << endl;

    Mecanico malo = m;
    strcpy(malo.cedula, "22222222");
    strcpy(malo.especialidad, "PINTURA");
    cout << "Especialidad PINTURA: "
         << (registrarMecanico(malo, &idM) == DATO_INVALIDO ? "rechazada (bien)" : "ERROR") << endl;

    // Orden ABIERTA de prueba para el mecanico 1 y el vehiculo 1 (directo con la persistencia)
    OrdenServicio o = {};
    o.idVehiculo = 1;
    o.idMecanico = 1;
    strcpy(o.estado, "ABIERTA");
    int idO = 0;
    guardarOrdenNueva(o, &idO);

    int abiertas = 0;
    contarOrdenesAbiertas(1, &abiertas);
    cout << "Ordenes abiertas de Jose: " << abiertas << endl;

    cout << "Dar de baja con orden ABIERTA: "
         << (darDeBajaMecanico(1) == TIENE_ORDENES_ABIERTAS ? "rechazado (bien)" : "ERROR") << endl;

    cout << "Eliminar vehiculo 1 (tiene orden): "
         << (eliminarVehiculo(1) == TIENE_ORDENES ? "rechazado (bien)" : "ERROR") << endl;

    // Cierro la orden y ahora si se puede dar de baja
    leerOrdenPorID(idO, &o);
    strcpy(o.estado, "CERRADA");
    sobrescribirOrden(o);
    cout << "Dar de baja con la orden CERRADA: " << (darDeBajaMecanico(1) == OK ? "OK" : "fallo") << endl;

    if (obtenerMecanicoPorID(1, &m) == ELIMINADO) {
        cout << "Mecanico 1: " << m.nombre << " (DADO DE BAJA)" << endl;
    }

    Mecanico mecs[MAX_RESULTADOS];
    int abiertasDe[MAX_RESULTADOS];
    listarMecanicos(mecs, abiertasDe, &cant, &hayMas);
    cout << "Listado de mecanicos: " << cant << " (incluye los de baja)" << endl;
    return 0;
}