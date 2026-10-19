# Sistema de Gestión de Taller Mecánico

Proyecto 1 de Programación 2 (2026-C), Universidad Rafael Urdaneta.
Prof. Ing. Victor Kneider.

## Integrantes

Estudiantes de Ingeniería en Computación, Universidad Rafael Urdaneta.

- Rodolfo Inciarte
- Kolja Balzan

## Descripción

Programa de consola en C++11 para administrar un taller mecánico. Permite:

- Registrar, buscar, actualizar, eliminar y listar **vehículos**.
- Registrar, buscar y listar **mecánicos**, con sus órdenes abiertas.
- Manejar el **inventario de repuestos**: entradas de mercancía, cambio de precio y aviso de BAJO STOCK.
- Abrir, cerrar y anular **órdenes de servicio**, agregar y quitar repuestos, y ver el detalle como una factura.

Toda la memoria se maneja a mano con `new` y `delete[]`. No se usa `vector` ni otros contenedores de la STL. `std::string` solo se usa para leer texto del teclado.

## Compilación y ejecución

```
g++ -std=c++11 -Wall -o taller main.cpp logica.cpp utilidades.cpp presentacion.cpp
```

Ejecutar (Windows):

```
.\taller.exe
```

Ejecutar las pruebas con la entrada desde archivo:

```
./taller < pruebas/entrada.txt > pruebas/salida.txt
```

En PowerShell el símbolo `<` no funciona, así que se usa:

```
cmd /c "taller.exe < pruebas\entrada.txt > pruebas\salida.txt"
```

## Archivos

| Archivo | Qué contiene |
|---|---|
| `estructuras.h` | Los 7 `struct` del enunciado |
| `logica.h` / `logica.cpp` | Capa de lógica: memoria, búsquedas, reglas del negocio |
| `presentacion.h` / `presentacion.cpp` | Capa de presentación: menús, lectura y mensajes |
| `utilidades.h` / `utilidades.cpp` | Validaciones (placa, cédula, fecha) y lectura segura de datos |
| `main.cpp` | Datos del taller y menú principal |
| `pruebas/` | `entrada.txt` y `salida.txt` |

## Diagrama de estructuras

```
SistemaTaller
 |-- taller ............ Taller (nombre, rif, direccion, telefono)
 |
 |-- vehiculos ------> [ Vehiculo | Vehiculo | Vehiculo | ... ]   num / capacidad
 |-- mecanicos ------> [ Mecanico | Mecanico | ... ]             num / capacidad
 |-- repuestos ------> [ Repuesto | Repuesto | ... ]             num / capacidad
 |-- ordenes --------> [ OrdenServicio | OrdenServicio | ... ]   num / capacidad
 |                            |
 |                            |-- idVehiculo  ...> Vehiculo.id
 |                            |-- idMecanico  ...> Mecanico.id
 |                            |-- detalles ---> [ DetalleOrden | DetalleOrden | ... ]
 |                                                   |-- idRepuesto ...> Repuesto.id
 |                                                   |-- cantidad
 |                                                   |-- precioUnitario
 |
 |-- siguienteIdVehiculo, siguienteIdMecanico, siguienteIdRepuesto, siguienteIdOrden
```

- `--->` es un puntero a un arreglo dinámico creado con `new`.
- `...>` es una relación por ID (se guarda el número, no un puntero).
- Cada orden es dueña de su propio arreglo de detalles (capacidad inicial 2).
- Los arreglos del sistema empiezan con capacidad 4 y se duplican cuando `num == capacidad`.

## Separación de capas

**Capa de lógica (`logica.cpp`)**: hace el trabajo. Busca, valida, guarda, borra y calcula. Nunca usa `cin` ni `cout`. Devuelve punteros, `bool`, números o arreglos de punteros para que la otra capa decida qué mostrar.

**Capa de presentación (`presentacion.cpp`)**: habla con el usuario. Muestra menús, pide datos, llama a la lógica y muestra el resultado. Sus funciones son `void`.

Ejemplo: `agregarRepuesto` (lógica) solo devuelve el repuesto creado o `nullptr`. `menuRegistrarRepuesto` (presentación) pide los datos y escribe "Repuesto registrado" o el error.

Ventaja: la lógica se puede probar sin menús, y si mañana cambia la interfaz, la lógica queda igual.

## Firmas diseñadas

Seguimos el mismo patrón de Vehículos que da el enunciado:

- **Agregar** devuelve un puntero al elemento creado, o `nullptr` si algún dato no es válido.
- **Buscar** devuelve un puntero, o `nullptr` si no existe.
- **Listar** devuelve `Entidad**` (un arreglo de direcciones) y la cantidad por `int*`. Quien llama libera solo ese arreglo con `delete[]`.
- **Acciones** que pueden salir bien o mal devuelven `bool`.

### Mecánicos

```cpp
bool esEspecialidadValida(const char* especialidad);
Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos);
Mecanico* buscarMecanicoPorID(SistemaTaller* s, int id);
Mecanico* buscarMecanicoPorCedula(SistemaTaller* s, const char* cedula);
int contarOrdenesAbiertas(SistemaTaller* s, int idMecanico);
Mecanico** listarMecanicos(SistemaTaller* s, int* cantidad);
```

- `agregarMecanico` devuelve `Mecanico*` para que el menú pueda mostrar el ID asignado. Devuelve `nullptr` si la cédula no es válida o si la especialidad no es una de las 5 permitidas.
- `contarOrdenesAbiertas` devuelve `int` porque la respuesta es una cantidad, no un sí o un no.

### Repuestos

```cpp
Repuesto* agregarRepuesto(SistemaTaller* s, Repuesto datos);
Repuesto* buscarRepuestoPorID(SistemaTaller* s, int id);
Repuesto* buscarRepuestoPorCodigo(SistemaTaller* s, const char* codigo);
Repuesto** buscarRepuestosPorTexto(SistemaTaller* s, const char* texto, int* cantidad);
bool registrarEntrada(SistemaTaller* s, int id, int cantidad);
Repuesto** listarRepuestos(SistemaTaller* s, int* cantidad);
bool esBajoStock(Repuesto* r);
bool cambiarPrecio(SistemaTaller* s, int id, float nuevoPrecio);
bool eliminarRepuesto(SistemaTaller* s, int id);
```

- `buscarRepuestosPorTexto` devuelve `Repuesto**` porque puede haber varias coincidencias.
- `esBajoStock` devuelve `bool`: el repuesto está en bajo stock cuando `stock <= stockMinimo`.
- `eliminarRepuesto` devuelve `false` si el repuesto no existe o si aparece en alguna orden, para no dejar órdenes apuntando a un repuesto borrado.

### Órdenes

```cpp
OrdenServicio* buscarOrdenPorID(SistemaTaller* s, int id);
OrdenServicio* buscarOrdenAbiertaDeVehiculo(SistemaTaller* s, int idVehiculo);
OrdenServicio* abrirOrden(SistemaTaller* s, int idVehiculo, int idMecanico, int kilometraje, const char* falla);
bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad);
bool quitarRepuestoDeOrden(SistemaTaller* s, int idOrden, int idRepuesto);
bool cerrarOrden(SistemaTaller* s, int idOrden, float manoObra);
bool anularOrden(SistemaTaller* s, int idOrden);
float calcularTotalOrden(OrdenServicio* o);
OrdenServicio** listarOrdenesPorEstado(SistemaTaller* s, const char* estado, int* cantidad);
OrdenServicio** listarOrdenesPorVehiculo(SistemaTaller* s, int idVehiculo, int* cantidad);
```

- `abrirOrden` devuelve `OrdenServicio*` para mostrar el número de la orden nueva. Devuelve `nullptr` si el vehículo o el mecánico no existen, o si el vehículo ya tiene una orden abierta.
- `buscarOrdenAbiertaDeVehiculo` sirve para cumplir la regla de una sola orden abierta por vehículo.
- `agregarRepuestoAOrden` descuenta el stock; `quitarRepuestoDeOrden` y `anularOrden` lo devuelven. Así el inventario y las órdenes siempre cuadran.
- `calcularTotalOrden` devuelve `float` y calcula el total cada vez (repuestos + mano de obra). El total no se guarda, porque un dato calculado que se guarda puede quedar desactualizado.

**Limitación que reconocemos:** las acciones que devuelven `bool` no dicen *por qué* fallaron. Elegimos `bool` por simplicidad, y el menú muestra un mensaje con las causas posibles. Para distinguir el motivo exacto se podría devolver un `int` con un código de error.

## Decisiones de diseño

- **Cédula:** se acepta `12345678` o `V-12345678`, porque el enunciado dice que el prefijo es opcional.
- **Precios en centavos:** el precio y la mano de obra se escriben en centavos (`2550` = 25.50) porque `leerEntero` valida enteros. Luego se divide entre `100.0f`.
- **Bordes ASCII** (`+`, `=`, `|`) en vez de `╔═╗`: los caracteres especiales salían como símbolos raros en la consola de Windows y en `salida.txt`.
- **Fin de entrada:** si el programa lee desde un archivo y el archivo se acaba, se cierra solo en vez de repetir "Valor invalido" para siempre.
- **Carácter `\r`:** en Windows, cada línea de un `.txt` termina con un `\r` invisible. `leerTexto` lo quita para que las placas y nombres leídos del archivo sean válidos.

## Preguntas de análisis

### 1. `agregarVehiculo` retorna un puntero a un elemento de `s->vehiculos`. Si guardan ese puntero en una variable, registran varios vehículos más y luego lo usan, ¿qué puede ocurrir y por qué?

El puntero guarda la dirección de una casilla del arreglo actual. Si al registrar más vehículos el arreglo se llena, `redimensionarVehiculos` pide un arreglo nuevo del doble, copia los vehículos y hace `delete[]` del arreglo viejo. El puntero guardado sigue apuntando al arreglo viejo, que ya fue devuelto. Es un **puntero colgante**: al usarlo se puede leer basura, modificar memoria ajena o cerrar el programa (comportamiento indefinido).

Pasa algo parecido si se elimina un vehículo anterior: los de la derecha se corren una posición y el puntero termina señalando a otro vehículo.

Por eso, después de agregar o eliminar no guardamos punteros por mucho tiempo: guardamos el **ID** y volvemos a buscar con `buscarVehiculoPorID`.

### 2. ¿Por qué `DetalleOrden` guarda `precioUnitario` en lugar de consultar el precio del repuesto cada vez que se calcula el total?

Porque el precio de un repuesto puede cambiar con `cambiarPrecio`. Si la orden consultara el precio actual, una orden cerrada hace un mes cambiaría su total cuando suba el precio, y la factura ya cobrada dejaría de coincidir. Guardar `precioUnitario` es como una foto del precio en el momento en que se usó el repuesto: el historial de la orden no cambia.

### 3. Cuando se redimensiona el arreglo de órdenes, ¿qué ocurre con el arreglo de detalles de cada orden? ¿Se copia, se comparte o se pierde?

**No se copia ni se pierde: se pasa la dirección.** En `redimensionarOrdenes` hacemos:

```cpp
for (int i = 0; i < s->numOrdenes; i++) {
    nueva[i] = s->ordenes[i];   // copia la orden, incluido el puntero detalles
}
delete[] s->ordenes;            // borra solo las ordenes viejas
```

Al copiar una `OrdenServicio` se copian sus campos, y uno de ellos es el puntero `detalles`. Se copia la **dirección**, no el arreglo de detalles. Por un momento la orden vieja y la nueva apuntan a los mismos detalles. Después, `delete[] s->ordenes` libera solo las órdenes viejas, no los arreglos de detalles. Los detalles siguen vivos y ahora solo los usa la orden nueva.

Por esto, en `liberarSistema` primero se borra cada `ordenes[i].detalles` y después `ordenes`. Si se borraran primero las órdenes, perderíamos esas direcciones y habría una fuga de memoria.