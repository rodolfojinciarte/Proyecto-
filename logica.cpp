#include "logica.h"
#include "utilidades.h"
#include <cstring>

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

// Devuelve una lista de punteros a todos los vehiculos
// Quien la llame debe borrarla con delete[]
Vehiculo** listarVehiculos(SistemaTaller* s, int* cantidad) {
    *cantidad = s->numVehiculos;
    Vehiculo** lista = new Vehiculo*[s->numVehiculos];
    for (int i = 0; i < s->numVehiculos; i++) {
        lista[i] = &s->vehiculos[i];
    }
    return lista;
}

// Devuelve los vehiculos cuyo propietario contiene el texto buscado
// Quien la llame debe borrarla con delete[]
Vehiculo** buscarVehiculosPorPropietario(SistemaTaller* s, const char* subcadena, int* cantidad) {
    Vehiculo** lista = new Vehiculo*[s->numVehiculos];
    *cantidad = 0;
    for (int i = 0; i < s->numVehiculos; i++) {
        if (contieneSinMayusculas(s->vehiculos[i].nombrePropietario, subcadena)) {
            lista[*cantidad] = &s->vehiculos[i];
            (*cantidad)++;
        }
    }
    return lista;
}

// Actualiza marca, modelo, anio y datos del propietario
// No cambia ID, placa, kilometraje ni fecha de registro
bool actualizarVehiculo(SistemaTaller* s, int id, Vehiculo datos) {
    Vehiculo* v = buscarVehiculoPorID(s, id);
    if (v == nullptr) {
        return false;
    }
    if (datos.anio < 1950 || datos.anio > 2027) {
        return false;
    }
    if (!esCedulaValida(datos.cedulaPropietario)) {
        return false;
    }
    strcpy(v->marca, datos.marca);
    strcpy(v->modelo, datos.modelo);
    v->anio = datos.anio;
    strcpy(v->nombrePropietario, datos.nombrePropietario);
    strcpy(v->cedulaPropietario, datos.cedulaPropietario);
    strcpy(v->telefonoPropietario, datos.telefonoPropietario);
    return true;
}

// Elimina un vehiculo corriendo los siguientes un puesto hacia atras
// No se puede si tiene ordenes registradas
bool eliminarVehiculo(SistemaTaller* s, int id) {
    int posicion = -1;
    for (int i = 0; i < s->numVehiculos; i++) {
        if (s->vehiculos[i].id == id) {
            posicion = i;
        }
    }
    if (posicion == -1) {
        return false;
    }
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == id) {
            return false;
        }
    }
    for (int i = posicion; i < s->numVehiculos - 1; i++) {
        s->vehiculos[i] = s->vehiculos[i + 1];
    }
    s->numVehiculos--;
    return true;
}
// ===== MECANICOS =====

// Revisa que la especialidad sea una de las 5 permitidas
bool esEspecialidadValida(const char* especialidad) {
    const char* permitidas[5] = {"MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(especialidad, permitidas[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Retorna el mecanico con ese ID, o nullptr si no existe
Mecanico* buscarMecanicoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numMecanicos; i++) {
        if (s->mecanicos[i].id == id) {
            return &s->mecanicos[i];
        }
    }
    return nullptr;
}

// Retorna el mecanico con esa cedula, o nullptr si no existe
Mecanico* buscarMecanicoPorCedula(SistemaTaller* s, const char* cedula) {
    for (int i = 0; i < s->numMecanicos; i++) {
        if (strcmp(s->mecanicos[i].cedula, cedula) == 0) {
            return &s->mecanicos[i];
        }
    }
    return nullptr;
}

// Valida nombre, cedula unica y especialidad. Asigna ID y fecha de ingreso
Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos) {
    if (strlen(datos.nombre) == 0) {
        return nullptr;
    }
    if (!esCedulaValida(datos.cedula)) {
        return nullptr;
    }
    if (buscarMecanicoPorCedula(s, datos.cedula) != nullptr) {
        return nullptr;
    }
    if (!esEspecialidadValida(datos.especialidad)) {
        return nullptr;
    }
    if (s->numMecanicos == s->capacidadMecanicos) {
        redimensionarMecanicos(s);
    }
    datos.id = s->siguienteIdMecanico;
    s->siguienteIdMecanico++;
    fechaActual(datos.fechaIngreso);
    s->mecanicos[s->numMecanicos] = datos;
    s->numMecanicos++;
    return &s->mecanicos[s->numMecanicos - 1];
}

// Cuenta cuantas ordenes ABIERTAS tiene un mecanico
int contarOrdenesAbiertas(SistemaTaller* s, int idMecanico) {
    int total = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idMecanico == idMecanico && strcmp(s->ordenes[i].estado, "ABIERTA") == 0) {
            total++;
        }
    }
    return total;
}

// Retorna lista de punteros a todos los mecanicos
// Quien la llame debe borrarla con delete[]
Mecanico** listarMecanicos(SistemaTaller* s, int* cantidad) {
    *cantidad = s->numMecanicos;
    Mecanico** lista = new Mecanico*[s->numMecanicos];
    for (int i = 0; i < s->numMecanicos; i++) {
        lista[i] = &s->mecanicos[i];
    }
    return lista;
}

// ===== REPUESTOS =====

// Retorna el repuesto con ese ID, o nullptr si no existe
Repuesto* buscarRepuestoPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numRepuestos; i++) {
        if (s->repuestos[i].id == id) {
            return &s->repuestos[i];
        }
    }
    return nullptr;
}

// Retorna el repuesto con ese codigo exacto, o nullptr
Repuesto* buscarRepuestoPorCodigo(SistemaTaller* s, const char* codigo) {
    for (int i = 0; i < s->numRepuestos; i++) {
        if (strcmp(s->repuestos[i].codigo, codigo) == 0) {
            return &s->repuestos[i];
        }
    }
    return nullptr;
}

// Valida nombre, codigo unico, precio > 0, stock >= 0 y stock minimo >= 0
Repuesto* agregarRepuesto(SistemaTaller* s, Repuesto datos) {
    if (strlen(datos.nombre) == 0 || strlen(datos.codigo) == 0) {
        return nullptr;
    }
    if (buscarRepuestoPorCodigo(s, datos.codigo) != nullptr) {
        return nullptr;
    }
    if (datos.precio <= 0 || datos.stock < 0 || datos.stockMinimo < 0) {
        return nullptr;
    }
    if (s->numRepuestos == s->capacidadRepuestos) {
        redimensionarRepuestos(s);
    }
    datos.id = s->siguienteIdRepuesto;
    s->siguienteIdRepuesto++;
    fechaActual(datos.fechaRegistro);
    s->repuestos[s->numRepuestos] = datos;
    s->numRepuestos++;
    return &s->repuestos[s->numRepuestos - 1];
}

// Busca repuestos cuyo codigo o nombre contenga el texto
// Quien la llame debe borrarla con delete[]
Repuesto** buscarRepuestosPorTexto(SistemaTaller* s, const char* texto, int* cantidad) {
    Repuesto** lista = new Repuesto*[s->numRepuestos];
    *cantidad = 0;
    for (int i = 0; i < s->numRepuestos; i++) {
        if (contieneSinMayusculas(s->repuestos[i].nombre, texto) ||
            contieneSinMayusculas(s->repuestos[i].codigo, texto)) {
            lista[*cantidad] = &s->repuestos[i];
            (*cantidad)++;
        }
    }
    return lista;
}

// Suma unidades al stock. La cantidad debe ser mayor que 0
bool registrarEntrada(SistemaTaller* s, int id, int cantidad) {
    Repuesto* r = buscarRepuestoPorID(s, id);
    if (r == nullptr || cantidad <= 0) {
        return false;
    }
    r->stock = r->stock + cantidad;
    return true;
}

// Retorna lista de punteros a todos los repuestos
// Quien la llame debe borrarla con delete[]
Repuesto** listarRepuestos(SistemaTaller* s, int* cantidad) {
    *cantidad = s->numRepuestos;
    Repuesto** lista = new Repuesto*[s->numRepuestos];
    for (int i = 0; i < s->numRepuestos; i++) {
        lista[i] = &s->repuestos[i];
    }
    return lista;
}

// true si el stock esta en el minimo o por debajo
bool esBajoStock(Repuesto* r) {
    return r->stock <= r->stockMinimo;
}

// Cambia el precio. No toca las ordenes ya registradas
bool cambiarPrecio(SistemaTaller* s, int id, float nuevoPrecio) {
    Repuesto* r = buscarRepuestoPorID(s, id);
    if (r == nullptr || nuevoPrecio <= 0) {
        return false;
    }
    r->precio = nuevoPrecio;
    return true;
}

// Elimina solo si el repuesto no aparece en ninguna orden
bool eliminarRepuesto(SistemaTaller* s, int id) {
    int posicion = -1;
    for (int i = 0; i < s->numRepuestos; i++) {
        if (s->repuestos[i].id == id) {
            posicion = i;
        }
    }
    if (posicion == -1) {
        return false;
    }
    for (int i = 0; i < s->numOrdenes; i++) {
        for (int j = 0; j < s->ordenes[i].numDetalles; j++) {
            if (s->ordenes[i].detalles[j].idRepuesto == id) {
                return false;
            }
        }
    }
    for (int i = posicion; i < s->numRepuestos - 1; i++) {
        s->repuestos[i] = s->repuestos[i + 1];
    }
    s->numRepuestos--;
    return true;
}

// ===== ORDENES =====

// Retorna la orden con ese ID, o nullptr si no existe
OrdenServicio* buscarOrdenPorID(SistemaTaller* s, int id) {
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].id == id) {
            return &s->ordenes[i];
        }
    }
    return nullptr;
}

// Retorna la orden ABIERTA de un vehiculo, o nullptr si no tiene
OrdenServicio* buscarOrdenAbiertaDeVehiculo(SistemaTaller* s, int idVehiculo) {
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == idVehiculo && strcmp(s->ordenes[i].estado, "ABIERTA") == 0) {
            return &s->ordenes[i];
        }
    }
    return nullptr;
}

// Abre una orden nueva aplicando todas las reglas del PDF
OrdenServicio* abrirOrden(SistemaTaller* s, int idVehiculo, int idMecanico, int kilometraje, const char* falla) {
    Vehiculo* v = buscarVehiculoPorID(s, idVehiculo);
    Mecanico* m = buscarMecanicoPorID(s, idMecanico);
    if (v == nullptr || m == nullptr) {
        return nullptr;
    }
    if (buscarOrdenAbiertaDeVehiculo(s, idVehiculo) != nullptr) {
        return nullptr;
    }
    if (contarOrdenesAbiertas(s, idMecanico) >= 3) {
        return nullptr;
    }
    if (kilometraje < v->kilometraje) {
        return nullptr;
    }
    if (s->numOrdenes == s->capacidadOrdenes) {
        redimensionarOrdenes(s);
    }
    OrdenServicio* o = &s->ordenes[s->numOrdenes];
    o->id = s->siguienteIdOrden;
    s->siguienteIdOrden++;
    o->idVehiculo = idVehiculo;
    o->idMecanico = idMecanico;
    fechaActual(o->fechaApertura);
    strcpy(o->fechaCierre, "");
    strcpy(o->estado, "ABIERTA");
    strncpy(o->descripcionFalla, falla, 199);
    o->descripcionFalla[199] = '\0';
    o->kilometrajeIngreso = kilometraje;
    o->manoObra = 0;
    o->detalles = new DetalleOrden[2];
    o->numDetalles = 0;
    o->capacidadDetalles = 2;
    s->numOrdenes++;
    v->kilometraje = kilometraje;
    return o;
}

// Agrega repuesto a una orden ABIERTA. Descuenta stock y guarda el precio de hoy
// Si el repuesto ya esta en la orden, suma en la misma linea
bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad) {
    OrdenServicio* o = buscarOrdenPorID(s, idOrden);
    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (o == nullptr || r == nullptr) {
        return false;
    }
    if (strcmp(o->estado, "ABIERTA") != 0) {
        return false;
    }
    if (cantidad <= 0 || cantidad > r->stock) {
        return false;
    }
    for (int i = 0; i < o->numDetalles; i++) {
        if (o->detalles[i].idRepuesto == idRepuesto) {
            o->detalles[i].cantidad = o->detalles[i].cantidad + cantidad;
            r->stock = r->stock - cantidad;
            return true;
        }
    }
    if (o->numDetalles == o->capacidadDetalles) {
        redimensionarDetalles(o);
    }
    o->detalles[o->numDetalles].idRepuesto = idRepuesto;
    o->detalles[o->numDetalles].cantidad = cantidad;
    o->detalles[o->numDetalles].precioUnitario = r->precio;
    o->numDetalles++;
    r->stock = r->stock - cantidad;
    return true;
}

// Quita la linea completa y devuelve sus unidades al inventario
bool quitarRepuestoDeOrden(SistemaTaller* s, int idOrden, int idRepuesto) {
    OrdenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || strcmp(o->estado, "ABIERTA") != 0) {
        return false;
    }
    int posicion = -1;
    for (int i = 0; i < o->numDetalles; i++) {
        if (o->detalles[i].idRepuesto == idRepuesto) {
            posicion = i;
        }
    }
    if (posicion == -1) {
        return false;
    }
    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r != nullptr) {
        r->stock = r->stock + o->detalles[posicion].cantidad;
    }
    for (int i = posicion; i < o->numDetalles - 1; i++) {
        o->detalles[i] = o->detalles[i + 1];
    }
    o->numDetalles--;
    return true;
}

// Cierra una orden ABIERTA. Necesita al menos un repuesto o mano de obra > 0
bool cerrarOrden(SistemaTaller* s, int idOrden, float manoObra) {
    OrdenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || strcmp(o->estado, "ABIERTA") != 0) {
        return false;
    }
    if (manoObra < 0) {
        return false;
    }
    if (o->numDetalles == 0 && manoObra == 0) {
        return false;
    }
    o->manoObra = manoObra;
    strcpy(o->estado, "CERRADA");
    fechaActual(o->fechaCierre);
    return true;
}

// Anula una orden ABIERTA o CERRADA y devuelve todo al inventario
bool anularOrden(SistemaTaller* s, int idOrden) {
    OrdenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || strcmp(o->estado, "ANULADA") == 0) {
        return false;
    }
    for (int i = 0; i < o->numDetalles; i++) {
        Repuesto* r = buscarRepuestoPorID(s, o->detalles[i].idRepuesto);
        if (r != nullptr) {
            r->stock = r->stock + o->detalles[i].cantidad;
        }
    }
    strcpy(o->estado, "ANULADA");
    return true;
}

// Total = mano de obra + suma de (cantidad x precio guardado)
float calcularTotalOrden(OrdenServicio* o) {
    float total = o->manoObra;
    for (int i = 0; i < o->numDetalles; i++) {
        total = total + o->detalles[i].cantidad * o->detalles[i].precioUnitario;
    }
    return total;
}

// Lista las ordenes con ese estado. Quien la llame la borra con delete[]
OrdenServicio** listarOrdenesPorEstado(SistemaTaller* s, const char* estado, int* cantidad) {
    OrdenServicio** lista = new OrdenServicio*[s->numOrdenes];
    *cantidad = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (strcmp(s->ordenes[i].estado, estado) == 0) {
            lista[*cantidad] = &s->ordenes[i];
            (*cantidad)++;
        }
    }
    return lista;
}

// Historial de un vehiculo. Quien la llame la borra con delete[]
OrdenServicio** listarOrdenesPorVehiculo(SistemaTaller* s, int idVehiculo, int* cantidad) {
    OrdenServicio** lista = new OrdenServicio*[s->numOrdenes];
    *cantidad = 0;
    for (int i = 0; i < s->numOrdenes; i++) {
        if (s->ordenes[i].idVehiculo == idVehiculo) {
            lista[*cantidad] = &s->ordenes[i];
            (*cantidad)++;
        }
    }
    return lista;
}