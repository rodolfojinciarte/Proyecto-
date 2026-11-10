# Sistema de Gestión de Taller Mecánico · Proyecto 2

Persistencia con archivos binarios y acceso aleatorio.
Programación 2 (2026-C), Universidad Rafael Urdaneta. Prof. Ing. Victor Kneider.

## Integrantes

Estudiantes de Ingeniería en Computación, Universidad Rafael Urdaneta.

- Rodolfo Inciarte
- Kolja Balza

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

Para ejecutar los 8 casos de uso de una vez (ver la sección **Pruebas**):

```
cmd /c pruebas\ejecutar_pruebas.bat
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

```cpp
// Vehículos
Resultado registrarVehiculo(Vehiculo datos, int* idAsignado);
Resultado obtenerVehiculoPorID(int id, Vehiculo* v);
Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo* v);
Resultado buscarVehiculosPorPropietario(const char* texto, Vehiculo resultados[], int* cantidad, bool* hayMas);
Resultado listarVehiculos(Vehiculo resultados[], int* cantidad, bool* hayMas);
Resultado actualizarVehiculo(int id, Vehiculo datos);
Resultado eliminarVehiculo(int id);
long posicionEnDiscoVehiculo(int id);

// Mecánicos
bool esEspecialidadValida(const char* especialidad);
Resultado registrarMecanico(Mecanico datos, int* idAsignado);
Resultado obtenerMecanicoPorID(int id, Mecanico* m);
Resultado contarOrdenesAbiertas(int idMecanico, int* cantidad);
Resultado listarMecanicos(Mecanico resultados[], int ordenesAbiertas[], int* cantidad, bool* hayMas);
Resultado darDeBajaMecanico(int id);

// Repuestos
Resultado registrarRepuesto(Repuesto datos, int* idAsignado);
Resultado obtenerRepuestoPorID(int id, Repuesto* r);
Resultado obtenerRepuestoPorCodigo(const char* codigo, Repuesto* r);
Resultado buscarRepuestosPorNombre(const char* texto, Repuesto resultados[], int* cantidad, bool* hayMas);
Resultado registrarEntradaRepuesto(int id, int cantidad);
Resultado listarRepuestos(Repuesto resultados[], int* cantidad, bool* hayMas);
bool esBajoStock(Repuesto r);
Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio);
Resultado eliminarRepuesto(int id);

// Órdenes
Resultado abrirOrden(int idVehiculo, int idMecanico, int kilometraje, const char* falla, int* idAsignado);
Resultado obtenerOrdenPorID(int id, OrdenServicio* o);
Resultado agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad);
Resultado quitarRepuestoDeOrden(int idOrden, int idRepuesto);
Resultado cerrarOrden(int idOrden, float manoObra);
Resultado anularOrden(int idOrden);
float calcularTotalOrden(OrdenServicio o);
Resultado listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int* cantidad, bool* hayMas);
Resultado listarOrdenesPorVehiculo(int idVehiculo, OrdenServicio resultados[], int* cantidad, bool* hayMas);

// Taller, mantenimiento y reportes
bool prepararArchivos();
bool tallerConfigurado();
Resultado configurarTaller(Taller t);
Resultado obtenerTaller(Taller* t);
Resultado verificarIntegridad(ReporteIntegridad* rep);
Resultado crearBackup(char* rutaBackup);
Resultado reporteHistorialVehiculo(const char* placa, Vehiculo* v, OrdenServicio ordenes[], int* cantidad, bool* hayMas, float* totalCerradas);
Resultado reporteRepuestosMasUsados(FilaTopRepuesto top[], int* cantidad);
Resultado reporteFacturacionMecanicos(FilaFacturacion filas[], int* cantidad, bool* hayMas);
```

**Cómo informan el éxito, el fallo y su causa:**

- Toda operación que puede fallar devuelve un `Resultado`. Cuando hay más de una causa posible, cada causa tiene su valor: por ejemplo, `abrirOrden` puede devolver `NO_EXISTE`, `ELIMINADO`, `VEHICULO_CON_ORDEN_ABIERTA`, `MECANICO_OCUPADO` o `KILOMETRAJE_MENOR`. Así la presentación muestra el mensaje exacto sin repetir las validaciones.
- Igual que en la persistencia, **ninguna función devuelve un puntero a un registro**: el registro leído se copia en una variable del que llama (`Vehiculo* v`) y el ID nuevo se entrega por `int* idAsignado`.
- `obtenerXPorID` devuelve `ELIMINADO` cuando el registro existe pero fue borrado, **y aun así llena la variable**, para que la presentación pueda decir cuál era (por ejemplo, el nombre de un mecánico dado de baja).
- Las búsquedas y listados no cargan el archivo completo: llenan un arreglo de máximo `MAX_RESULTADOS = 20` y ponen `hayMas = true` si quedaron registros sin entregar. La presentación avisa en ese caso.
- Las funciones que solo responden sí/no (`esBajoStock`, `esEspecialidadValida`, `tallerConfigurado`) devuelven `bool` porque no pueden fallar.
- La lógica no usa `cin`/`cout` y no abre archivos: todo lo lee y escribe a través de la persistencia.

## Mantenimiento

- **Verificar integridad** (`verificarIntegridad`): recorre los archivos un registro a la vez y anota como `ROTO:` cada orden que apunta a un vehículo, mecánico o repuesto inexistente, cada orden ABIERTA con vehículo o mecánico inactivo, cada repuesto con stock negativo y cada header que no coincide con lo que hay en el archivo (registros físicos contados con `tellg` y registros activos).
- **Backup** (`crearBackup`): crea la carpeta `backup_AAAA-MM-DD_HH-MM-SS` y copia byte a byte los cinco `.bin`.

## Reportes

Los tres reportes leen los archivos un registro a la vez, sin cargarlos completos en memoria. Solo suman las órdenes **CERRADAS**; las ABIERTAS y ANULADAS no.

1. **Historial de vehículo:** dada una placa, muestra el vehículo y todas sus órdenes (número, fecha, mecánico, estado y total), más el total facturado en órdenes cerradas. Las que no suman se marcan "(no suma)".
2. **Repuestos más utilizados:** para cada repuesto cuenta las unidades usadas en órdenes cerradas y lo inserta ordenado en un arreglo de 10 posiciones (`TOP_REPUESTOS`). Solo existe ese arreglo de 10 en memoria.
3. **Facturación por mecánico:** para cada mecánico (activos y dados de baja, marcados `(BAJA)`) muestra cuántas órdenes cerradas tiene, el total de mano de obra y el total de repuestos de esas órdenes.

## Datos de ejemplo

La carpeta `datos/` se entrega con: 8 vehículos, 4 mecánicos (Luis Fuenmayor dado de baja), 12 repuestos y 10 órdenes:

| Estado | Órdenes |
|---|---|
| CERRADA | #1, #2, #3, #4, #5 |
| ANULADA | #6 (anulada después de cerrarla), #7 (anulada estando abierta) |
| ABIERTA | #8, #9, #10 |

Las órdenes #1, #2, #3 y #10 tienen 4 o más líneas. Estos datos se generan con el Caso de Uso 8.

## Pruebas

La carpeta `pruebas/` tiene una entrada y una salida por cada ejecución del programa. Cuando un caso necesita reiniciar el programa, hay un archivo por ejecución (`entrada_3a.txt`, `entrada_3b.txt`, `entrada_3c.txt`, …).

`pruebas/ejecutar_pruebas.bat` ejecuta todos los casos: antes de cada caso borra los `.bin` de `datos/` para empezar de cero, y al final `datos/` queda con los datos de ejemplo.

| Caso | Archivos | Qué demuestra |
|---|---|---|
| 1. Persistencia | 1a, 1b | 1a registra 3 vehículos, 2 mecánicos y 4 repuestos; 1b (otra ejecución) los lista con los mismos IDs |
| 2. Acceso directo | 2a | Registra 6 vehículos, elimina el 3, busca el 5 (byte 1168) y el 3 (informa ELIMINADO) |
| 3. Orden con detalle | 3a, 3b, 3c | 3a abre la orden y agrega 3 repuestos; 3b agrega un 4.º, acumula 2 unidades, quita uno (su stock vuelve) y la cierra; además cambia el precio del filtro; 3c muestra la orden con el precio histórico |
| 4. 15 líneas | 4a, 4b | 4a agrega 15 repuestos y el 16.º es rechazado; 4b muestra las 15 líneas después de reiniciar |
| 5. Anulación | 5a, 5b, 5c | 5a cierra una orden de 3 líneas y lista el stock; 5b la anula; 5c muestra que el stock volvió al valor previo |
| 6. Baja de mecánico | 6a, 6b | 6a: baja rechazada con orden abierta, se cierra la orden y la baja es aceptada; 6b: la orden cerrada sigue con su nombre y una orden nueva con él es rechazada |
| 7. Integridad | 7a, 7b | 7a crea un backup, registra un vehículo y le abre una orden; el `.bat` copia `vehiculos.bin` del backup; 7b detecta la orden que apunta a un vehículo inexistente |
| 8. Reportes | 8a, 8b | 8a carga los datos de ejemplo; 8b genera los tres reportes (las órdenes abiertas y anuladas no suman) y verifica la integridad |

## Preguntas de análisis

### 1. Escribir con `write` una `OrdenServicio` del Proyecto 1 (con `DetalleOrden*`), cerrar, volver a ejecutar y leerla

`write` copia los bytes de la estructura tal cual. El campo `detalles` es un puntero: lo que se guarda es **una dirección de memoria de esa ejecución**, no las líneas. Al cerrar el programa, esa memoria se devuelve al sistema operativo. En la siguiente ejecución, al leer el registro, `detalles` trae una dirección vieja que ya no pertenece al programa: acceder a las líneas lee basura o cierra el programa (comportamiento indefinido). Por eso una estructura que va a disco no puede tener punteros: hay que guardar los datos mismos, con tamaño fijo.

### 2. Orden de escritura al agregar un repuesto a una orden

Al agregar un repuesto se escriben dos archivos: la orden (con su línea nueva) en `ordenes.bin` y el stock en `repuestos.bin`. Antes de escribir nada, la lógica hace **todas las validaciones** (orden ABIERTA, repuesto activo, stock suficiente, espacio para la línea). Recién entonces escribe, y elegimos este orden: **primero la orden, después el stock**.

Si el programa se interrumpe entre las dos escrituras, la orden ya tiene la línea pero el stock no se descontó: el inventario queda **más alto** de lo real.

Este orden deja el daño más fácil de detectar y corregir porque la línea de la orden queda como **evidencia**: dice qué repuesto, cuántas unidades y a qué precio. Comparando las líneas de las órdenes con el stock se puede saber cuánto falta descontar.

Con el orden contrario (primero el stock), el stock bajaría pero la orden no tendría la línea: las unidades desaparecerían sin dejar rastro de a dónde fueron, y no habría forma de saber qué pasó.

Usamos el mismo criterio en todas las operaciones compuestas: abrir orden (orden y después kilometraje del vehículo), quitar repuesto y anular (orden y después devolución del stock).

### 3. ¿Qué habría pasado con el historial si se eliminara físicamente un mecánico?

Las órdenes guardan el `idMecanico`. Si el mecánico se borrara físicamente, sus órdenes cerradas y anuladas apuntarían a un ID que ya no existe: no se podría mostrar quién hizo el trabajo, y la facturación por mecánico perdería esos datos. Además, con borrado físico los registros siguientes se correrían, el ID ya no coincidiría con la posición y la fórmula de acceso aleatorio dejaría de funcionar. La baja lógica (`eliminado = true`) conserva el registro en su lugar: el mecánico no recibe órdenes nuevas, pero su nombre sigue disponible para el historial.
