# Sistema de Gestión de Taller Mecánico · Proyecto 2

Persistencia con archivos binarios y acceso aleatorio.
Programación 2 (2026-C), Universidad Rafael Urdaneta. Prof. Ing. Victor Kneider.

## Integrantes

Estudiantes de Ingeniería en Computación, Universidad Rafael Urdaneta.

- Rodolfo Inciarte
- Kolja Balzan

## Compilación y ejecución

```
g++ -std=c++11 -Wall -o taller main.cpp logica.cpp persistencia.cpp presentacion.cpp utilidades.cpp
```

```
.\taller.exe
```

Pruebas con entrada desde archivo (en PowerShell el `<` no funciona, se usa `cmd /c`):

```
cmd /c "taller.exe < pruebas\entrada_1a.txt > pruebas\salida_1a.txt"
```

Los archivos binarios están en la carpeta `datos/`. Si falta alguno, el programa lo crea al arrancar.

## Capas

| Capa | Archivos | Qué hace |
|---|---|---|
| Persistencia | `persistencia.h` / `.cpp` | Lee y escribe registros crudos en los `.bin`. No aplica reglas, no filtra eliminados, no usa `cin`/`cout` |
| Lógica | `logica.h` / `.cpp` | Reglas del negocio. Usa la persistencia. No usa `cin`/`cout` |
| Presentación | `presentacion.h` / `.cpp`, `main.cpp` | Menús y mensajes. No abre archivos |

## Archivos y tamaños

| Archivo | Contenido |
|---|---|
| `datos/taller.bin` | Un solo registro `Taller`, sin header |
| `datos/vehiculos.bin` | Header + vehículos |
| `datos/mecanicos.bin` | Header + mecánicos (activos y dados de baja) |
| `datos/repuestos.bin` | Header + repuestos |
| `datos/ordenes.bin` | Header + órdenes (cada una con sus líneas adentro) |

| Estructura | sizeof (bytes) |
|---|---|
| `ArchivoHeader` | 16 |
| `Taller` | 360 |
| `Vehiculo` | 288 |
| `Mecanico` | 176 |
| `Repuesto` | 208 |
| `DetalleOrden` | 12 |
| `OrdenServicio` | 464 |

## Acceso aleatorio

Fórmula de posición del registro con índice físico `i`:

```
posicion = sizeof(ArchivoHeader) + i * sizeof(Estructura)
```

**Relación entre ID e índice:** `i = id - 1`. Por lo tanto:

```
posicion = sizeof(ArchivoHeader) + (id - 1) * sizeof(Estructura)
```

Ejemplo: el vehículo con ID 2 está en el byte 16 + 1 × 288 = 304.

Esto es válido porque:

1. Los IDs empiezan en 1 y se asignan en orden de inserción (el header guarda `proximoID`).
2. Los registros se agregan siempre al final.
3. Nunca se mueven ni se borran físicamente (borrado lógico con `eliminado = true`).

**Qué la dejaría de hacer válida:** el borrado físico (correr los registros para tapar el hueco, como en el Proyecto 1), compactar el archivo, reutilizar IDs de registros borrados, o que los IDs no empezaran en 1. En cualquiera de esos casos, el ID `n` ya no estaría en el índice `n - 1` y habría que recorrer el archivo para encontrarlo.

Las búsquedas por otros campos (placa, cédula, código, nombre) sí recorren el archivo, leyendo un solo registro a la vez.

## Diseño del detalle de la orden

**Decisión:** cada `OrdenServicio` lleva adentro un arreglo de tamaño fijo con sus líneas.

```cpp
const int MAX_LINEAS = 15;

struct DetalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario;   // precio histórico
};

struct OrdenServicio {
    // ... campos de la orden ...
    DetalleOrden lineas[MAX_LINEAS];
    int numLineas;          // cuántas casillas están usadas
    // ...
};
```

**Cómo cumple los requisitos de la sección 3:**

1. Las líneas se guardan dentro de la orden, con repuesto, cantidad y precio histórico, así que sobreviven entre ejecuciones.
2. Admite 15 líneas por orden.
3. Todas las órdenes ocupan los mismos 464 bytes, así que el acceso aleatorio sigue funcionando.
4. Al leer una orden por ID vienen sus líneas, sin leer ninguna otra orden.
5. Se mantienen las reglas del Proyecto 1 (acumular unidades, quitar la línea completa devolviendo el stock, precio histórico).

**Bytes en disco:** una orden ocupa **464 bytes con 1 línea y 464 bytes con 15 líneas**. El arreglo de líneas mide 15 × 12 = 180 bytes; con 1 línea se usan 12 y quedan 168 sin usar.

**Alternativa descartada:** un archivo `detalles.bin` aparte, donde cada línea guarda el `idOrden` al que pertenece. No desperdicia espacio y no tiene límite de líneas, pero:

- Para mostrar una orden habría que recorrer `detalles.bin` buscando sus líneas, o mantener una lista enlazada de posiciones dentro del archivo.
- Quitar una línea dejaría huecos que habría que marcar como eliminados.
- Sería un archivo más con header, que hay que incluir en el backup y en la verificación de integridad.
- Cada operación de órdenes escribiría en un archivo más, con más puntos donde una interrupción deja datos inconsistentes.

Preferimos la solución más simple y segura, a cambio de 168 bytes por orden en el peor caso.

**Quitar una línea:** se busca la línea del repuesto, se devuelven sus unidades al stock y se corren las líneas siguientes una posición a la izquierda dentro del arreglo (así no quedan huecos entre las líneas usadas), se resta 1 a `numLineas` y se sobrescribe la orden en su misma posición del archivo.

## Firmas: capa de persistencia

Las cinco operaciones son iguales para vehículos, mecánicos, repuestos y órdenes; solo cambian el tipo y la ruta.

```cpp
bool inicializarArchivo(const char* ruta);
bool leerHeader(const char* ruta, ArchivoHeader* h);
bool actualizarHeader(const char* ruta, ArchivoHeader h);

long posicionVehiculo(int id);
Resultado guardarVehiculoNuevo(Vehiculo v, int* idAsignado);
Resultado leerVehiculoPorID(int id, Vehiculo* v);
Resultado sobrescribirVehiculo(Vehiculo v);
Resultado marcarVehiculoEliminado(int id);
Resultado contarVehiculosActivos(int* cantidad);
// igual para Mecanico, Repuesto y OrdenServicio

bool existeTaller();
bool guardarTaller(Taller t);
bool leerTaller(Taller* t);
bool inicializarArchivos();
```

**Cómo informan el resultado:**

- Ninguna función devuelve un puntero a un registro: los datos viven en disco, no en un arreglo en memoria. La función **llena una variable del que la llama** (`Vehiculo* v`) con una copia leída del disco.
- Devuelven un `Resultado` (enum en `resultados.h`): `OK`, `ERROR_ARCHIVO` (no se pudo abrir o leer) o `NO_EXISTE` (ese ID nunca se asignó).
- `leerXPorID` entrega el registro **aunque esté eliminado**; decidir si sirve es trabajo de la lógica.
- `guardarXNuevo` informa el ID asignado por `int* idAsignado`.
- Las funciones de header devuelven `bool`: si el archivo quedó listo o no.

## Firmas: capa de lógica

*(Se completa al terminar la lógica.)*

## Preguntas de análisis

### 1. Escribir con `write` una `OrdenServicio` del Proyecto 1 (con `DetalleOrden*`), cerrar, volver a ejecutar y leerla

`write` copia los bytes de la estructura tal cual. El campo `detalles` es un puntero: lo que se guarda es **una dirección de memoria de esa ejecución**, no las líneas. Al cerrar el programa, esa memoria se devuelve al sistema operativo. En la siguiente ejecución, al leer el registro, `detalles` trae una dirección vieja que ya no pertenece al programa: acceder a las líneas lee basura o cierra el programa (comportamiento indefinido). Por eso una estructura que va a disco no puede tener punteros: hay que guardar los datos mismos, con tamaño fijo.

### 2. Orden de escritura al agregar un repuesto a una orden

*(Se completa al programar las operaciones compuestas.)*

### 3. ¿Qué habría pasado con el historial si se eliminara físicamente un mecánico?

Las órdenes guardan el `idMecanico`. Si el mecánico se borrara físicamente, sus órdenes cerradas y anuladas apuntarían a un ID que ya no existe: no se podría mostrar quién hizo el trabajo, y la facturación por mecánico perdería esos datos. Además, con borrado físico los registros siguientes se correrían, el ID ya no coincidiría con la posición y la fórmula de acceso aleatorio dejaría de funcionar. La baja lógica (`eliminado = true`) conserva el registro en su lugar: el mecánico no recibe órdenes nuevas, pero su nombre sigue disponible para el historial.