#include "logica.h"
#include "utilidades.h"

void inicializarSistema( SistemaTaller* s, Taller datosTaller){

       // aqui guardas los datos del taller
    s->taller = datosTaller;

    // aqui preparas la lista de vehiculos (3 lineas)
    s->capacidadVehiculos = 4; // en la caja caben 4
    s->numVehiculos = 0; // en la caja hay 0
    s->vehiculos = new Vehiculo[s->capacidadVehiculos];  // compro la caja

    s->capacidadMecanicos = 4;
    s->numMecanicos = 0;
    s->mecanicos = new Mecanico[s->capacidadMecanicos];

    s->capacidadRepuestos = 4;
    s->numRepuestos = 0;
    s->repuestos = new Repuesto[s->capacidadRepuestos];

    s->capacidadOrdenes = 4;
    s->numOrdenes = 0;
    s->ordenes = new OrdenServicio [s->capacidadOrdenes];

    s->siguienteIdVehiculo = 1;
    s->siguienteIdMecanico = 1;
    s->siguienteIdRepuesto = 1;
    s->siguienteIdOrden = 1;

    //Porque el primer vehículo que se registre será el #1, el primer mecánico el #1, etc. Cada vez que se registre uno, este número sube.
}

void  liberarSistema(SistemaTaller* s) {
    // Primero los detalles: su direccion esta guardada dentro de las ordenes.
    // Si borro primero las ordenes, pierdo esas direcciones y se crea una fuga de memoria.
    //si i = 0 y es menor q la cantidad de ordenes, en cada vuelta suma 1 a i
    for(int i= 0; i < s->numOrdenes; i++){
        delete[] s->ordenes[i].detalles;
    }

    delete[] s->vehiculos;
    delete[] s-> mecanicos;
    delete[] s-> repuestos;
    delete[] s->ordenes;

    s->vehiculos = nullptr;
    s->mecanicos = nullptr;
    s->repuestos = nullptr;
    s->ordenes = nullptr;
    //elimina direcciones
}
//redimensionarVehiculos: Cuando la caja de carros se llene, si la caja tiene espacios para 4 carros y llega un 5to, no cabe enotonces se busca una mas grande, se pasa los carros a la caja nueva, se delete la vieja y se usa la nueva apartir de ahora 
void redimensionarVehiculos(SistemaTaller* s) {

    //se busca la caja mas grande 
    int nuevaCapacidad = s->capacidadVehiculos * 2;
    Vehiculo* nueva = new Vehiculo[nuevaCapacidad];

    //pasar los carros de la vieja caja  a la nueva 
    for(int i= 0; i < s->numVehiculos; i++ ) {
        nueva[i] = s->vehiculos[i];

    }

    //devolviendo la caja vieja 
    delete[] s->vehiculos;

    //usar la caja nueva
    s->vehiculos = nueva;
    s->capacidadVehiculos = nuevaCapacidad;
}
void redimensionarMecanicos(SistemaTaller* s) {

    //se busca la caja mas grande 
    int nuevaCapacidad = s->capacidadMecanicos * 2;
    Mecanico* nueva = new Mecanico[nuevaCapacidad];

    //pasar los carros de la vieja caja  a la nueva 
    for(int i= 0; i < s->numMecanicos; i++ ) {
        nueva[i] = s->mecanicos[i];

    }

    //devolviendo la caja vieja 
    delete[] s->mecanicos;

    //usar la caja nueva
    s->mecanicos = nueva;
    s->capacidadMecanicos = nuevaCapacidad;
}
void redimensionarRepuestos(SistemaTaller* s) {

    //se busca la caja mas grande 
    int nuevaCapacidad = s->capacidadRepuestos * 2;
    Repuesto* nueva = new Repuesto[nuevaCapacidad];

    //pasar los carros de la vieja caja  a la nueva 
    for(int i= 0; i < s->numRepuestos; i++ ) {
        nueva[i] = s->repuestos[i];

    }

    //devolviendo la caja vieja 
    delete[] s->repuestos;

    //usar la caja nueva
    s->repuestos = nueva;
    s->capacidadRepuestos = nuevaCapacidad;
}
void redimensionarOrdenes(SistemaTaller* s) {

    //se busca la caja mas grande
    int nuevaCapacidad = s->capacidadOrdenes * 2;
    OrdenServicio* nueva = new OrdenServicio[nuevaCapacidad];

    //pasar las ordenes de la vieja caja a la nueva
    //(se copia la DIRECCION de los detalles de cada orden, no los detalles)
    for(int i = 0; i < s->numOrdenes; i++) {
        nueva[i] = s->ordenes[i];
    }

    //devolviendo la caja vieja
    //esto NO borra los detalles: siguen vivos y la orden nueva los apunta)
    delete[] s->ordenes;

    //usar la caja nueva
    s->ordenes = nueva;
    s->capacidadOrdenes = nuevaCapacidad;
}

void redimensionarDetalles(OrdenServicio* orden) {

    //se busca la caja mas grande
    int nuevaCapacidad = orden->capacidadDetalles * 2;
    DetalleOrden* nueva = new DetalleOrden[nuevaCapacidad];

    //pasar los detalles de la vieja caja a la nueva
    for(int i = 0; i < orden->numDetalles; i++) {
        nueva[i] = orden->detalles[i];
    }

    //devolviendo la caja vieja
    delete[] orden->detalles;

    //usar la caja nueva
    orden->detalles = nueva;
    orden->capacidadDetalles = nuevaCapacidad;
}

// Busca un vehiculo por su ID. Si no lo encuentra devuelve nullptr
Vehiculo* buscarVehiculoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numVehiculos; i++) {
        if (s->vehiculos[i].id == id) {
            return &s->vehiculos[i];
        }
    }
    return nullptr;
}

// Busca un vehiculo por su placa exacta. Si no lo encuentra devuelve nullptr
Vehiculo* buscarVehiculoPorPlaca(SistemaTaller* s, const char* placa) {
    for (int i = 0; i < s->numVehiculos; i++) {
        if (strcmp(s->vehiculos[i].placa, placa) == 0) {
            return &s->vehiculos[i];
        }
    }
    return nullptr;
}
// Agrega un vehiculo. Si algun dato es invalido devuelve nullptr
Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos) {
    // Validaciones
    if (!esPlacaValida(datos.placa)) {
        return nullptr;
    }
    if (buscarVehiculoPorPlaca(s, datos.placa) != nullptr) {
        return nullptr;
    }
    if (datos.anio < 1950 || datos.anio > 2027) {
        return nullptr;
    }
    if (datos.kilometraje < 0) {
        return nullptr;
    }
    if (!esCedulaValida(datos.cedulaPropietario)) {
        return nullptr;
    }

    // Si la caja esta llena, mudanza a una del doble
    if (s->numVehiculos == s->capacidadVehiculos) {
        redimensionarVehiculos(s);
    }

    // Le ponemos ID y fecha de registro
    datos.id = s->siguienteIdVehiculo;
    s->siguienteIdVehiculo++;
    fechaActual(datos.fechaRegistro);

    // Lo guardamos en el siguiente puesto libre
    s->vehiculos[s->numVehiculos] = datos;
    s->numVehiculos++;

    return &s->vehiculos[s->numVehiculos - 1];
}